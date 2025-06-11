//
// Created by pwoli on 07.06.2025.
//
#include "CRSF_Config.h"
#include "CRSF_Connection.h"
#include "CRSF_CommonTypes.h"
#include "CRSF_CRC_8.h"

#include <string.h>

// Telemetry buffer
#define CRSF_TX_SYNC_BYTE  CRSF_TX_Buffer[0]
#define CRSF_TX_MSG_LEN    CRSF_TX_Buffer[1]
#define CRSF_TX_FRAME_TYPE CRSF_TX_Buffer[2]
#define CRSF_TX_DATA_BEGIN &(CRSF_TX_Buffer[3])
#define CRSF_TX_DATA_LEN   (CRSF_TX_MSG_LEN - 2)
#define CRSF_TX_CRC_BEGIN  &(CRSF_TX_Buffer[2])
#define CRSF_TX_CRC        CRSF_TX_Buffer[CRSF_TX_MSG_LEN + 1]
#define CRSF_TX_DEST       CRSF_TX_Buffer[3]
#define CRSF_TX_SRC        CRSF_TX_Buffer[4]

// TX statistics
bool CRSF_TelemetryQueued = false;

uint8_t CRSF_TX_Buffer[64];

HAL_StatusTypeDef CRSF_SendTelemetry(UART_HandleTypeDef* huart)
{
	const HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(huart, CRSF_TX_Buffer, CRSF_TX_MSG_LEN + 2);

	if(status != 0)
	{
		// TODO: handle err
		DEBUG_LOG("Tele: %d\n", status);
	}

	CRSF_TelemetryQueued = false;

	return status;
}

void CRSF_QueueGPSData(const CRSF_GPSData* gps)
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(CRSF_GPSData) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_GPS;

	CRSF_GPSData* buff = (CRSF_GPSData*)CRSF_TX_DATA_BEGIN;

	buff->Latitude       = __REV(gps->Latitude);
	buff->Longitude      = __REV(gps->Longitude);
	buff->GroundSpeed    = __REV16(gps->GroundSpeed);
	buff->GroundCourse   = __REV16(gps->GroundCourse);
	buff->SatelliteCount = gps->SatelliteCount;

	CRSF_TX_CRC = CRSF_CRC_Calculate(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;
}

void CRSF_QueueBatteryData(const CRSF_BatteryData* battery)
{
	typedef struct __attribute__((packed))
	{
		int16_t Voltage;
		int16_t Current;
		uint8_t UsedCapacity[3];
		int8_t BatteryRemaining;
	} sBat;

	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(sBat) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_BATTERY_SENSOR;

	sBat* buff = (sBat*)CRSF_TX_DATA_BEGIN;

	buff->Voltage = __REV16(battery->Voltage);
	buff->Current = __REV16(battery->Current);

	const uint32_t uc     = battery->UsedCapacity;
	buff->UsedCapacity[0] = (uc >> 16) & 0xFF;
	buff->UsedCapacity[1] = (uc >> 8) & 0xFF;
	buff->UsedCapacity[2] = (uc >> 0) & 0xFF;

	buff->BatteryRemaining = battery->BatteryRemaining;

	CRSF_TX_CRC = CRSF_CRC_Calculate(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;
}

void CRSF_QueueVariometerData(const int16_t climb)
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(climb) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_VARIO;

	const uint16_t data = __REV16(climb);
	memcpy(CRSF_TX_DATA_BEGIN, &data, sizeof(data));

	CRSF_TX_CRC = CRSF_CRC_Calculate(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);
	CRSF_TelemetryQueued = true;
}

void CRSF_QueueBarometerData(const CRSF_BarometerData* barometer)
{
	typedef struct __attribute__((packed))
	{
		uint16_t Altitude;
		uint16_t Heading;
	} sBaro;

	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(sBaro) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_BARO_ALTITUDE;

	sBaro* buff = (sBaro*)CRSF_TX_DATA_BEGIN;
	if (barometer->Altitude <= 22767)
	{
		if (barometer->Altitude >= -10000)
		{
			buff->Altitude =__REV16(barometer->Altitude + 10000);
		}
		else
		{
			buff->Altitude = 0;
		}
	}
	else
	{
		buff->Altitude = (uint16_t)__REV16(barometer->Altitude / 10) | 0x8000;
	}

	CRSF_TX_CRC = CRSF_CRC_Calculate(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);
	CRSF_TelemetryQueued = true;
}

void CRSF_QueuePing()
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = 3;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_DEVICE_PING;
	CRSF_TX_DEST       = CRSF_ADDRESS_BROADCAST;
	CRSF_TX_SRC        = CRSF_ADDRESS_FLIGHT_CONTROLLER;
	CRSF_TX_CRC        = CRSF_CRC_Calculate(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;
}