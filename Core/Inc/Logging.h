/*
 * Logging.h
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_

#include "fatfs.h"

#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    float roll_measured;
    float pitch_measured;
    float yaw_rate_measured;

    float roll_setpoint;
    float pitch_setpoint;
    float yaw_rate_setpoint;

    float roll_correction;
    float pitch_correction;
    float yaw_rate_correction;

    uint16_t throttle;

    uint16_t motor1;
    uint16_t motor2;
    uint16_t motor3;
    uint16_t motor4;

    uint32_t valid_frame_count;

    bool receiver_valid;
    bool sensor_valid;
    bool arm_requested;
    bool armed;

    uint32_t timestamp_ms;


} FlightData;


void Logger_Reset(void);

void Logger_ReceiveData(const FlightData *data);

void Logger_ProcessData(void);





#endif /* INC_LOGGING_H_ */
