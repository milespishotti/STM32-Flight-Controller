/*
 * Safety.h
 *
 *  Created on: Aug 3, 2026
 *      Author: miles
 */

#ifndef INC_SAFETY_H_
#define INC_SAFETY_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool arm_requested;
    uint16_t throttle;
    bool receiver_valid;
    bool sensor_valid;
} Safety_Input;

void Safety_Init(void);

void Safety_Update(const Safety_Input *input);

bool Safety_IsArmed(void);



#endif /* INC_SAFETY_H_ */
