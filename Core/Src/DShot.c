/*
 * DShot.c
 *
 *  Created on: Jul 14, 2026
 *      Author: miles
 */

#include "DShot.h"

#define DSHOT_LEN 18
#define DSHOT_1_PULSE 420
#define DSHOT_0_PULSE 210

extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_up;

static uint16_t dshot_buffer1[DSHOT_LEN] = {0};
static uint16_t dshot_buffer2[DSHOT_LEN] = {0};
static uint16_t dshot_buffer3[DSHOT_LEN] = {0};
static uint16_t dshot_buffer4[DSHOT_LEN] = {0};

static uint16_t burst_buffer[DSHOT_LEN * 4] = {0};

volatile uint8_t dshot_busy = 0;
volatile uint32_t dshot_start_count = 0;
volatile uint32_t dshot_done_count = 0;
volatile uint32_t dshot_busy_skip_count = 0;

volatile uint32_t dshot_start_error_count = 0;
volatile HAL_StatusTypeDef dshot_last_status = HAL_OK;


static void (*hal_dshot_complete_callback)(DMA_HandleTypeDef *hdma) = NULL;

static void DShot_BuildPacket(uint16_t throttle_cmd, uint16_t output_buffer[DSHOT_LEN])
{
    uint16_t telemetry = 0;
    uint16_t value = (throttle_cmd << 1) | telemetry;

    uint16_t csum = 0;
    uint16_t csum_data = value;

    for (int i = 0; i < 3; i++)
    {
        csum ^= csum_data;
        csum_data >>= 4;
    }

    csum &= 0x0F;

    uint16_t packet = (value << 4) | csum;

    for (int i = 0; i < 16; i++)
    {
        output_buffer[i] = (packet & (1 << (15 - i))) ? DSHOT_1_PULSE : DSHOT_0_PULSE;
    }

    output_buffer[16] = 0;
    output_buffer[17] = 0;
}

static void DShot_BuildBurstBuffer(uint16_t buffer1[DSHOT_LEN], uint16_t buffer2[DSHOT_LEN], uint16_t buffer3[DSHOT_LEN], uint16_t buffer4[DSHOT_LEN])
{
    for (int i = 0; i < DSHOT_LEN; i ++)
    {
        uint16_t base = i * 4;

        burst_buffer[base + 0] = buffer1[i];
        burst_buffer[base + 1] = buffer2[i];
        burst_buffer[base + 2] = buffer3[i];
        burst_buffer[base + 3] = buffer4[i];
    }
}


void DShot_Init(void)
{

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

}

void DShot_DMA_Complete(DMA_HandleTypeDef *hdma)
{
    if (hdma != &hdma_tim1_up)
    {
        return;
    }

    if (hal_dshot_complete_callback != NULL)
        {
            hal_dshot_complete_callback(hdma);
        }

    HAL_TIM_DMABurst_WriteStop(&htim1, TIM_DMA_UPDATE);


    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);

    dshot_done_count++;
    dshot_busy = 0;

}


void DShot_Send(uint16_t throttle_cmd1, uint16_t throttle_cmd2, uint16_t throttle_cmd3, uint16_t throttle_cmd4)
{
    if (dshot_busy)
    {
        dshot_busy_skip_count++;
        return;
    }

    dshot_busy = 1;
    dshot_start_count++;

    DShot_BuildPacket(throttle_cmd1, dshot_buffer1);
    DShot_BuildPacket(throttle_cmd2, dshot_buffer2);
    DShot_BuildPacket(throttle_cmd3, dshot_buffer3);
    DShot_BuildPacket(throttle_cmd4, dshot_buffer4);

    DShot_BuildBurstBuffer(dshot_buffer1, dshot_buffer2, dshot_buffer3, dshot_buffer4);


    __HAL_TIM_SET_COUNTER(&htim1, 0);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);



    HAL_StatusTypeDef status = HAL_TIM_DMABurst_MultiWriteStart(&htim1, TIM_DMABASE_CCR1, TIM_DMA_UPDATE, (uint32_t *)burst_buffer,TIM_DMABURSTLENGTH_4TRANSFERS, DSHOT_LEN * 4);

    dshot_last_status = status;

    if (status != HAL_OK)
    {
        dshot_busy = 0;
        dshot_start_error_count++;
        return;
    }

    hal_dshot_complete_callback = hdma_tim1_up.XferCpltCallback;
    hdma_tim1_up.XferCpltCallback = DShot_DMA_Complete;

}










