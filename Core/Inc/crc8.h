/*
 * crc8.h
 *
 *  Created on: Jan 12, 2025
 *      Author: pwoli
 */

#ifndef INC_CRC8_H_
#define INC_CRC8_H_

#define CRC8_POLYNOMIAL 0xd5
#define CRC8_INIT 0x00

static uint8_t CRC8Table[256];

void CRC_GenerateTable8()
{
	for (uint16_t i = 0; i < 256; i++)
	{
		uint8_t crc = i;
	    for (uint8_t j = 0; j < 8; j++)
	    {
	    	if (crc & 0x80)
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
}

uint8_t CRC_CalculateCRC8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = CRC8Table[0 ^ data[0]];

    for (uint8_t i = 1; i < length; i++)
    {
        crc = CRC8Table[crc ^ data[i]];
    }
    return crc;
}

#endif /* INC_CRC8_H_ */
