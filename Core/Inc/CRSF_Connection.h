/*
 * CRSF_Connection.h
 *
 *  Created on: Dec 7, 2024
 *      Author: pwoli
 */
#ifndef INC_CRSF_CONNECTION_H_
#define INC_CRSF_CONNECTION_H_

#include "CRSF_HandsetData.h"
#include "CRSF_TelemetryData.h"

#include <stdbool.h>

#if defined(STM32F3)
#  include "stm32f3xx_hal.h"
#elif defined(STM32F4)
#  include "stm32f4xx_hal.h"
#elif defined(STM32F7)
#  include "stm32f7xx_hal.h"
#elif defined(STM32L4)
#  include "stm32l4xx_hal.h"
#elif defined(STM32L %)
#  include "stm32l5xx_hal.h"
#elif defined(STM32H7)
#  include "stm32h7xx_hal.h"
#elif defined(STM32G4)
#  include "stm32g4xx_hal.h"
#endif


// Telemetry config
#define TELEMETRY_ENABLED 0
#define SERIAL_DEBUG      0

// Arming behaviour
#define CRSF_HANDLE_ARM  1
#define CRSF_ARM_CHANNEL CRSF_Channels.Ch5
#define CRSF_ARM_DELAY   5  // Packet num

#if SERIAL_DEBUG
#  define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#  define DEBUG_LOG(...)
#endif


/**
 * @brief Crossfire protocol frame types
 */
typedef enum {
	CRSF_FRAMETYPE_GPS                       = 0x02,  // GPS position, ground speed, heading, altitude, satellite count
	CRSF_FRAMETYPE_VARIO                     = 0x07,  // Vertical speed
	CRSF_FRAMETYPE_BATTERY_SENSOR            = 0x08,  // Battery voltage, current, mAh, remaining percent
	CRSF_FRAMETYPE_BARO_ALTITUDE             = 0x09,  // Barometric altitude, vertical speed (optional)
	CRSF_FRAMETYPE_HEARTBEAT                 = 0x0B,  // (CRSFv3) Heartbeat
	CRSF_FRAMETYPE_LINK_STATISTICS           = 0x14,  // Signal info. RSSI, SNR, Link Quality (LQ), RF mode, power
	CRSF_FRAMETYPE_RC_CHANNELS_PACKED        = 0x16,  // Channels data (both handset to TX and RX to flight controller)
	CRSF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED = 0x17,  // (CRSFv3) Channels subset data
	CRSF_FRAMETYPE_LINK_RX_ID                = 0x1C,  // Receiver RSSI percent, power?
	CRSF_FRAMETYPE_LINK_TX_ID                = 0x1D,  // Transmitter RSSI percent, power, fps?
	CRSF_FRAMETYPE_ATTITUDE                  = 0x1E,  // Attitude: pitch, roll, yaw
	CRSF_FRAMETYPE_FLIGHT_MODE               = 0x21,  // Flight controller flight mode string
	CRSF_FRAMETYPE_DEVICE_PING               = 0x28,  // Sender requesting DEVICE_INFO from all destination devices
	CRSF_FRAMETYPE_DEVICE_INFO               = 0x29,  // Device name, firmware, hardware version, serial number
	CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY  = 0x2B,  // Configuration item data chunk
	CRSF_FRAMETYPE_PARAMETER_READ            = 0x2C,  // Configuration item read request
	CRSF_FRAMETYPE_PARAMETER_WRITE           = 0x2D,  // Configuration item write request
	CRSF_FRAMETYPE_ELRS_STATUS               = 0x2E,  // !!Non Standard!! ExpressLRS good/bad packet count, status flags
	CRSF_FRAMETYPE_COMMAND                   = 0x32,  // CRSF command execute
	CRSF_FRAMETYPE_RADIO_ID                  = 0x3A,  // Extended type used for OPENTX_SYNC
	CRSF_FRAMETYPE_KISS_REQ                  = 0x78,  // KISS request
	CRSF_FRAMETYPE_KISS_RESP                 = 0x79,  // KISS response
	CRSF_FRAMETYPE_MSP_REQ                   = 0x7A,  // MSP parameter request / command
	CRSF_FRAMETYPE_MSP_RESP                  = 0x7B,  // MSP parameter response chunk
	CRSF_FRAMETYPE_MSP_WRITE                 = 0x7C,  // MSP parameter write
	CRSF_FRAMETYPE_DISPLAYPORT_CMD           = 0x7D,  // (CRSFv3) MSP DisplayPort control command
	CRSF_FRAMETYPE_ARDUPILOT_RESP            = 0x80   // Ardupilot output?
} CRSF_FRAMETYPE;

/**
 * @brief Crossfire protocol extended mode adresses adresses
 */
typedef enum {
	CRSF_ADDRESS_BROADCAST         = 0x00,  // Broadcast (all devices process packet)
	CRSF_ADDRESS_USB               = 0x10,  // ?
	CRSF_ADDRESS_BLUETOOTH         = 0x12,  // Bluetooth module
	CRSF_ADDRESS_TBS_CORE_PNP_PRO  = 0x80,  // ?
	CRSF_ADDRESS_RESERVED1         = 0x8A,  // Reserved for one
	CRSF_ADDRESS_SENSOR            = 0xC0,  // External current sensor
	CRSF_ADDRESS_GPS               = 0xC2,  // External GPS
	CRSF_ADDRESS_TBS_BLACKBOX      = 0xC4,  // External Blackbox logging device
	CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,  // Flight Controller (Betaflight / iNav)
	CRSF_ADDRESS_RESERVED2         = 0xCA,  // Reserved, for two
	CRSF_ADDRESS_RACE_TAG          = 0xCC,  // Race tag?
	CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,  // Handset (EdgeTX), not transmitter
	CRSF_ADDRESS_CRSF_RECEIVER     = 0xEC,  // Receiver hardware (TBS Nano RX / RadioMaster RP1)
	CRSF_ADDRESS_CRSF_TRANSMITTER  = 0xEE,  // Transmitter module, not handset
	CRSF_ADDRESS_ELRS_LUA          = 0xEF   // !!Non-Standard!! Source address used by ExpressLRS Lua
} CRSF_ADDRESS;


/**
 * @brief Crossfire protocol sync bytes
 */
typedef enum { CRSF_SYNC_DEFAULT = 0xC8, CRSF_SYNC_EDGE_TX = 0xEE } CRSF_SYNC;

// Handset Data
extern CRSF_ChannelsPacked CRSF_Channels;
extern CRSF_LinkStatistics CRSF_LinkState;

// RX statistics
extern uint32_t CRSF_LastChannelsPacked;
extern uint32_t CRSF_PPS;  // Packets per second

// ARM status
extern bool CRSF_ArmStatus;

// TX statistics
extern bool CRSF_TelemetryQueued;


/**
 * @brief Called every time, after receiving RC Channels Packed packet
 */
void CRSF_OnChannelsPacked();

/**
 * @brief Called every time, after receiving Link Statistics packet
 */
void CRSF_OnLinkStatistics();

/**
 * @brief  	   Initialise communication with the reciever
 * @param[in]  huart uart handle
 */
void CRSF_Init(UART_HandleTypeDef* huart);

/**
 * @brief     Parse recieved packet (call inside RxCpltCallback)
 * @param[in] huart uart handle
 */
void CRSF_HandleRX(const UART_HandleTypeDef* huart);

/**
 * @brief 	   Queue GPS data to be sent as telemetry
 * @param[in] *gps GPS data structure pointer
 */
void CRSF_QueueGPSData(CRSF_GPSData* gps);

/**
 * @brief	   Queue Battery data to be sent as telemetry
 * @param[in] *bat Battery data structure pointer
 */
void CRSF_QueueBatteryData(CRSF_BatteryData* bat);

/**
 * @brief  Queue ping sent to handset
 * 		   (Last time I checked it doesn't repond :( )
 */
void CRSF_QueuePing();

/**
 * @brief	   Restore communication after uart error (call inside ErrorCallback)
 * @param[in]  huart uart handle
 */
void CRSF_HandleErr(UART_HandleTypeDef* huart);

/**
 * @brief Print channels data (1-4) through serial connection
 * The definition is marker __weak so you can easily change printed channels
 */
void CRSF_DEBUG_PrintChannels();

#endif /* INC_CRSF_CONNECTION_H_ */
