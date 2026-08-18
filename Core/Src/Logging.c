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
#include <string.h>


#define LOG_BUFFER_SIZE 1000

static FlightData log_buffer[LOG_BUFFER_SIZE];
static uint16_t log_index = 0;
static bool buffer_full = false;

void Logger_Reset(void)
{

   log_index = 0;
   buffer_full = false;
}


void Logger_ReceiveData(const FlightData *data)
{

    if (log_index < LOG_BUFFER_SIZE)
    {
        log_buffer[log_index] = *data;
        log_index++;
    }
    else
    {
        buffer_full = true;
    }

}

void Logger_ProcessData(void)
{
    if (log_index == 0)
    {
        return;
    }

    FATFS fs;
    FIL file;
    FRESULT res;
    UINT bytes_written;
    FILINFO fno;

    res = f_mount(&fs, "0:", 1);

    printf("mount res = %d\r\n", res);

    if (res!=FR_OK)
    {
        return;
    }

    char filename[20];

    for (uint16_t i = 0; i < 1000; i++)
    {
        snprintf(filename, sizeof(filename),
                           "0:/LOG%03u.CSV", i);

        res = f_stat(filename, &fno);

        if (res == FR_NO_FILE)
        {
            break;
        }
    }

    if (res != FR_NO_FILE)
    {
        f_mount(NULL, "0:", 1);
        return;
    }

    printf("filename = %s, stat res = %d\r\n", filename, res);

    res = f_open(&file, filename, FA_CREATE_NEW | FA_WRITE);

    printf("open res = %d\r\n", res);

    if (res != FR_OK)
    {
        f_mount(NULL, "0:", 1);
        return;
    }

    const char *header =
        "timestamp_ms,roll_measured,pitch_measured,yaw_rate_measured,"
        "roll_setpoint,pitch_setpoint,yaw_rate_setpoint,"
        "roll_correction,roll_p_term,roll_i_term,roll_d_term,"
        "pitch_correction,pitch_p_term,pitch_i_term,pitch_d_term,"
        "yaw_rate_correction,yaw_p_term,yaw_i_term,yaw_d_term,"
        "throttle,motor1,motor2,motor3,motor4,"
        "valid_frame_count,receiver_valid,sensor_valid,arm_requested,armed\r\n";

    res = f_write(&file, header, strlen(header), &bytes_written);

    if ((res != FR_OK) || (bytes_written != strlen(header)))
    {
        f_close(&file);
        f_mount(NULL, "0:", 1);
        return;
    }

    char line[512];

   for (uint16_t i = 0; i < log_index; i++)
   {

       int len = snprintf(
           line,
           sizeof(line),
           "%lu,"
           "%.2f,%.2f,%.2f,"
           "%.2f,%.2f,%.2f,"
           "%.2f,%.2f,%.2f,%.2f,"
           "%.2f,%.2f,%.2f,%.2f,"
           "%.2f,%.2f,%.2f,%.2f,"
           "%u,%u,%u,%u,%u,"
           "%lu,%u,%u,%u,%u\r\n",

           (unsigned long)log_buffer[i].timestamp_ms,

           log_buffer[i].roll_measured,
           log_buffer[i].pitch_measured,
           log_buffer[i].yaw_rate_measured,

           log_buffer[i].roll_setpoint,
           log_buffer[i].pitch_setpoint,
           log_buffer[i].yaw_rate_setpoint,

           log_buffer[i].roll_correction,
           log_buffer[i].roll_p_term,
           log_buffer[i].roll_i_term,
           log_buffer[i].roll_d_term,

           log_buffer[i].pitch_correction,
           log_buffer[i].pitch_p_term,
           log_buffer[i].pitch_i_term,
           log_buffer[i].pitch_d_term,

           log_buffer[i].yaw_rate_correction,
           log_buffer[i].yaw_p_term,
           log_buffer[i].yaw_i_term,
           log_buffer[i].yaw_d_term,

           log_buffer[i].throttle,
           log_buffer[i].motor1,
           log_buffer[i].motor2,
           log_buffer[i].motor3,
           log_buffer[i].motor4,

           (unsigned long)log_buffer[i].valid_frame_count,
           log_buffer[i].receiver_valid,
           log_buffer[i].sensor_valid,
           log_buffer[i].arm_requested,
           log_buffer[i].armed
       );

       if ((len < 0) || (len >= sizeof(line)))
       {
           f_close(&file);
           f_mount(NULL, "0:", 1);
           return;
       }
       else
       {
           res = f_write(&file, line, len, &bytes_written);
       }

       if ((res != FR_OK) || (bytes_written != len))
       {
           f_close(&file);
           f_mount(NULL, "0:", 1);
           return;
       }

   }


   f_close(&file);
   f_mount(NULL, "0:", 1);

   Logger_Reset();

   return;
}



