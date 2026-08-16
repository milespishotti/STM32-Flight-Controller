/*
 * SDCard.h
 *
 *  Created on: Aug 15, 2026
 *      Author: miles
 */

#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

#include <stdbool.h>
#include "main.h"
#include "debug.h"

extern SPI_HandleTypeDef hspi2;


bool SDCard_Init(void);


#endif /* INC_SDCARD_H_ */
