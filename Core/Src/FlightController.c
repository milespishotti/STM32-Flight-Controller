/*
 * FlightController.c
 *
 *  Created on: Jul 30, 2026
 *      Author: miles
 */

#include "FlightController.h"

#include <stddef.h>
#include <stdbool.h>

#define PITCH_TRIM_DEG 2.5f

static PID_Output FlightController_Compute(PID_Controller *state, float error, float dt, bool integral_enabled);

static PID_Controller roll_pid;
static PID_Controller pitch_pid;
static PID_Controller yaw_rate_pid;


static FlightController_Output output;

void FlightController_Init(void)
{
    roll_pid = (PID_Controller) {
            .kp = 7.0f,
            .ki = 1.0f,
            .kd = 0.20f,
            .previous_error = 0.0f,
            .integral = 0.0f,
    };

    pitch_pid = (PID_Controller) {
                .kp = 7.0f,
                .ki = 1.6f,
                .kd = 0.20f,
                .previous_error = 0.0f,
                .integral = 0.0f,
    };

    yaw_rate_pid = (PID_Controller) {
                .kp = 0.5f,
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

static PID_Output FlightController_Compute(PID_Controller *state, float error, float dt, bool integral_enabled)
{

    PID_Output result ={0};

    if (dt <= 0.0f)
    {
        return result;
    }

    result.proportional = state->kp * error;

    if (integral_enabled)
    {
        state->integral += error * dt;
    }
    else
    {
        state->integral = 0.0f;
    }

    result.integral = state->ki * state->integral;

    float derivative = (error - state->previous_error) / dt;
    result.derivative = state->kd * derivative;

    result.correction =
            result.proportional +
            result.integral +
            result.derivative;

    state->previous_error = error;

    return result;

}


void FlightController_Update(const FlightController_Input *input)
{

    if (input == NULL)
    {
        return;
    }

    bool integral_enabled = false;

    uint16_t throttle = input->throttle;

    if (throttle >= 650)
    {
        integral_enabled = true;
    }

    float roll_error = input->roll_setpoint - input->roll_measured;

    float pitch_target = input->pitch_setpoint + PITCH_TRIM_DEG;
    float pitch_error = pitch_target - input->pitch_measured;

    float yaw_rate_error = input->yaw_rate_setpoint - input->yaw_rate_measured;

    float dt = input->dt;

    PID_Output roll_result = FlightController_Compute(&roll_pid, roll_error, dt, integral_enabled);
    PID_Output pitch_result = FlightController_Compute(&pitch_pid, pitch_error, dt, integral_enabled);
    PID_Output yaw_result = FlightController_Compute(&yaw_rate_pid, yaw_rate_error, dt, integral_enabled);

    output.roll_correction = roll_result.correction;
    output.roll_p_term = roll_result.proportional;
    output.roll_i_term = roll_result.integral;
    output.roll_d_term = roll_result.derivative;

    output.pitch_correction = pitch_result.correction;
    output.pitch_p_term = pitch_result.proportional;
    output.pitch_i_term = pitch_result.integral;
    output.pitch_d_term = pitch_result.derivative;

    output.yaw_rate_correction = yaw_result.correction;
    output.yaw_p_term = yaw_result.proportional;
    output.yaw_i_term = yaw_result.integral;
    output.yaw_d_term = yaw_result.derivative;

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



