/*
 * RCInput.h
 *
 *  Created on: Jul 27, 2026
 *      Author: miles
 */

#ifndef INC_RCINPUT_H_
#define INC_RCINPUT_H_

#include <stdint.h>

#define CRSF_CHANNEL_COUNT 16


typedef struct
{
    float roll_setpoint;
    float pitch_setpoint;
    float yaw_rate_setpoint;
    uint16_t throttle;
    uint8_t armed;
} RC_Setpoints;

void RCInput_Update(const uint16_t channels[CRSF_CHANNEL_COUNT]);

const RC_Setpoints *RCInput_GetSetpoints(void);

#endif /* INC_RCINPUT_H_ */
