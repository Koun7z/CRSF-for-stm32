//
// Created by pwoli on 17.04.2025.
//
#include "CRSF_CRC_8.h"
#include "CRSF_HAL_Include.h"

#if CRSF_CRC_HARD
CRC_HandleTypeDef hcrc;
#else
static uint8_t CRC8Table[256];
#endif

void CRSF_CRC_Init(void)
{
#if CRSF_CRC_HARD
	hcrc.Instance = CRC;
	CRC->POL = CRC8_POLYNOMIAL;
	CRC->INIT = CRC8_INIT;
#else
	for(size_t i = 0; i < 256; i++)
	{
		uint8_t crc = i;
		for(uint8_t j = 0; j < 8; j++)
		{
			if(crc & 0x80)
			{
				crc = (crc << 1) ^ CRC8_POLYNOMIAL;
			}
			else
			{
				crc <<= 1;
			}
		}
		CRC8Table[i] = crc & 0xff;
	}
#endif
}

uint8_t CRSF_CRC_Calculate(const uint8_t* data, uint8_t length)
{
#if CRSF_CRC_HARD
	return HAL_CRC_Calculate(&hcrc, data, length);
#else
	uint8_t crc = CRC8Table[0 ^ data[0]];

	for(uint8_t i = 1; i < length; i++)
	{
		crc = CRC8Table[crc ^ data[i]];
	}
	return crc;
#endif
}