/*
 * CRSF_Connection.c
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */


#include "CRSF_Connection.h"

#define CRSF_RX_SYNC_BYTE CRSF_RX_Buffer[0]
#define CRSF_RX_MSG_LEN CRSF_RX_Buffer[1]
#define CRSF_RX_FRAME_TYPE CRSF_RX_Buffer[2]
#define CRSF_RX_DATA_BEGIN &(CRSF_RX_Buffer[3])
#define CRSF_RX_DATA_LEN (CRSF_RX_MSG_LEN - 2)
#define CRSF_RX_CRC_BEGIN &(CRSF_RX_Buffer[2])

#define CRSF_TX_SYNC_BYTE CRSF_TX_Buffer[0]
#define CRSF_TX_MSG_LEN CRSF_TX_Buffer[1]
#define CRSF_TX_FRAME_TYPE CRSF_TX_Buffer[2]
#define CRSF_TX_DATA_BEGIN &(CRSF_TX_Buffer[3])
#define CRSF_TX_DATA_LEN (CRSF_TX_MSG_LEN - 2)
#define CRSF_TX_CRC_BEGIN &(CRSF_TX_Buffer[2])
#define CRSF_TX_CRC CRSF_TX_Buffer[CRSF_TX_MSG_LEN + 1]
#define CRSF_TX_DEST CRSF_TX_Buffer[3]
#define CRSF_TX_SRC CRSF_TX_Buffer[4]

UART_HandleTypeDef* _uart;
CRC_HandleTypeDef* _crc;

uint8_t CRSF_RX_Buffer[64];
uint8_t CRSF_TX_Buffer[64];

uint32_t CRSF_LastPacket = 0;
uint32_t CRSF_PPS = 0;


struct CRSF_ChannelsPacked CRSF_Channels;
struct CRSF_LinkStatistics CRSF_LinkState;

bool CRSF_NewData = false;

static bool _sendData(UART_HandleTypeDef* huart, void* data, uint32_t nbytes)
{

	HAL_StatusTypeDef status = HAL_UART_Transmit_IT(huart, data, nbytes);

	switch(status)
	{
		case 0:
			return true;
		default:
			//TODO: err logging
			return false;
	}

	return false;
}

static bool _sendTele()
{
	HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(_uart, CRSF_TX_Buffer, CRSF_TX_MSG_LEN + 2);

	switch(status)
	{
		case 0:
			return true;
		default:
			//TODO: err logging
			return false;
	}

	return false;
}

static void _receptionComplete()
{
	CRSF_RX_SYNC_BYTE = 0;
	CRSF_RX_MSG_LEN = 0;
	CRSF_NewData = false;

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
			break;

		case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
			memcpy(&CRSF_Channels, CRSF_RX_DATA_BEGIN, CRSF_RX_DATA_LEN);
			break;
		case CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED:
		case CRSF_FRAMETYPE_DEVICE_PING:
			printf("TODO: %u\n", CRSF_RX_FRAME_TYPE);
			break;
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
			printf("TODO: %u\n", CRSF_RX_FRAME_TYPE);
			break;

		default:
			printf("Unhandled pc %u\n", CRSF_RX_FRAME_TYPE);
			break;
	}
}

void CRSF_HandleRX(UART_HandleTypeDef *huart)
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

	if(HAL_CRC_Calculate(_crc, CRSF_RX_CRC_BEGIN, CRSF_RX_MSG_LEN) != 0)
	{
		printf("CRC-err\n");
		_receptionComplete();
		return;
	}

	CRSF_LastPacket = HAL_GetTick();
	static uint32_t _packetCount = 0;
	static uint32_t _lastAvg = 0;

	_packetCount++;
	if(HAL_GetTick() - _lastAvg >= 1000)
	{
		CRSF_PPS = _packetCount;
		_packetCount = 0;
		_lastAvg = HAL_GetTick();
		//printf("PPS: %lu\n", CRSF_PPS);
	}

	_parseData();

	_receptionComplete();
}

bool CRSF_SendGPS(struct CRSF_GPSData* gps)
{
	return _sendData(_uart, gps, sizeof(struct CRSF_GPSData));
}

bool CRSF_SendBatteryData(struct CRSF_BatteryData* bat)
{
	CRSF_TX_SYNC_BYTE = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN = sizeof(struct CRSF_BatteryData) + 2;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_BATTERY_SENSOR;
	struct CRSF_BatteryData* buff = CRSF_TX_DATA_BEGIN;

	buff->Voltage = __REV16(bat->Voltage);
	buff->Current = __REV16(bat->Current);
	buff->UsedCapacity = __REV(bat->UsedCapacity) >> 8;
	buff->BateryRemaining = bat->BateryRemaining;

	CRSF_TX_CRC = HAL_CRC_Calculate(_crc, CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	return _sendTele();
}

bool CRSF_SendPing()
{
	CRSF_TX_SYNC_BYTE = CRSF_SYNC_DEFAULT;
	CRSF_TX_MSG_LEN = 3;
	CRSF_TX_FRAME_TYPE = CRSF_FRAMETYPE_DEVICE_PING;
	CRSF_TX_DEST = CRSF_ADDRESS_BROADCAST;
	CRSF_TX_SRC = CRSF_ADDRESS_FLIGHT_CONTROLLER;
	CRSF_TX_CRC = HAL_CRC_Calculate(_crc, CRSF_TX_CRC_BEGIN, CRSF_TX_MSG_LEN + 1);

	return _sendTele();
}

void CRSF_HandleTX()
{

}


void CRSF_HandleErr(UART_HandleTypeDef* huart)
{
	if(huart != _uart)
	{
		return;
	}

	uint32_t err = huart->ErrorCode;;
	printf("err: %lu\n", err);

	_receptionComplete();
}

void CRSF_Init(UART_HandleTypeDef* huart, CRC_HandleTypeDef* hcrc)
{
	_uart = huart;
	_crc = hcrc;
	_receptionComplete();
}
