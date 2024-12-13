/*
 * CRSF_Connection.c
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */


#include "CRSF_Connection.h"

UART_HandleTypeDef* _uart;
CRC_HandleTypeDef* _crc;

uint8_t CRSF_Buffer[64];

uint32_t CRSF_LastChannelsPacked = 0;
struct CRSF_ChannelsPacked CRSF_Channels;

bool CRSF_NewData = false;

static void _receptionComplete()
{
	CRSF_SYNC_BYTE = 0;
	CRSF_MSG_LEN = 0;
	CRSF_NewData = false;

	HAL_UARTEx_ReceiveToIdle_DMA(_uart, CRSF_Buffer, 64);
	__HAL_DMA_DISABLE_IT(_uart->hdmarx, DMA_IT_HT);
}

static void _parseData()
{
	switch(CRSF_FRAME_TYPE)
	{
		case CRSF_FRAMETYPE_HEARTBEAT:
		case CRSF_FRAMETYPE_LINK_STATISTICS:
			printf("TODO: %u\n", CRSF_FRAME_TYPE);
			break;

		case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
			memcpy(&CRSF_Channels, CRSF_DATA_BEGIN, CRSF_DATA_LEN);
			CRSF_LastChannelsPacked = HAL_GetTick();
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
			printf("TODO: %u\n", CRSF_FRAME_TYPE);
			break;

		default:
			printf("Unhandled pc %u\n", CRSF_FRAME_TYPE);
			break;
	}
}

void CRSF_HandleRX(UART_HandleTypeDef *uart)
{
	if(uart != _uart)
	{
		return;
	}

	if(CRSF_SYNC_BYTE != CRSF_SYNC_DEFAULT && CRSF_SYNC_BYTE != CRSF_SYNC_EDGE_TX)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(_uart, CRSF_Buffer, 64);
		__HAL_DMA_DISABLE_IT(_uart->hdmarx, DMA_IT_HT);
		return;
	}

	if(HAL_CRC_Calculate(_crc, CRSF_CRC_BEGIN, CRSF_MSG_LEN) != 0)
	{
		printf("CRC-err\n");
		_receptionComplete();
		return;
	}

	_parseData();

	_receptionComplete();
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
