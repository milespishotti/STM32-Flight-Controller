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



void CRSF_Init(void)
{
    HAL_UART_Receive_DMA(&huart6, rx_buffer, CRSF_RC_FRAME_SIZE);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart6)
    {
        CRSF_ParseFrame();

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



























