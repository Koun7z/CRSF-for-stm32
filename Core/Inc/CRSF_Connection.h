/*
 * CRSF_Connection.h
 *
 *  Created on: Dec 7, 2024
 *      Author: pwoli
 */

#ifndef INC_CRSF_CONNECTION_H_
#define INC_CRSF_CONNECTION_H_

#include "CRSF_HAL_Include.h"
#include "CRSF_HandsetData.h"
#include "CRSF_TelemetryData.h"
#include "CRSF_CommonTypes.h"

#include <stdbool.h>

#if SERIAL_DEBUG
#  define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#  define DEBUG_LOG(...)
#endif

// Handset Data
extern CRSF_ChannelsPacked CRSF_Channels;
extern CRSF_LinkStatistics CRSF_LinkState;

// RX statistics
extern uint32_t CRSF_LastChannelsPacked;
extern uint32_t CRSF_PPS;  // Packets per second

// Connection status
extern bool CRSF_ArmStatus;
extern bool CRSF_FailsafeStatus;

// TX statistics
extern bool CRSF_TelemetryQueued;

/*
** Callbacks
*/

/**
 * @brief Called every time, after receiving RC Channels Packed packet
 */
void CRSF_OnChannelsPacked();

/**
 * @brief Called every time, after receiving Link Statistics packet
 */
void CRSF_OnLinkStatistics();

/**
 * Called after Low LQ failsafe occurred.
 */
void CRSF_OnFailsafe();

/*
** Functions
*/

/**
 * @brief  	   Initialise communication with the reciever
 * @param[in]  huart uart handle
 */
void CRSF_Init(UART_HandleTypeDef* huart);

/**
 * @brief      Parse received packet.
 *             Should be called after every HAL_UARTEx_RxEventCallback.
 */
void CRSF_HandleRX(void);

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
 * 		   (Last time I checked it doesn't respond :( )
 */
void CRSF_QueuePing(void);

/**
 * @brief  Communication after uart error
 *         Should be called after every HAL_UART_ErrorCallback
 */
void CRSF_HandleErr();

/**
 * @brief  Print channels data (1-4) through serial connection
 *         The definition is marked __weak so you can easily change printed channels
 */
void CRSF_DEBUG_PrintChannels(void);

#endif /* INC_CRSF_CONNECTION_H_ */
