/*
 * DShot.h
 *
 *  Created on: Jul 14, 2026
 *      Author: miles
 */

#ifndef INC_DSHOT_H_
#define INC_DSHOT_H_


#include "main.h"
#include "debug.h"

void DShot_Init(void);
void DShot_Send(uint16_t throttle_cmd1, uint16_t throttle_cmd2, uint16_t throttle_cmd3, uint16_t throttle_cmd4);
void DShot_DMA_Complete(DMA_HandleTypeDef *hdma);

extern volatile uint8_t dshot_busy;
extern volatile uint32_t dshot_start_count;
extern volatile uint32_t dshot_done_count;
extern volatile uint32_t dshot_busy_skip_count;

extern volatile uint32_t dshot_start_error_count;
extern volatile HAL_StatusTypeDef dshot_last_status;


#endif /* INC_DSHOT_H_ */
