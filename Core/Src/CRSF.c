/*
 * CRSF.c
 *
 *  Created on: Jul 24, 2026
 *      Author: miles
 */

#include "CRSF.h"
#include "main.h"
#include "debug.h"

#include <stdbool.h>


#define CRSF_RC_FRAME_SIZE 26
#define CRSF_CHANNEL_COUNT 16

#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
#define CRSF_FRAME_TYPE_RC_CHANNELS 0x16
#define CRSF_RC_FRAME_LENGTH_FIELD 24

#define CRSF_DMA_BUFFER_SIZE 128

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

static uint8_t rx_buffer[CRSF_DMA_BUFFER_SIZE];
static uint16_t channels[CRSF_CHANNEL_COUNT];

static uint8_t stream_buffer[CRSF_DMA_BUFFER_SIZE] = {0};
static uint16_t num_bytes_stored = 0;

static uint16_t last_position = 0;

static uint8_t CRSF_CalculateCRC(const uint8_t *data, uint8_t length);
static void CRSF_UpdateChannels(const uint8_t *payload);
static void CRSF_ProcessBytes(const uint8_t *byte_array, uint16_t num_bytes);


static volatile uint32_t valid_frame_count = 0;


uint32_t bad_address_count = 0;
uint32_t bad_length_count = 0;
uint32_t bad_type_count = 0;
uint32_t bad_crc_count = 0;


uint32_t parse_call_count = 0;
uint32_t parse_fail_count = 0;

volatile uint32_t crsf_dma_callback_count = 0;
volatile uint32_t crsf_dma_size = 0;


void CRSF_Init(void)
{

    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buffer, CRSF_DMA_BUFFER_SIZE);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart6)
    {
            crsf_dma_callback_count++;
            crsf_dma_size = Size;

            if (last_position < Size)
            {
                CRSF_ProcessBytes(&rx_buffer[last_position], Size - last_position);
                last_position = Size;
            }
            else if (last_position > Size)
            {
                if (last_position < CRSF_DMA_BUFFER_SIZE)
                {
                    CRSF_ProcessBytes(&rx_buffer[last_position], CRSF_DMA_BUFFER_SIZE - last_position);
                }
                if (Size > 0)
                {
                    CRSF_ProcessBytes(&rx_buffer[0], Size);
                }
                last_position = Size;
            }

    }
}



const uint16_t *CRSF_GetChannels(void)
{
    return channels;
}



static void CRSF_ProcessBytes(const uint8_t *byte_array, uint16_t num_bytes)
{
    uint16_t byte_array_index = 0;

    while ((num_bytes_stored < CRSF_DMA_BUFFER_SIZE) && (byte_array_index < num_bytes))
    {
        stream_buffer[num_bytes_stored] = byte_array[byte_array_index];
        num_bytes_stored++;
        byte_array_index++;
    }

    uint16_t scan_index = 0;

    while (scan_index < num_bytes_stored)
    {
        if (stream_buffer[scan_index] == CRSF_ADDRESS_FLIGHT_CONTROLLER)
        {
            if (num_bytes_stored - scan_index < CRSF_RC_FRAME_SIZE)
            {
                uint16_t bytes_left = num_bytes_stored - scan_index;

                for (int j = 0; j < bytes_left; j ++)
                {
                    stream_buffer[j] = stream_buffer[scan_index + j];
                }
                num_bytes_stored = bytes_left;
                return;
            }

            if (stream_buffer[scan_index + 1] != CRSF_RC_FRAME_LENGTH_FIELD)
            {
                scan_index++;
                continue;
            }

            if (stream_buffer[scan_index + 2] != CRSF_FRAME_TYPE_RC_CHANNELS)
            {
                scan_index++;
                continue;
            }

            uint8_t calculated_crc = CRSF_CalculateCRC(&stream_buffer[scan_index + 2], 23);

            if (calculated_crc != stream_buffer[scan_index + 25])
            {
                scan_index++;
                continue;
            }

            CRSF_UpdateChannels(&stream_buffer[scan_index + 3]);

            valid_frame_count++;

            uint16_t bytes_left = num_bytes_stored - scan_index - CRSF_RC_FRAME_SIZE;

            for (int j = 0; j < bytes_left; j++)
            {
                stream_buffer[j] = stream_buffer[j + scan_index + CRSF_RC_FRAME_SIZE];
            }

            num_bytes_stored = bytes_left;
            scan_index = 0;
            continue;


        }
        scan_index++;
    }

    num_bytes_stored = 0;
}


//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart == &huart6)
//    {
//        crsf_dma_callback_count++;
//
//        CRSF_ProcessBytes(rx_buffer, CRSF_RC_FRAME_SIZE);
//
//
//        HAL_UART_Receive_DMA(&huart6, rx_buffer, CRSF_RC_FRAME_SIZE);
//    }
//}



static void CRSF_UpdateChannels(const uint8_t *payload)
{
    uint16_t bit_index = 0;

    for (uint16_t channel = 0; channel < CRSF_CHANNEL_COUNT; channel++)
    {

        uint8_t byte_index = bit_index / 8;
        uint8_t bit_offset = bit_index % 8;

        uint32_t window = ((uint32_t)payload[byte_index]) | ((uint32_t)payload[byte_index + 1] << 8);
        if ((byte_index + 2) < 22)
        {
            window |= ((uint32_t)payload[byte_index+2] << 16);
        }

        channels[channel] = (window >> bit_offset) & 0x07FF;

        bit_index += 11;
    }
}


static uint8_t CRSF_CalculateCRC(const uint8_t *data, const uint8_t length)
{
    uint8_t crc = 0;

    for (uint8_t j = 0; j < length; j ++)
    {
        crc ^= data[j];

        for (uint8_t i = 0; i < 8; i ++)
        {
            bool msb = crc & 0x80;

            crc <<= 1;

            if (msb)
            {
                crc ^= 0xD5;
            }
        }

    }

    return crc;

}

void CRSF_FrameGenerator(const uint16_t channels[CRSF_CHANNEL_COUNT], uint8_t frame[CRSF_RC_FRAME_SIZE])
{

    for (int i = 0; i < CRSF_RC_FRAME_SIZE; i++)
    {
        frame[i] = 0;
    }


    frame[0] = CRSF_ADDRESS_FLIGHT_CONTROLLER;
    frame[1] = CRSF_RC_FRAME_LENGTH_FIELD;
    frame[2] = CRSF_FRAME_TYPE_RC_CHANNELS;

    frame[3] = channels[0]; // first eight bits of channel 0
    frame[4] = ((channels[1] & 0x1F) << 3) | ((channels[0] >> 8) & 0x07); // first five bits of channel 1 + last three bits of channel 0
    frame[5] = ((channels[2] & 0x03) << 6) | ((channels[1] >> 5) & 0x3F);  // first two bits of channel 2 + last six bits of channel 1
    frame[6] = ((channels[2] >> 2) & 0xFF);   // third through the 10th bits of channel 2
    frame[7] = ((channels[3] & 0x7F) << 1) | ((channels[2] >> 10) & 0x01); // first seven bits of channel 3 and last bit of channel 2
    frame[8] = ((channels[4] & 0x0F) << 4) | ((channels[3] >> 7) & 0x0F); // first four bits of channel 4 and last four bits of channel 3
    frame[9] = ((channels[5] & 0x01) << 7) | ((channels[4] >> 4) & 0x7F); // first bit of channel 5 and last seven bits of channel 4
    frame[10] = ((channels[5] >> 1) & 0xFF); // second through the 9th bits of channel 5
    frame[11] = ((channels[6] & 0x3F) << 2) | ((channels[5] >> 9) & 0x03); // first six bits of channel 6 and last two bits of channel 5
    frame[12] = ((channels[7] & 0x07) << 5) | ((channels[6] >> 6) & 0x1F); // first three bits of channel 7 and last five bits of channel 6
    frame[13] = ((channels[7] >> 3) & 0xFF);  // last eight bits of channel 7

    frame[14] = channels[8];
    frame[15] = ((channels[9] & 0x1F) << 3) | ((channels[8] >> 8) & 0x07);
    frame[16] = ((channels[10] & 0x03) << 6) | ((channels[9] >> 5) & 0x3F);
    frame[17] = ((channels[10] >> 2) & 0xFF);
    frame[18] = ((channels[11] & 0x7F) << 1) | ((channels[10] >> 10) & 0x01);
    frame[19] = ((channels[12] & 0x0F) << 4) | ((channels[11] >> 7) & 0x0F);
    frame[20] = ((channels[13] & 0x01) << 7) | ((channels[12] >> 4) & 0x7F);
    frame[21] = ((channels[13] >> 1) & 0xFF);
    frame[22] = ((channels[14] & 0x3F) << 2) | ((channels[13] >> 9) & 0x03);
    frame[23] = ((channels[15] & 0x07) << 5) | ((channels[14] >> 6) & 0x1F);
    frame[24] = ((channels[15] >> 3) & 0xFF);

    frame[25] = CRSF_CalculateCRC(&frame[2], 23);

}


void CRSF_TestFunction(const uint8_t *data, uint16_t length) {


    CRSF_ProcessBytes(data, length);

}

uint32_t CRSF_GetValidFrameCount(void)
{
    return valid_frame_count;
}






















