/*
 * MotorMixer.c
 *
 *  Created on: Jul 16, 2026
 *      Author: miles
 */
#include "MotorMixer.h"

// Clamps motor commands to acceptable DShot throttle ranges
// 0-47 is reserved for DShot commands
// anything over 2027 is invalid, 48-2047 is the valid dshot throttle ranges
static uint16_t MotorMixer_Clamp(int32_t value)
{
    if (value < 48)
    {
        value = 48;
    }
    else if (value > 2047)
    {
        value = 2047;
    }
    return (uint16_t)value;
}


MotorOutputs MotorMixer_Mix(int16_t roll_correction, int16_t pitch_correction, int16_t yaw_correction, uint16_t base_throttle)
{
    MotorOutputs output;

    int32_t m1 = base_throttle - roll_correction + pitch_correction - yaw_correction;
    int32_t m2 = base_throttle + roll_correction + pitch_correction + yaw_correction;
    int32_t m3 = base_throttle + roll_correction - pitch_correction - yaw_correction;
    int32_t m4 = base_throttle - roll_correction - pitch_correction + yaw_correction;

    output.m1 = MotorMixer_Clamp(m1);
    output.m2 = MotorMixer_Clamp(m2);
    output.m3 = MotorMixer_Clamp(m3);
    output.m4 = MotorMixer_Clamp(m4);



    return output;
}

