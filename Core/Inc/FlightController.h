/*
 * FlightController.h
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#ifndef INC_FLIGHTCONTROLLER_H_
#define INC_FLIGHTCONTROLLER_H_

#include <stdint.h>

typedef struct
{
    float roll_setpoint;
    float pitch_setpoint;
    float yaw_rate_setpoint;

    float roll_measured;
    float pitch_measured;
    float yaw_rate_measured;

    uint16_t throttle;

    float dt;
} FlightController_Input;

typedef struct
{
    float roll_correction;
    float pitch_correction;
    float yaw_rate_correction;

    float roll_p_term;
    float roll_i_term;
    float roll_d_term;

    float pitch_p_term;
    float pitch_i_term;
    float pitch_d_term;

    float yaw_p_term;
    float yaw_i_term;
    float yaw_d_term;

} FlightController_Output;

typedef struct
{
    float kp;
    float kd;
    float ki;

    float previous_error;
    float integral;
} PID_Controller;

typedef struct
{
    float correction;
    float proportional;
    float integral;
    float derivative;
} PID_Output;



void FlightController_Update(const FlightController_Input *input);

void FlightController_Init(void);

const FlightController_Output *FlightController_GetOutput(void);

void FlightController_Reset(void);


#endif /* INC_FLIGHTCONTROLLER_H_ */
