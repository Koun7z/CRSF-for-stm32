/*
 * CRSF_CRC_8.h
 *
 *  Created on: Jan 12, 2025
 *      Author: pwoli
 */

#ifndef INC_CRC8_H_
#define INC_CRC8_H_

#define HARD_CRC 0

#define CRC8_POLYNOMIAL 0xd5
#define CRC8_INIT       0x00


#if !HARD_CRC
static uint8_t CRC8Table[256];
#endif

#if HARD_CRC
void CRSF_CRC_Init(CRC_HandleTypeDef* hcrc);
#else

void CRSF_CRC_Init(void);
#endif

uint8_t CRSF_CRC_Calculate(const uint8_t* data, uint8_t length);

#endif /* INC_CRC8_H_ */
