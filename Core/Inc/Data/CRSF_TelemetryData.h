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
	int32_t Latitude;       // [deg * 1e7]
	int32_t Longitude;      // [deg * 1e7]
	int16_t GroundSpeed;    // [km/h]
	int16_t GroundCourse;   // [deg * 100]
	uint16_t Altitude;      // [meters + 1000m]
	uint8_t SatelliteCount;
} CRSF_GPSData;

typedef struct
{
	int16_t Voltage;          // [dV <- It's a deci-volt]
	int16_t Current;          // [dA]
	uint32_t UsedCapacity;    // [mAh]
	int32_t BatteryRemaining; // [%]
} CRSF_BatteryData;

typedef struct
{
	int32_t Altitude;      // [dm]
	int32_t VerticalSpeed; // [cm / s]
} CRSF_BarometerData;

#endif /* INC_CRSF_TELEMETRYDATA_H_ */
