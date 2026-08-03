/*
 * Safety.c
 *
 *  Created on: Aug 3, 2026
 *      Author: miles
 */


#include "Safety.h"

#define ARMING_THROTTLE_THRESHOLD 65


static bool armed;
static bool armed_switch_ready;


void Safety_Init(void)
{
    armed = false;
    armed_switch_ready = false;
}

void Safety_Update(const Safety_Input *input)
{
    if (!input)
    {
        armed = false;
        armed_switch_ready = false;
        return;
    }


    if (input->receiver_valid == false)
    {
        armed = false;
        armed_switch_ready = false;
        return;
    }

    if (input->arm_requested == false)
       {
           armed_switch_ready = true;
           armed = false;
           return;
       }

    if (input->arm_requested == true)
    {
        if (armed == false)
        {
            if ((input->throttle < ARMING_THROTTLE_THRESHOLD) && (armed_switch_ready == true))
            {
                armed = true;
                return;
            }
            else
            {
                armed = false;
                armed_switch_ready = false;
                return;
            }
        }

        armed = true;
        return;
    }

}

bool Safety_IsArmed(void)
{
    return armed;
}

