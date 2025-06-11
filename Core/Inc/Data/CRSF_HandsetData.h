/*
 * CRSF_HandsetData.h
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */

#ifndef INC_CRSF_HANDSETDATA_H_
#define INC_CRSF_HANDSETDATA_H_

#include <stdint.h>

typedef struct __attribute__((packed)) CRSF_LinkStatistics
{
	uint8_t UplinkRSSI_Ant1;
	uint8_t UplinkRSSI_Ant2;
	uint8_t UpLinkQuality;
	int8_t UplinkSNR;
	uint8_t DivActiveAnt;
	uint8_t RFMode;
	uint8_t UplinkTXPow;
	uint8_t DownlinkRSSI;
	uint8_t DownlinkQuality;
	int8_t DownlinkSNR;
} CRSF_LinkStatistics;

#endif /* INC_CRSF_HANDSETDATA_H_ */
