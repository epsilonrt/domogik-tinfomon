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
 * @brief Configurable device side
 */
#include <string.h>
#include <sysio/string.h>
#include "dmg-tinfomon.h"
#include "config.h"

/* private functions ======================================================== */
// --------------------------------------------------------------------------
//  It's best to put the logic for reading the device configuration
//  and parsing it into your code in a seperate function so it can
//  be used by your prvDeviceConfigChanged and your startup code that
//  will want to parse the same data after a setting file is loaded
void
prvDeviceSetConfig (gxPLDevice * device) {
  const char * str;

  str = gxPLDeviceConfigValueGet (device, XPL_TINFOMON_INTERVAL_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.ulInterval = (time_t) n;
      xCtx.xLastFrmTime.tv_sec = 0;
      xCtx.xLastFrmTime.tv_usec = 0;
    }
  }
  
  str = gxPLDeviceConfigValueGet (device, XPL_TINFOMON_ADPS_DELAY_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.ulAdpsDelay = (time_t) n;
    }
  }

  str = gxPLDeviceConfigValueGet (device, XPL_TINFOMON_BAUDRATE_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.ulBaudrate = n;
    }
  }

  str = gxPLDeviceConfigValueGet (device, XPL_TINFOMON_INDEX_GAP_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.ulIndexGap = n;
      xCtx.ulLastFrmIndex = 0;
    }
  }

}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration
static void
prvDeviceConfigChanged (gxPLDevice * device, void * udata) {

  gxPLMessageSourceInstanceIdSet (xCtx.xMsg, gxPLDeviceInstanceId (device) );

  // Read setting items for device and install
  prvDeviceSetConfig (device);
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
// Create xPL application and device
gxPLDevice *
xDeviceCreate (gxPLSetting * setting) {
  gxPLApplication * app;
  gxPLDevice * device;

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    return NULL;
  }

  // Initialize sensor device
  // Create a device for us
  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, XPL_TINFOMON_VENDOR_ID,
                                         XPL_TINFOMON_DEVICE_ID,
                                         gxPLConfigPath (XPL_TINFOMON_CONFIG_FILENAME) );
  if (device == NULL) {

    return NULL;
  }

  gxPLDeviceVersionSet (device, XPL_TINFOMON_DEVICE_VERSION);

  // If the configuration was not reloaded, then this is our first time and
  // we need to define what the configurables are and what the default values
  // should be.
  if (gxPLDeviceIsConfigured (device) == false) {

    // Define configurable items and give it a default
    gxPLDeviceConfigItemAdd (device, XPL_TINFOMON_INTERVAL_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, XPL_TINFOMON_INTERVAL_NAME,
                              gxPLLongToStr (XPL_TINFOMON_INTERVAL) );

    gxPLDeviceConfigItemAdd (device, XPL_TINFOMON_BAUDRATE_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, XPL_TINFOMON_BAUDRATE_NAME,
                              gxPLLongToStr (XPL_TINFOMON_BAUDRATE) );

    gxPLDeviceConfigItemAdd (device, XPL_TINFOMON_INDEX_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, XPL_TINFOMON_INDEX_GAP_NAME,
                              gxPLLongToStr (XPL_TINFOMON_INDEX_GAP) );

    gxPLDeviceConfigItemAdd (device, XPL_TINFOMON_ADPS_DELAY_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, XPL_TINFOMON_ADPS_DELAY_NAME,
                              gxPLLongToStr (XPL_TINFOMON_ADPS_DELAY) );

  }

  // Create a sensor.basic message conforming to http://xplproject.org.uk/wiki/Schema_-_SENSOR.html
  xCtx.xMsg = gxPLDeviceMessageNew (device, gxPLMessageTrigger);
  assert (xCtx.xMsg);

  // Setting up the message
  gxPLMessageBroadcastSet (xCtx.xMsg, true);

  // Parse the device configurables into a form this program
  // can use (whether we read a setting or not)
  prvDeviceSetConfig (device);

  // Add a device change listener we'll use to pick up a new gap
  gxPLDeviceConfigListenerAdd (device, prvDeviceConfigChanged, NULL);

  return device;
}

/* ========================================================================== */
