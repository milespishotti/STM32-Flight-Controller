/*
 * SDCard.c
 *
 *  Created on: Aug 15, 2026
 *      Author: miles
 */

#include "SDCard.h"


static void SD_Select(void)
{
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void SD_Deselect(void)
{
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}


static uint8_t SD_SendCommand(uint8_t command_number, uint32_t argument, uint8_t crc)
{
    uint8_t dummy = 0xFF;
    uint8_t rx = 0xFF;

    uint8_t cmd[6] = {0};

    cmd[0] = 0x40 | command_number;
    cmd[1] = (argument >> 24) & 0xFF;
    cmd[2] = (argument >> 16) & 0xFF;
    cmd[3] = (argument >> 8)  & 0xFF;
    cmd[4] = argument & 0xFF;
    cmd[5] = crc;

    SD_Select();

    HAL_SPI_Transmit(&hspi2, cmd, 6, HAL_MAX_DELAY);

    for (int i = 0; i < 8; i ++)
    {
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);

        if ((rx & 0x80) == 0)
        {
            return rx;
        }
    }
    return 0xFF;

}


bool SDCard_Init(void)
{
        uint8_t dummy = 0xFF;
        uint8_t rx = 0xFF;

          // Card deselected: provide startup clocks
          HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

          for (int i = 0; i < 10; i++)
          {
              HAL_SPI_Transmit(&hspi2, &dummy, 1, HAL_MAX_DELAY);
          }

          uint8_t cmd0_response = SD_SendCommand(0, 0x00000000, 0x95);

          SD_Deselect();
          HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);

          printf("CMD0 response: 0x%02X\r\n", cmd0_response);


          uint8_t cmd8_response[5] = {0};

          cmd8_response[0] = SD_SendCommand(8, 0x000001AA, 0x87);


          if (cmd8_response[0] == 0x01)
          {
              for (int i = 1; i < 5; i ++)
              {
                  HAL_SPI_TransmitReceive(&hspi2, &dummy, &cmd8_response[i], 1, HAL_MAX_DELAY);
              }
          }

          SD_Deselect();
          HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);


          printf("CMD8: %02X %02X %02X %02X %02X\r\n",
                 cmd8_response[0],
                 cmd8_response[1],
                 cmd8_response[2],
                 cmd8_response[3],
                 cmd8_response[4]);


          bool amcd41_success = false;

          uint8_t acmd41_response = 0xFF;
          uint8_t cmd55_response = 0xFF;

          uint32_t start = HAL_GetTick();


          while ((HAL_GetTick() - start) < 1000)
          {

              cmd55_response = SD_SendCommand(55, 0x00000000, 0x01);

              SD_Deselect();
              HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);

              acmd41_response = SD_SendCommand(41, 0x40000000, 0x01);

              SD_Deselect();
              HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);

              if (acmd41_response == 0x00)
              {
                  amcd41_success = true;
                  break;
              }

              HAL_Delay(1);

          }

          printf("CMD55 response: 0x%02X\r\n", cmd55_response);
          printf("ACMD41 response: 0x%02X\r\n", acmd41_response);
          printf("ACMD41 success: %s\r\n", amcd41_success ? "YES" : "NO");


          uint8_t cmd58_response[5] = {0};

          cmd58_response[0] = SD_SendCommand(58, 0x00000000, 0x01);



          if (cmd58_response[0] == 0x00)
          {
              for (int i = 1; i < 5; i ++)
              {
                  HAL_SPI_TransmitReceive(&hspi2, &dummy, &cmd58_response[i], 1, HAL_MAX_DELAY);
              }
          }

          SD_Deselect();
          HAL_SPI_TransmitReceive(&hspi2, &dummy, &rx, 1, HAL_MAX_DELAY);

          bool high_capacity = (cmd58_response[1] & 0x40) != 0;

          printf("High capacity: %s\r\n", high_capacity ? "YES" : "NO");
          printf("CMD58: %02X %02X %02X %02X %02X\r\n",
                  cmd58_response[0],
                  cmd58_response[1],
                  cmd58_response[2],
                  cmd58_response[3],
                  cmd58_response[4]);
          return true;





}


