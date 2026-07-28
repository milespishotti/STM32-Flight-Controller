/*
 * RCInput.c
 *
 *  Created on: Jul 27, 2026
 *      Author: miles
 */

#include "RCInput.h"
#include <stdint.h>
#include <math.h>


static RC_Setpoints setpoints;



void RCInput_Update(const uint16_t channels[CRSF_CHANNEL_COUNT])
{
    uint16_t raw_roll = channels[0];
    uint16_t raw_pitch = channels[1];
    uint16_t raw_throttle = channels[2];
    uint16_t raw_yaw_rate = channels[3];

    /* Clamp throttle to valid CRSF range */
    if (raw_throttle < 172)
    {
        raw_throttle = 172;
    }
    else if (raw_throttle > 1811)
    {
        raw_throttle = 1811;
    }

    /* Normalize stick inputs */
    float normalized_roll = (raw_roll - 992.0f) / 819.0f;
    float normalized_pitch = (raw_pitch - 992.0f) / 819.0f;
    float normalized_yaw_rate = (raw_yaw_rate - 992.0f) / 819.0f;
    float normalized_throttle = (raw_throttle - 172.0f) / (1811.0f - 172.0f);

    /* Convert to controller setpoints */
    float roll_setpoint = normalized_roll * 30.0f;
    float pitch_setpoint = normalized_pitch * 30.0f;
    float yaw_rate_setpoint = normalized_yaw_rate * 150.0f;
    float throttle_setpoint = 48.0f + normalized_throttle * (2047.0f - 48.0f);

    /* Deadband */
    const float angle_deadband = 2.5f;
    const float yaw_deadband = 5.0f;

    if (fabsf(roll_setpoint) < angle_deadband)
    {
        roll_setpoint = 0.0f;
    }

    if (fabsf(pitch_setpoint) < angle_deadband)
    {
        pitch_setpoint = 0.0f;
    }

    if (fabsf(yaw_rate_setpoint) < yaw_deadband)
    {
        yaw_rate_setpoint = 0.0f;
    }

    /* Store internally */
    setpoints.roll_setpoint = roll_setpoint;
    setpoints.pitch_setpoint = pitch_setpoint;
    setpoints.yaw_rate_setpoint = yaw_rate_setpoint;
    setpoints.throttle = (uint16_t)throttle_setpoint;
}


