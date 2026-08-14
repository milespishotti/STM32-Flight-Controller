/*
 * I2CRecovery.c
 *
 *  Created on: Aug 13, 2026
 *      Author: miles
 */
#include "I2CRecovery.h"
#include "main.h"
#include <stdbool.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;

#define SDA GPIO_PIN_9 // PB9
#define SCL GPIO_PIN_8  // PB8

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET




static void I2CRecovery_Delay(void)
{
    for (volatile uint32_t i = 0; i < 100; i++)
    {
        __NOP();
    }
}

bool I2CRecovery(void)
{

    bool bus_released = false;

    HAL_I2C_DeInit(&hi2c1);

    GPIO_InitTypeDef GPIO_InitStruct = {0};


    GPIO_InitStruct.Pin = SDA | SCL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // since MPU already has pullups
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_WritePin(GPIOB, SDA, HIGH);
    HAL_GPIO_WritePin(GPIOB, SCL, HIGH);

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    if ((HAL_GPIO_ReadPin(GPIOB, SDA) == HIGH) && (HAL_GPIO_ReadPin(GPIOB, SCL) == HIGH))
    {
        bus_released = true;
    }

    else if ((HAL_GPIO_ReadPin(GPIOB, SDA) == LOW) && (HAL_GPIO_ReadPin(GPIOB, SCL) == HIGH))
    {
        uint8_t loop_count = 0;
        while ((HAL_GPIO_ReadPin(GPIOB, SDA) == LOW) && (loop_count < 9))
        {
            HAL_GPIO_WritePin(GPIOB, SCL, LOW);
            I2CRecovery_Delay();

            HAL_GPIO_WritePin(GPIOB, SCL, HIGH);
            I2CRecovery_Delay();

            loop_count++;
        }

        if ((HAL_GPIO_ReadPin(GPIOB, SDA) == HIGH))
        {
            bus_released = true;
        }

    }

    bool success = false;

    if (bus_released == true)
    {

        if ((HAL_GPIO_ReadPin(GPIOB, SCL) == HIGH))
        {
            HAL_GPIO_WritePin(GPIOB, SDA, LOW);
            I2CRecovery_Delay();

            HAL_GPIO_WritePin(GPIOB, SCL, HIGH);
            I2CRecovery_Delay();

            HAL_GPIO_WritePin(GPIOB, SDA, HIGH);
            I2CRecovery_Delay();

            success = ((HAL_GPIO_ReadPin(GPIOB, SDA) == HIGH) && (HAL_GPIO_ReadPin(GPIOB, SCL) == HIGH));

        }



    }

    return success;



}




