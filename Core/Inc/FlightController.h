/*
 * FlightController.h
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#ifndef INC_FLIGHTCONTROLLER_H_
#define INC_FLIGHTCONTROLLER_H_

typedef struct
{
    float roll_setpoint;
    float pitch_setpoint;
    float yaw_rate_setpoint;

    float roll_measured;
    float pitch_measured;
    float yaw_rate_measured;

    float dt;
} FlightController_Input;

typedef struct
{
    float roll_correction;
    float pitch_correction;
    float yaw_rate_correction;
} FlightController_Output;

typedef struct
{
    float kp;
    float kd;
    float ki;

    float previous_error;
    float integral;
} PID_Controller;



void FlightController_Update(const FlightController_Input *input);

void FlightController_Init(void);

const FlightController_Output *FlightController_GetOutput(void);

void FlightController_Reset(void);


#endif /* INC_FLIGHTCONTROLLER_H_ */
