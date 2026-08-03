/*
 * CRSF.h
 *
 *  Created on: Jul 24, 2026
 *      Author: miles
 */

#ifndef INC_CRSF_H_
#define INC_CRSF_H_

#include <stdint.h>
extern volatile uint32_t crsf_dma_callback_count;

extern uint32_t bad_address_count;
extern uint32_t bad_length_count;
extern uint32_t bad_type_count;
extern uint32_t bad_crc_count;

extern uint32_t parse_call_count;
extern uint32_t parse_fail_count;

const uint16_t *CRSF_GetChannels(void);
void CRSF_Init(void);

uint32_t CRSF_GetValidFrameCount(void);

void CRSF_FrameGenerator(const uint16_t channels[16], uint8_t frame[26]);

void CRSF_TestFunction(const uint8_t *data, uint16_t length);


void CRSF_TestFrame(const uint8_t frame[26]);

#endif /* INC_CRSF_H_ */
