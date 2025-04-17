//
// Created by pwoli on 17.04.2025.
//

#ifndef CRSF_CONFIG_H
#define CRSF_CONFIG_H

/*
** Telemetry config
*/

#define TELEMETRY_ENABLED 0
#define SERIAL_DEBUG      0

/*
** Arms status handling config
*/

#define CRSF_HANDLE_ARM  1
#define CRSF_ARM_CHANNEL CRSF_Channels.Ch5
#define CRSF_ARM_DELAY   5  // Packet num

/*
** Failsafe config
*/

#define CRSF_FAILSAFE_ENABLE 1
#define CRSF_LQ_FAILSAFE_THRESHOLD  80      // [%]
#define CRSF_RSSI_FAILSAFE_THRESHOLD 105    // [dBm * -1]

#endif //CRSF_CONFIG_H
