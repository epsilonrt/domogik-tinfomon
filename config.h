/**
 * Copyright © 2016-2017 epsilonRT, All rights reserved.
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * <http://www.cecill.info>. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 *
 * @file
 * @brief
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

/* constants ================================================================ */
#define XPL_TINFOMON_VENDOR_ID       "domogik"
#define XPL_TINFOMON_DEVICE_ID       "tinfomon"
#define XPL_TINFOMON_CONFIG_FILENAME "domogik-tinfomon.xpl"
#define XPL_TINFOMON_INSTANCE_ID     NULL // NULL for auto instance
#define XPL_TINFOMON_DEVICE_VERSION  VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe
#define XPL_TINFOMON_LOG_LEVEL       LOG_INFO
#define DAEMON_MAX_RESTARTS      100
#define GXPL_POLL_RATE_MS        1000

#define XPL_TINFOMON_PSIM_DEFAULT_INTERVAL 60

// Configurables
#define XPL_TINFOMON_INTERVAL        30 
#define XPL_TINFOMON_INTERVAL_NAME   "stat-interval"
#define XPL_TINFOMON_BAUDRATE        1200
#define XPL_TINFOMON_BAUDRATE_NAME   "baudrate"
#define XPL_TINFOMON_ADPS_DELAY      300
#define XPL_TINFOMON_ADPS_DELAY_NAME "adps-delay"
#define XPL_TINFOMON_INDEX_GAP       1
#define XPL_TINFOMON_INDEX_GAP_NAME  "index-gap"

#define XPL_TINFOMON_DEFAULT_INTERVAL 60

#if defined(BOARD_RASPBERRYPI)
#define XPL_TINFOMON_SERIAL_PORT     "/dev/ttyAMA0"
#elif defined(BOARD_NANOPI)
#define XPL_TINFOMON_SERIAL_PORT     "/dev/ttyS1"
#else
#define XPL_TINFOMON_SERIAL_PORT     "/dev/ttyUSB0"
#endif

/* default values =========================================================== */
#define DEFAULT_IO_LAYER                  "udp"
#define DEFAULT_CONNECT_TYPE              gxPLConnectViaHub
#define DEFAULT_HEARTBEAT_INTERVAL        300
#define DEFAULT_CONFIG_HEARTBEAT_INTERVAL 60
#define DEFAULT_HUB_DISCOVERY_INTERVAL    3
#define DEFAULT_ALLOC_STR_GROW            256
#define DEFAULT_LINE_BUFSIZE              256
#define DEFAULT_MAX_DEVICE_GROUP          4
#define DEFAULT_MAX_DEVICE_FILTER         4
#define DEFAULT_XBEE_PORT                 "/dev/ttySC0"

// Unix only
#define DEFAULT_CONFIG_HOME_DIRECTORY     ".gxpl"
#define DEFAULT_CONFIG_SYS_DIRECTORY      "/etc/gxpl"

/* build options ============================================================ */
#define CONFIG_DEVICE_CONFIGURABLE    1
#define CONFIG_DEVICE_GROUP           1
#define CONFIG_DEVICE_FILTER          1
// add the "remote-addr" field in hbeat.basic
#define CONFIG_HBEAT_BASIC_EXTENSION  1

/* conditionals options ====================================================== */

/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
