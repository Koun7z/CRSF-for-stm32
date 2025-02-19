/*
 * CRSF_HandsetData.h
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */

#ifndef INC_CRSF_HANDSETDATA_H_
#define INC_CRSF_HANDSETDATA_H_

#include <stdint.h>

struct __attribute__((packed)) CRSF_ChannelsPacked
{
	unsigned Ch1  : 11;
	unsigned Ch2  : 11;
	unsigned Ch3  : 11;
	unsigned Ch4  : 11;
	unsigned Ch5  : 11;
	unsigned Ch6  : 11;
	unsigned Ch7  : 11;
	unsigned Ch8  : 11;
	unsigned Ch9  : 11;
	unsigned Ch10 : 11;
	unsigned Ch11 : 11;
	unsigned Ch12 : 11;
	unsigned Ch13 : 11;
	unsigned Ch14 : 11;
	unsigned Ch15 : 11;
	unsigned Ch16 : 11;
	uint8_t armStatus;  // optional ExpressLRS 4.0
};

struct __attribute__((packed)) CRSF_LinkStatistics
{
	uint8_t UplinkRSSI_Ant1;
	uint8_t UplinkRSSI_Ant2;
	uint8_t UplinklinkQuality;
	int8_t UplinkSNR;
	uint8_t DivActiveAnt;
	uint8_t RFMode;
	uint8_t UplinkTXPow;
	uint8_t DownlinkRSSI;
	uint8_t DownlinkQuality;
	int8_t DownlinkSNR;
};

#endif /* INC_CRSF_HANDSETDATA_H_ */
