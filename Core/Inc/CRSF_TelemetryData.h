/*
 * CRSF_TelemetryData.h
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */

#ifndef INC_CRSF_TELEMETRYDATA_H_
#define INC_CRSF_TELEMETRYDATA_H_

struct __attribute__((packed)) CRSF_GPSData
{
	int32_t Latitude;
	int32_t Longitude;
	int16_t GroundSpeed;
	int16_t GroundCourse;
	uint16_t Altitue;
	uint8_t SatelliteCount;
};

struct __attribute__((packed)) CRSF_BatteryData
{
	int Voltage          : 16;
	int Current          : 16;
	int UsedCapacity     : 24;
	int BatteryRemaining : 8;
};

struct __attribute__((packed)) CRSF_BarometerData
{
	unsigned Unit          : 1;
	unsigned Altitude      : 15;
	unsigned VerticalSpeed : 16;
};

#endif /* INC_CRSF_TELEMETRYDATA_H_ */
