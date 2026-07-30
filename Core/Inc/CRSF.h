/*
 * CRSF.h
 *
 *  Created on: Jul 24, 2026
 *      Author: miles
 */

#ifndef INC_CRSF_H_
#define INC_CRSF_H_

#include <stdint.h>

const uint16_t *CRSF_GetChannels(void);
void CRSF_Init(void);

uint32_t CRSF_GetValidFrameCount(void);

void CRSF_FrameGenerator(const uint16_t channels[16], uint8_t frame[26]);

void CRSF_TestFrame(const uint8_t frame[26]);

#endif /* INC_CRSF_H_ */
