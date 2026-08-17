/*
 * SDCard.h
 *
 *  Created on: Aug 15, 2026
 *      Author: miles
 */

#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "debug.h"

/* SD Card Breakout Pins
 *
 * CS = PC1 / A4
 * SCK = PB13 / right male header in line with D4
 * MISO = PC2 / left male header next to PC1
 * MOSI = PC3 / left male header next to PC0
 *
 */


extern SPI_HandleTypeDef hspi2;

bool SDCard_ReadBlock(uint32_t block_number, uint8_t *data);

bool SDCard_WriteBlock(uint32_t block_number, const uint8_t *data);

bool SDCard_GetSectorCount(uint32_t *sector_count);





bool SDCard_Init(void);

bool SDCard_TestBlockIO(void);


#endif /* INC_SDCARD_H_ */
