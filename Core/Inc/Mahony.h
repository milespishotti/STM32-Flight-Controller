/*
 * Mahony.h
 *
 *  Created on: Jul 9, 2026
 *      Author: miles
 */

#ifndef INC_MAHONY_H_
#define INC_MAHONY_H_

void Mahony_Init(void);

void Mahony_GetEuler(float *roll, float *pitch, float *yaw);

void Mahony_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt);



#endif /* INC_MAHONY_H_ */
