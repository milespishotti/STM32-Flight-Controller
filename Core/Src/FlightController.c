/*
 * FlightController.c
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#include "FlightController.h"

#include <stddef.h>

static float FlightController_Compute(PID_Controller *state, float error, float dt);

static PID_Controller roll_pid;
static PID_Controller pitch_pid;
static PID_Controller yaw_rate_pid;

static FlightController_Output output;

void FlightController_Init(void)
{
    roll_pid = (PID_Controller) {
            .kp = 1.0f,
            .ki = 0.0f,
            .kd = 0.0f,
            .previous_error = 0.0f,
            .integral = 0.0f,
    };

    pitch_pid = (PID_Controller) {
                .kp = 1.0f,
                .ki = 0.0f,
                .kd = 0.0f,
                .previous_error = 0.0f,
                .integral = 0.0f,
    };

    yaw_rate_pid = (PID_Controller) {
                .kp = 1.0f,
                .ki = 0.0f,
                .kd = 0.0f,
                .previous_error = 0.0f,
                .integral = 0.0f,
    };

    output = (FlightController_Output) {
        .roll_correction = 0.0f,
        .pitch_correction = 0.0f,
        .yaw_rate_correction = 0.0f
    };

}

static float FlightController_Compute(PID_Controller *state, float error, float dt)
{

    if (dt <= 0.0f)
    {
        return 0.0f;
    }

    float proportional = state->kp * error;

    state->integral += error * dt;

    float derivative = (error - state->previous_error) / dt;

    float correction = proportional + (state->ki * state->integral) + (state->kd * derivative);

    state->previous_error = error;

    return correction;

}


void FlightController_Update(const FlightController_Input *input)
{

    if (input == NULL)
    {
        return;
    }

    float roll_error = input->roll_setpoint - input->roll_measured;
    float pitch_error = input->pitch_setpoint - input->pitch_measured;
    float yaw_rate_error = input->yaw_rate_setpoint - input->yaw_rate_measured;

    float dt = input->dt;

    output.roll_correction = FlightController_Compute(&roll_pid, roll_error, dt);
    output.pitch_correction = FlightController_Compute(&pitch_pid, pitch_error, dt);
    output.yaw_rate_correction = FlightController_Compute(&yaw_rate_pid, yaw_rate_error, dt);

}

const FlightController_Output *FlightController_GetOutput(void)
{
    return &output;
}

void FlightController_Reset(void)
{
    roll_pid.previous_error = 0.0f;
    roll_pid.integral = 0.0f;

    pitch_pid.previous_error = 0.0f;
    pitch_pid.integral = 0.0f;

    yaw_rate_pid.previous_error = 0.0f;
    yaw_rate_pid.integral = 0.0f;

    output.roll_correction = 0.0f;
    output.pitch_correction = 0.0f;
    output.yaw_rate_correction = 0.0f;

}



