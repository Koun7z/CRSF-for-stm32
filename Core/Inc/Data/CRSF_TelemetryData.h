/*
 * CRSF_TelemetryData.h
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */

#ifndef INC_CRSF_TELEMETRYDATA_H_
#define INC_CRSF_TELEMETRYDATA_H_

#include <stdint.h>

typedef struct __attribute__((packed))
{
	int32_t Latitude;
	int32_t Longitude;
	int16_t GroundSpeed;
	int16_t GroundCourse;
	uint16_t Altitude;
	uint8_t SatelliteCount;
} CRSF_GPSData;

typedef struct __attribute__((packed))
{
	int Voltage          : 16;
	int Current          : 16;
	int UsedCapacity     : 24;
	int BatteryRemaining : 8;
} CRSF_BatteryData;

typedef struct __attribute__((packed))
{
	unsigned Unit          : 1;
	unsigned Altitude      : 15;
	unsigned VerticalSpeed : 16;
} CRSF_BarometerData;

#endif /* INC_CRSF_TELEMETRYDATA_H_ */
