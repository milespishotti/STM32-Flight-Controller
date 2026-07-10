/*
 * Mahony.c
 *
 *  Created on: Jul 9, 2026
 *      Author: miles
 */

#include "Mahony.h"
#include <math.h>


#define DEG_TO_RAD 0.01745329252f
#define RAD_TO_DEG 57.29577951f

static float q0 = 1.0f;
static float q1 = 0.0f;
static float q2 = 0.0f;
static float q3 = 0.0f;

static float integralX = 0.0f;
static float integralY = 0.0f;
static float integralZ = 0.0f;

static float twoKp = 1.0f;
static float twoKi = 0.0f;



void Mahony_Init(void)
{
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;

    integralX = 0.0f;
    integralY = 0.0f;
    integralZ = 0.0f;
}

void Mahony_UpdateIMU(float gx, float gy, float gz,
                      float ax, float ay, float az,
                      float dt)
{
    float recipNorm;

    float halfVx;
    float halfVy;
    float halfVz;

    float halfEx;
    float halfEy;
    float halfEz;

    float qa;
    float qb;
    float qc;

    gx *= DEG_TO_RAD;
    gy *= DEG_TO_RAD;
    gz *= DEG_TO_RAD;

    /*
     * Apply accelerometer correction only when the
     * acceleration vector is valid.
     */
    if (!(ax == 0.0f && ay == 0.0f && az == 0.0f))
    {
        recipNorm = 1.0f / sqrtf(ax * ax +
                                 ay * ay +
                                 az * az);

        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        halfVx = q1 * q3 - q0 * q2;
        halfVy = q0 * q1 + q2 * q3;
        halfVz = q0 * q0 - 0.5f + q3 * q3;

        halfEx = ay * halfVz - az * halfVy;
        halfEy = az * halfVx - ax * halfVz;
        halfEz = ax * halfVy - ay * halfVx;

        if (twoKi > 0.0f)
        {
            integralX += twoKi * halfEx * dt;
            integralY += twoKi * halfEy * dt;
            integralZ += twoKi * halfEz * dt;

            gx += integralX;
            gy += integralY;
            gz += integralZ;
        }
        else
        {
            integralX = 0.0f;
            integralY = 0.0f;
            integralZ = 0.0f;
        }

        gx += twoKp * halfEx;
        gy += twoKp * halfEy;
        gz += twoKp * halfEz;
    }

    /*
     * Integrate the corrected gyro into the quaternion.
     * This still happens if accelerometer correction is unavailable.
     */
    gx *= 0.5f * dt;
    gy *= 0.5f * dt;
    gz *= 0.5f * dt;

    qa = q0;
    qb = q1;
    qc = q2;

    q0 += -qb * gx - qc * gy - q3 * gz;
    q1 +=  qa * gx + qc * gz - q3 * gy;
    q2 +=  qa * gy - qb * gz + q3 * gx;
    q3 +=  qa * gz + qb * gy - qc * gx;

    recipNorm = 1.0f / sqrtf(q0 * q0 +
                             q1 * q1 +
                             q2 * q2 +
                             q3 * q3);

    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;
}


void Mahony_GetEuler(float *roll, float *pitch, float *yaw)
{
    float pitchInput;

    *roll = atan2f(
        2.0f * (q0 * q1 + q2 * q3),
        1.0f - 2.0f * (q1 * q1 + q2 * q2)
    ) * RAD_TO_DEG;

    pitchInput = 2.0f * (q0 * q2 - q3 * q1);

    if (pitchInput > 1.0f)
    {
        pitchInput = 1.0f;
    }
    else if (pitchInput < -1.0f)
    {
        pitchInput = -1.0f;
    }

    *pitch = asinf(pitchInput) * RAD_TO_DEG;

    *yaw = atan2f(
        2.0f * (q0 * q3 + q1 * q2),
        1.0f - 2.0f * (q2 * q2 + q3 * q3)
    ) * RAD_TO_DEG;
}





