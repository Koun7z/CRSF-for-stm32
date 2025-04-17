/*
 * CRSF_CRC_8.h
 *
 *  Created on: Jan 12, 2025
 *      Author: pwoli
 */

#ifndef INC_CRC8_H_
#define INC_CRC8_H_

#include "CRSF_HAL_Include.h"

#define CRSF_CRC_HARD 0

#define CRC8_POLYNOMIAL 0xd5
#define CRC8_INIT       0x00

void CRSF_CRC_Init(void);

uint8_t CRSF_CRC_Calculate(const uint8_t* data, uint8_t length);

#endif /* INC_CRC8_H_ */
