//
// Created by pwoli on 17.04.2025.
//

#ifndef CRSF_COMMONTYPES_H
#define CRSF_COMMONTYPES_H

/**
 * @brief Crossfire protocol sync bytes
 */
typedef enum CRSF_SYNC { CRSF_SYNC_DEFAULT = 0xC8, CRSF_SYNC_EDGE_TX = 0xEE } CRSF_SYNC;

/**
 * @brief Crossfire protocol frame types
 */
typedef enum CRSF_FRAMETYPE {
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
typedef enum CRSF_ADDRESS {
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

#endif //CRSF_COMMONTYPES_H
