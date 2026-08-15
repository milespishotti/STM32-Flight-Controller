/*
 * Logging.c
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#include "Logging.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static FlightData data_storage;

static bool data_available = false;

static uint8_t snapshot_counter = 0;

void Logger_Reset(void)
{

    data_storage = (FlightData) {
        .roll_measured = 0.0f,
        .pitch_measured = 0.0f,
        .yaw_rate_measured = 0.0f,
        .roll_setpoint = 0.0f,
        .pitch_setpoint = 0.0f,
        .yaw_rate_setpoint = 0.0f,
        .roll_correction = 0.0f,
        .pitch_correction = 0.0f,
        .yaw_rate_correction = 0.0f,

        .throttle = 0,
        .valid_frame_count = 0,

        .motor1 = 0,
        .motor2 = 0,
        .motor3 = 0,
        .motor4 = 0,

        .receiver_valid = false,
        .sensor_valid = false,
        .arm_requested = false,
        .armed = false
    };


    data_available = false;
    snapshot_counter = 0;
}


void Logger_ReceiveData(const FlightData *data)
{

    if (snapshot_counter >= 9)
    {

        data_storage = *data;

        data_available = true;
        snapshot_counter = 0;

    }
    else
    {
        snapshot_counter++;
    }

}

void Logger_ProcessData(void)
{
    if (data_available == false)
    {
        return;
    }

    data_available = false;


    printf("Roll: %.2f | Pitch: %.2f | Roll SP: %.2f | Pitch SP: %.2f | Yaw Rate SP: %.2f | Roll Corr: %.2f | Pitch Corr: %.2f | Yaw Corr: %.2f | Throttle: %u | Frames: %lu\r\n",
           data_storage.roll_measured,
           data_storage.pitch_measured,
           data_storage.roll_setpoint,
           data_storage.pitch_setpoint,
           data_storage.yaw_rate_setpoint,
           data_storage.roll_correction,
           data_storage.pitch_correction,
           data_storage.yaw_rate_correction,
           data_storage.throttle,
           (unsigned long)data_storage.valid_frame_count);

    printf("M1:%u M2:%u M3:%u M4:%u\r\n",
           data_storage.motor1,
           data_storage.motor2,
           data_storage.motor3,
           data_storage.motor4);

    printf("Receiver: %i Sensor: %i Arm_Requested: %i Arm: %i\r\n", data_storage.receiver_valid, data_storage.sensor_valid, data_storage.arm_requested, data_storage.armed);

}



