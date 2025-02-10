/*
 * CRSF_Connection.c
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */
#include "CRSF_Connection.h"

#include "crc8.h"


// RX buffer
#define CRSF_RX_SYNC_BYTE  CRSF_RX_Buffer[0]
#define CRSF_RX_MSG_LEN    CRSF_RX_Buffer[1]
#define CRSF_RX_FRAME_TYPE CRSF_RX_Buffer[2]
#define CRSF_RX_DATA_BEGIN &(CRSF_RX_Buffer[3])
#define CRSF_RX_DATA_LEN   (CRSF_RX_MSG_LEN - 2)
#define CRSF_RX_CRC_BEGIN  &(CRSF_RX_Buffer[2])

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

// Arming behaviour
#define CRSF_HANDLE_ARM  1
#define CRSF_ARM_CHANNEL CRSF_Channels.Ch5
#define CRSF_ARM_DELAY   5  // Packet num

// Global data structures
struct CRSF_ChannelsPacked CRSF_Channels;
struct CRSF_LinkStatistics CRSF_LinkState;

// RX statistics
uint32_t CRSF_PPS                = 0;
uint32_t CRSF_LastChannelsPacked = 0;
bool CRSF_ArmStatus              = false;

// TX statistics
bool CRSF_TelemetryQueued = false;

// Internal data
UART_HandleTypeDef* _uart;

uint8_t CRSF_RX_Buffer[64];
uint8_t CRSF_TX_Buffer[64];

uint32_t CRSF_LastPacket = 0;
uint32_t _packetCount    = 0;

// Event callbacks
__weak void CRSF_OnChannelsPacked() {}
__weak void CRSF_OnLinkStatistics() {}

HAL_StatusTypeDef CRSF_SendTelemetry()
{
	HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(_uart, CRSF_TX_Buffer, CRSF_TX_MSG_LEN + 2);

	if(status != 0)
	{
		// TODO: handle err
		DEBUG_LOG("Tele: %d\n", status);
	}

	CRSF_TelemetryQueued = false;

	return status;
}

static void _receptionComplete()
{
	CRSF_RX_SYNC_BYTE = 0;
	CRSF_RX_MSG_LEN   = 0;

	HAL_UARTEx_ReceiveToIdle_DMA(_uart, CRSF_RX_Buffer, 64);
	__HAL_DMA_DISABLE_IT(_uart->hdmarx, DMA_IT_HT);
}

static void _parseData()
{
	switch(CRSF_RX_FRAME_TYPE)
	{
		case CRSF_FRAMETYPE_HEARTBEAT:
		case CRSF_FRAMETYPE_LINK_STATISTICS:
			memcpy(&CRSF_LinkState, CRSF_RX_DATA_BEGIN, CRSF_RX_DATA_LEN);

			CRSF_OnLinkStatistics();

			DEBUG_LOG("RQly: %lu\n", CRSF_LinkState.UplinklinkQuality);
			break;

		case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
			memcpy(&CRSF_Channels, CRSF_RX_DATA_BEGIN, CRSF_RX_DATA_LEN);

#if CRSF_HANDLE_ARM
			static uint8_t armCtr = 0;
			bool arm              = (bool)(CRSF_ARM_CHANNEL > 1000);
			if(CRSF_ArmStatus != arm)
			{
				armCtr++;
				if(armCtr > CRSF_ARM_DELAY)
				{
					CRSF_ArmStatus = arm;
				}
			}
			else
			{
				arm = 0;
			}
#endif

			CRSF_LastChannelsPacked      = HAL_GetTick();
			static uint16_t _packetCount = 0;
			static uint32_t _lastAvg     = 0;
			if(HAL_GetTick() - _lastAvg >= 1000)
			{
				CRSF_PPS     = _packetCount;
				_packetCount = 0;
				_lastAvg     = HAL_GetTick();
				DEBUG_LOG("PPS: %lu\n", CRSF_PPS);
			}

			_packetCount++;
			CRSF_OnChannelsPacked();

			break;
		case CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED:
		case CRSF_FRAMETYPE_DEVICE_PING:
		case CRSF_FRAMETYPE_DEVICE_INFO:
		case CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY:
		case CRSF_FRAMETYPE_PARAMETER_READ:
		case CRSF_FRAMETYPE_PARAMETER_WRITE:
		case CRSF_FRAMETYPE_ELRS_STATUS:
		case CRSF_FRAMETYPE_COMMAND:
		case CRSF_FRAMETYPE_RADIO_ID:
		case CRSF_FRAMETYPE_KISS_REQ:
		case CRSF_FRAMETYPE_KISS_RESP:
		case CRSF_FRAMETYPE_MSP_REQ:
		case CRSF_FRAMETYPE_MSP_RESP:
		case CRSF_FRAMETYPE_MSP_WRITE:
		case CRSF_FRAMETYPE_DISPLAYPORT_CMD:
		case CRSF_FRAMETYPE_ARDUPILOT_RESP:
			DEBUG_LOG("TODO: %u\n", CRSF_RX_FRAME_TYPE);
			break;

		default:
			DEBUG_LOG("Unhandled pc %u\n", CRSF_RX_FRAME_TYPE);
			break;
	}
}

void CRSF_HandleRX(UART_HandleTypeDef* huart)
{
	if(huart != _uart)
	{
		return;
	}

	if(CRSF_RX_SYNC_BYTE != CRSF_SYNC_DEFAULT && CRSF_RX_SYNC_BYTE != CRSF_SYNC_EDGE_TX)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(_uart, CRSF_RX_Buffer, 64);
		__HAL_DMA_DISABLE_IT(_uart->hdmarx, DMA_IT_HT);
		return;
	}

	if(CRC_CalculateCRC8(CRSF_RX_CRC_BEGIN, CRSF_RX_MSG_LEN) != 0)
	{
		DEBUG_LOG("CRC\n");
		_receptionComplete();
		return;
	}

	_parseData();

#if TELEMETRY_ENABLED
	CRSF_SendTelemetry();
#endif

	_receptionComplete();
}

bool CRSF_QueueGPSData(struct CRSF_GPSData* gps)
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(struct CRSF_GPSData) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_GPS;

	struct CRSF_GPSData* buff = CRSF_TX_DATA_BEGIN;

	buff->Latitude       = __REV(gps->Latitude);
	buff->Longitude      = __REV(gps->Longitude);
	buff->GroundSpeed    = __REV16(gps->GroundSpeed);
	buff->GroundCourse   = __REV16(gps->GroundCourse);
	buff->SatelliteCount = gps->SatelliteCount;

	CRSF_TX_CRC = CRC_CalculateCRC8(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;

	return true;
}

bool CRSF_QueueBatteryData(struct CRSF_BatteryData* bat)
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(struct CRSF_BatteryData) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_BATTERY_SENSOR;

	struct CRSF_BatteryData* buff = CRSF_TX_DATA_BEGIN;

	buff->Voltage          = __REV16(bat->Voltage);
	buff->Current          = __REV16(bat->Current);
	buff->UsedCapacity     = __REV(bat->UsedCapacity) >> 8;
	buff->BatteryRemaining = bat->BatteryRemaining;

	CRSF_TX_CRC = CRC_CalculateCRC8(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;
	return true;
}

bool CRSF_QueueVariometerData(int16_t climb)
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(climb) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_VARIO;

	int16_t* buff = CRSF_TX_Buffer;
	*buff         = __REV16(climb);

	CRSF_TX_CRC = CRC_CalculateCRC8(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;
	return true;
}

bool CRSF_QueueBarometerData()
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = sizeof(struct CRSF_BarometerData) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_BARO_ALTITUDE;

	// TODO

	return false;
}

bool CRSF_QueuePing()
{
	CRSF_TX_SYNC_BYTE  = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN    = 3;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_DEVICE_PING;
	CRSF_TX_DEST       = CRSF_ADDRESS_BROADCAST;
	CRSF_TX_SRC        = CRSF_ADDRESS_FLIGHT_CONTROLLER;
	CRSF_TX_CRC        = CRC_CalculateCRC8(CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	CRSF_TelemetryQueued = true;

	return true;
}

void CRSF_HandleTX() {}

void CRSF_HandleErr(UART_HandleTypeDef* huart)
{
	if(huart != _uart)
	{
		return;
	}

	uint32_t err = huart->ErrorCode;

	DEBUG_LOG("err: %lu\n", err);
	_receptionComplete();
}

void CRSF_Init(UART_HandleTypeDef* huart)
{
	_uart = huart;

	CRC_GenerateTable8();

	_receptionComplete();
}

void CRSF_DEBUG_PrintChannels()
{
	DEBUG_LOG("Ch1: %lu, Ch2: %lu, Ch3: %lu, Ch4: %lu\n", CRSF_Channels.Ch1, CRSF_Channels.Ch2, CRSF_Channels.Ch3,
	          CRSF_Channels.Ch4);
}
