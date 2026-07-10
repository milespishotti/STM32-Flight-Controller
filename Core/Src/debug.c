/*
 * debug.c
 *
 *  Created on: Jul 9, 2026
 *      Author: miles
 */


#include "debug.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

int __io_putchar(int ch)
{
    HAL_UART_Transmit(
            &huart2,
            (uint8_t*)&ch,
            1,
            HAL_MAX_DELAY
            );
    return ch;
}



