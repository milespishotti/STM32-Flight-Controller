/*
 * SDCard.c
 *
 *  Created on: Aug 15, 2026
 *      Author: miles
 */

#include "SDCard.h"

static uint8_t test_block[512];
static uint8_t readback_block[512];

static void SD_CreateTestBuffer(void)
{
    for (int i = 0; i < 512; i++)
    {
        test_block[i] = i & 0xFF;
    }
}



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


bool SDCard_WriteBlock(uint32_t block_number, const uint8_t *data)
{

    uint8_t cmd24_response = SD_SendCommand(24, block_number, 0x01);
    uint8_t dummy = 0xFF;
           uint8_t dummy_rx = 0xFF;
           uint8_t data_token = 0xFE;

    if (cmd24_response == 0x00)
    {



        HAL_SPI_Transmit(&hspi2, &dummy, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi2, &data_token, 1, HAL_MAX_DELAY);

        HAL_SPI_Transmit(&hspi2, data, 512, HAL_MAX_DELAY);

        uint8_t crc[2] = {0xFF, 0xFF};

        HAL_SPI_Transmit(&hspi2, crc, 2, HAL_MAX_DELAY);

        uint8_t response = 0xFF;

        HAL_SPI_TransmitReceive(&hspi2, &dummy, &response, 1, HAL_MAX_DELAY);

        if ((response & 0x1F) != 0x05)
        {
            SD_Deselect();
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
            return false;
        }

        uint32_t start = HAL_GetTick();

        while ((HAL_GetTick() - start) < 1000)
        {
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &response, 1, HAL_MAX_DELAY);

            if (response == 0xFF)
            {
                SD_Deselect();
                HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
                return true;
            }

        }
        SD_Deselect();
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
    }
    SD_Deselect();
    HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
    return false;
}


bool SDCard_ReadBlock(uint32_t block_number, uint8_t *data)
{
    uint8_t cmd17_response = SD_SendCommand(17, block_number, 0x01);

    uint8_t dummy = 0xFF;
            uint8_t dummy_rx = 0xFF;
            uint8_t data_token = 0xFE;
            uint8_t response = 0xFF;

    if (cmd17_response == 0x00)
    {


        bool data_incoming = false;

        uint32_t start = HAL_GetTick();

        while ((HAL_GetTick() - start) < 1000)
        {

            HAL_SPI_TransmitReceive(&hspi2, &dummy, &response, 1, HAL_MAX_DELAY);

            if (response == data_token)
            {
                data_incoming = true;
                break;
            }
        }

        if (data_incoming == true)
        {
            uint8_t dummy_tx[512];
            memset(dummy_tx, 0xFF, sizeof(dummy_tx));

            HAL_SPI_TransmitReceive(&hspi2, dummy_tx, data, 512, HAL_MAX_DELAY);

            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

            SD_Deselect();
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

            return true;

        }

        SD_Deselect();
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
        return false;
    }
    SD_Deselect();
    HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
    return false;
}

bool SDCard_TestBlockIO(void)
{
    SD_CreateTestBuffer();

    if (!SDCard_WriteBlock(10000, test_block))
    {
        return false;
    }

    if (!SDCard_ReadBlock(10000, readback_block))
    {
        return false;
    }

    for (int i = 0; i < 512; i++)
    {
        if (test_block[i] != readback_block[i])
        {
            return false;
        }
    }

    return true;
}


static bool SD_ReadCSD(uint8_t csd[16])
{
    uint8_t cmd9_response = SD_SendCommand(9, 0x00000000, 0x01);

    uint8_t dummy = 0xFF;
    uint8_t dummy_rx = 0xFF;
    uint8_t data_token = 0xFE;
    uint8_t response = 0xFF;

    if (cmd9_response == 0x00)
    {

        bool data_incoming = false;

        uint32_t start = HAL_GetTick();

        while ((HAL_GetTick() - start) < 1000)
        {

            HAL_SPI_TransmitReceive(&hspi2, &dummy, &response, 1, HAL_MAX_DELAY);

            if (response == data_token)
            {
                data_incoming = true;
                break;
            }
        }

        if (data_incoming == true)
        {
            for (int i = 0; i < 16; i++)
            {

                HAL_SPI_TransmitReceive(&hspi2, &dummy, &csd[i], 1, HAL_MAX_DELAY);
            }

            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

            SD_Deselect();
            HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

            return true;
        }

        SD_Deselect();
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

        return false;

    }

    SD_Deselect();
    HAL_SPI_TransmitReceive(&hspi2, &dummy, &dummy_rx, 1, HAL_MAX_DELAY);

    return false;
}

bool SDCard_GetSectorCount(uint32_t *sector_count)
{
    uint8_t csd[16];

    if (!SD_ReadCSD(csd))
    {
        return false;
    }

    // Verify CSD version 2.0
    if ((csd[0] & 0xC0) != 0x40)
    {
        return false;
    }

    uint32_t c_size =
        ((uint32_t)(csd[7] & 0x3F) << 16) |
        ((uint32_t)csd[8] << 8) |
        csd[9];

    *sector_count = (c_size + 1) * 1024;

    return true;
}

