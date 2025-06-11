//
// Created by pwoli on 17.04.2025.
//

#ifndef CRSF_CONFIG_H
#define CRSF_CONFIG_H


/*
** Serial debug
*/

#define SERIAL_DEBUG 0

#if SERIAL_DEBUG
#  include <stdio.h>
#  define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#  define DEBUG_LOG(...)
#endif

/*
** Telemetry config
*/

#define TELEMETRY_ENABLED 0

/*
** Arms status handling config
*/

#define CRSF_HANDLE_ARM  1
#define CRSF_ARM_CHANNEL CRSF_Channels[4]
#define CRSF_ARM_DELAY   5  // Packet num

/*
** Failsafe config
*/

#define CRSF_FAILSAFE_ENABLE 1
#define CRSF_LQ_FAILSAFE_THRESHOLD  60   // [%]
#define CRSF_RSSI_FAILSAFE_THRESHOLD 90  // [dBm * -1]

#endif //CRSF_CONFIG_H
