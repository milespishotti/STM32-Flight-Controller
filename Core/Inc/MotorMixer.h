/*
 * MotorMixer.h
 *
 *  Created on: Jul 16, 2026
 *      Author: miles
 */

/* Orientation Convention
 *
 * raise nose = +pitch
 * lower noise = -pitch
 *
 * raise right side = +roll
 * lower right side = -roll
 *
 * rotate ccw = +yaw
 * rotate cw = yaw
 *
 * Motor Naming Convention (when viewing from top)
 *
 * M1 = upper left motor = D7 / PA8
 * M2 = upper right motor = D8 / PA9
 * M3 = lower right motor = D2 / PA10
 * M4 = lower left motor = Right Male In line with D12 / PA11
 *
 *
 *I messed up orientation
 *
 * m1 = upper right
 * m2 = upper left
 * m3 = lower right
 * m4 = lower left
 *
 *
 *
 *
 *
 */

#ifndef INC_MOTORMIXER_H_
#define INC_MOTORMIXER_H_

#include <stdint.h>

typedef struct
{
    uint16_t m1;
    uint16_t m2;
    uint16_t m3;
    uint16_t m4;
} MotorOutputs;

MotorOutputs MotorMixer_Mix(int16_t roll_correction, int16_t pitch_correction, int16_t yaw_correction, uint16_t base_throttle);



#endif /* INC_MOTORMIXER_H_ */
