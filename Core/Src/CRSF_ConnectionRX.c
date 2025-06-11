/*
 * CRSF_Connection.c
 *
 *  Created on: Dec 13, 2024
 *      Author: pwoli
 */

#include "CRSF_CommonTypes.h"
#include "CRSF_Config.h"
#include "CRSF_Connection.h"
#include "CRSF_CRC_8.h"

#include <stdio.h>
#include <string.h>


// RX buffer
#define CRSF_RX_SYNC_BYTE  CRSF_RX_Buffer[0]
#define CRSF_RX_MSG_LEN    CRSF_RX_Buffer[1]
#define CRSF_RX_FRAME_TYPE CRSF_RX_Buffer[2]
#define CRSF_RX_DATA_BEGIN &(CRSF_RX_Buffer[3])
#define CRSF_RX_DATA_LEN   (CRSF_RX_MSG_LEN - 2)
#define CRSF_RX_CRC_BEGIN  &(CRSF_RX_Buffer[2])

// Global data structures
uint32_t CRSF_Channels[16] = {0};
CRSF_LinkStatistics CRSF_LinkState = {0, 0, 100, 0, 0, 0, 0, 0, 0, 0};

// RX statistics
uint32_t CRSF_PPS                = 0;
uint32_t CRSF_LastChannelsPacked = 0;
bool CRSF_ArmStatus              = false;
bool CRSF_FailsafeStatus         = false;

// Internal data
UART_HandleTypeDef* uart;
uint8_t CRSF_RX_Buffer[64];

/*
** Event callbacks
*/

__attribute__((weak)) void CRSF_OnChannelsPacked() {}
__attribute__((weak)) void CRSF_OnLinkStatistics() {}
__attribute__((weak)) void CRSF_OnFailsafe() {}


/*
** Functions
*/

static void receptionReset(void)
{
	CRSF_RX_SYNC_BYTE = 0;

	HAL_UARTEx_ReceiveToIdle_DMA(uart, CRSF_RX_Buffer, 64);
	__HAL_DMA_DISABLE_IT(uart->hdmarx, DMA_IT_HT);
}

/**
 * @brief Unpacks 11-bit packed struct into uint32_t array
 */
static void unpackBitField(uint32_t* dst, const uint8_t* src)
{
	uint32_t read_bits = 0;
	uint32_t src_ptr = 0;
	uint32_t val = 0;
	for (uint32_t i = 0; i < 16; i++)
	{
		while (read_bits < 11)
		{
			val |= src[src_ptr++] << read_bits;
			read_bits += 8;
		}

		dst[i] = val & 0x7FF;
		val >>= 11;
		read_bits -= 11;
	}
}

static void parseData(void)
{
	switch(CRSF_RX_FRAME_TYPE)
	{
		case CRSF_FRAMETYPE_HEARTBEAT:
			break;
		case CRSF_FRAMETYPE_LINK_STATISTICS:
			memcpy(&CRSF_LinkState, CRSF_RX_DATA_BEGIN, sizeof(CRSF_LinkStatistics));

			CRSF_OnLinkStatistics();

#if CRSF_FAILSAFE_ENABLE
			if(CRSF_LinkState.UpLinkQuality < CRSF_LQ_FAILSAFE_THRESHOLD
			   || CRSF_LinkState.UplinkRSSI_Ant1 > CRSF_RSSI_FAILSAFE_THRESHOLD)
			{
				CRSF_FailsafeStatus = true;
				CRSF_OnFailsafe();
			}
#endif

			DEBUG_LOG("RQly: %hhu\n", CRSF_LinkState.UpLinkQuality);
			break;

		case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
			unpackBitField(CRSF_Channels, CRSF_RX_DATA_BEGIN);

#if CRSF_HANDLE_ARM
			static uint8_t armCtr = 0;

			const bool arm = (bool)(CRSF_ARM_CHANNEL > 1500);
			if(CRSF_ArmStatus != arm)
			{
				// To avoid unwonted arm status switching, change arm state
				// only after the value on rc channel persists for n frames (config).
				armCtr++;
				if(armCtr > CRSF_ARM_DELAY)
				{
					CRSF_ArmStatus = arm;
				}
			}
			else
			{
				armCtr = 0;
			}
#endif

			// Actual number of correctly received rc packets (from past second)
			CRSF_LastChannelsPacked      = HAL_GetTick();
			static uint32_t packetCount = 0;
			static uint32_t lastAvg     = 0;
			const uint32_t tick = HAL_GetTick();
			if(tick - lastAvg >= 1000)
			{
				CRSF_PPS     = packetCount;
				packetCount = 0;
				lastAvg     = tick;
				DEBUG_LOG("PPS: %lu\n", CRSF_PPS);
			}
			packetCount++;

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
			DEBUG_LOG("TODO: 0x%hhx\n", CRSF_RX_FRAME_TYPE);
			break;

		default:
			DEBUG_LOG("Unhandled type: 0x%hhx\n", CRSF_RX_FRAME_TYPE);
			break;
	}
}

void CRSF_Init(UART_HandleTypeDef* huart)
{
	uart = huart;

	CRSF_CRC_Init();

	CRSF_RX_SYNC_BYTE = 0;
	CRSF_RX_MSG_LEN   = 0;

	receptionReset();
}

void CRSF_HandleRX()
{
	if(CRSF_RX_SYNC_BYTE != CRSF_SYNC_DEFAULT && CRSF_RX_SYNC_BYTE != CRSF_SYNC_EDGE_TX)
	{
		receptionReset();
		return;
	}

	if(CRSF_CRC_Calculate(CRSF_RX_CRC_BEGIN, CRSF_RX_MSG_LEN) != 0)
	{
		DEBUG_LOG("CRC\n");
		receptionReset();
		return;
	}

	parseData();

#if TELEMETRY_ENABLED
	CRSF_SendTelemetry(uart);
#endif

	receptionReset();
}

void CRSF_HandleErr(void)
{
	DEBUG_LOG("err: %lu\n\r", uart->ErrorCode);
	receptionReset();
}

void CRSF_DEBUG_PrintChannels(uint32_t n_channels)
{
	if (n_channels > 16)
	{
		n_channels = 16;
	}

	printf("T: %lu", HAL_GetTick());
	for (uint32_t i = 0; i < n_channels; i++)
	{
		printf(", Ch%lu: %4lu", i, CRSF_Channels[i]);
	}
	printf("\n");
}
