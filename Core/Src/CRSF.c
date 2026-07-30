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

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

static uint8_t rx_buffer[CRSF_RC_FRAME_SIZE];
static uint16_t channels[CRSF_CHANNEL_COUNT];

static void CRSF_ParseFrame(void);
static uint8_t CRSF_CalculateCRC(const uint8_t *data, uint8_t length);
static void CRSF_UpdateChannels(const uint8_t *payload);

static volatile uint32_t valid_frame_count = 0;


void CRSF_Init(void)
{
    HAL_UART_Receive_DMA(&huart6, rx_buffer, CRSF_RC_FRAME_SIZE);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart6)
    {
//        printf("%02X %02X %02X %02X\r\n",
//                       rx_buffer[0],
//                       rx_buffer[1],
//                       rx_buffer[2],
//                       rx_buffer[3]);
        CRSF_ParseFrame();
        valid_frame_count++;
//        printf("RX\r\n");

        HAL_UART_Receive_DMA(&huart6, rx_buffer, CRSF_RC_FRAME_SIZE);
    }
}

const uint16_t *CRSF_GetChannels(void)
{
    return channels;
}

static void CRSF_ParseFrame(void)
{
    if (rx_buffer[0] != CRSF_ADDRESS_FLIGHT_CONTROLLER)
    {
        return;
    }

    if (rx_buffer[1] != CRSF_RC_FRAME_LENGTH_FIELD)
    {
        return;
    }

    if (rx_buffer[2] != CRSF_FRAME_TYPE_RC_CHANNELS)
    {
        return;
    }

    uint8_t calculated_crc = CRSF_CalculateCRC(&rx_buffer[2], 23);

//    printf("CRC calc: %02X  recv: %02X\r\n",
//           calculated_crc,
//           rx_buffer[25]);

    if (calculated_crc != rx_buffer[25])
    {
        return;
    }

    CRSF_UpdateChannels(&rx_buffer[3]);

}


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


void CRSF_TestFrame(const uint8_t frame[26]) {

    for (int i = 0; i < CRSF_RC_FRAME_SIZE; i ++)
    {
        rx_buffer[i] = frame[i];
    }

    CRSF_ParseFrame();

}

uint32_t CRSF_GetValidFrameCount(void)
{
    return valid_frame_count;
}






















