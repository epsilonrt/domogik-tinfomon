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
 * @brief Main task
 */
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sysio/string.h>
#include "dmg-tinfomon.h"
#include "config.h"

/* public variables ========================================================= */
xTiContext xCtx = {

  .sSerialPort = XPL_TINFOMON_SERIAL_PORT
};

/* private variables ======================================================== */
static bool bMainIsRun = true;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// Print usage info
static void
prvPrintUsage (void) {
  printf ("%s - xPL Teleinformation monitor\n", __progname);
  printf ("Copyright (c) 2016 epsilonRT\n\n");
  printf ("Usage: %s [-i interface] [-n iolayer] [-W timeout] [-p port]  [-D] [-d] [-h]\n", __progname);
  printf ("  -i interface - use interface named interface (i.e. eth0) as network interface\n");
  printf ("  -n iolayer   - use hardware abstraction layer to access the network\n"
          "                 (i.e. udp, xbeezb... default: udp)\n");
  printf ("  -W timeout   - set the timeout at the opening of the io layer\n");
  printf ("  -D           - do not daemonize -- run from the console\n");
  printf ("  -p           - serial port to which the electricity meter sends its data.\n"
          "                 default: %s\n", XPL_TINFOMON_SERIAL_PORT);
  printf ("  -d           - enable debugging, it can be doubled or tripled to\n"
          "                 increase the level of debug.\n");
  printf ("  -c           - enable power calculation if papp is not provided\n");
  printf ("  -h           - print this message\n\n");
}

// -----------------------------------------------------------------------------
static void
prvSignalHandler (int sig) {

  if ( (sig == SIGTERM) || (sig == SIGINT) ) {

    bMainIsRun = false;
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
void
vMain (gxPLSetting * setting) {
  int ret;
  gxPLDevice * device;
  gxPLApplication * app;

  PNOTICE ("starting xpl-tinfomon... (%s log)", sLogPriorityStr (setting->log) );

  // Create xPL application and device
  device = xDeviceCreate (setting);
  if (device == NULL) {

    PERROR ("Unable to start xPL");
    exit (EXIT_FAILURE);
  }

  // take the application to be able to close
  app = gxPLDeviceParent (device);


  // Ti init.
  ret = iTinfoMonOpen (device);
  if (ret != 0) {

    PERROR ("Unable to setting teleinfo link, error %d", ret);
    gxPLAppClose (app);
    exit (EXIT_FAILURE);
  }

  /*
   * !!!!!!!!!!!! TODO !!!!!!!!!!!!
   * Here you can add more blocks to initialize or open
   */

  // Enable the device to do the job
  gxPLDeviceEnable (device, true);

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  while (bMainIsRun) {

    // Main Loop
    ret = gxPLAppPoll (app, GXPL_POLL_RATE_MS);
    if (ret != 0) {

      PWARNING ("Unable to poll xPL network, error %d", ret);
    }

    ret = iTinfoMonPoll (device);
    if (ret < 0) {

      PWARNING ("Unable to poll teleinfo link, error %d", ret);
    }
  }

  ret = iTinfoMonClose (device);
  if (ret != 0) {

    PWARNING ("Unable to close teleinfo link, error %d", ret);
  }

  // Sends heartbeat end messages to all devices
  ret = gxPLAppClose (app);
  if (ret != 0) {

    PWARNING ("Unable to close xPL network, error %d", ret);
  }

  PNOTICE ("xpl-tinfomon closed, Have a nice day !");
  exit (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
void
vParseAdditionnalOptions (int argc, char *argv[]) {
  int c;

  static const char short_options[] = "hc::p:" GXPL_GETOPT;
  static struct option long_options[] = {
    {"port",     required_argument, NULL, 'p'},
    {"help",     no_argument,       NULL, 'h' },
    {"cons",     optional_argument, NULL, 'c' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'p':
        xCtx.sSerialPort = optarg;
        PDEBUG ("set serial port to %s", xCtx.sSerialPort);
        break;

      case 'c':
        xCtx.uPappSimInterval = XPL_TINFOMON_PSIM_DEFAULT_INTERVAL;
        if (optarg) {
          long l;
          
          if (iStrToLong (optarg, &l, 10) == 0) {
            if ((l > 0) && (l <= UINT_MAX)) {
              
              xCtx.uPappSimInterval = (unsigned) l;
              PDEBUG ("set cons to %u",xCtx.uPappSimInterval);
              break;
            }
          }
          PERROR ("Unable to set consumption interval to %s", optarg); 
          break;         
        }
        PDEBUG ("set cons to %u (default)",xCtx.uPappSimInterval);
        break;
        
      case 'h':
        prvPrintUsage();
        exit (EXIT_SUCCESS);
        break;

      default:
        break;
    }
  }
  while (c != -1);
}

/* ========================================================================== */
