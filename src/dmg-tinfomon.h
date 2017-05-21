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
#ifndef _DMG_TINFOMON_HEADER_
#define _DMG_TINFOMON_HEADER_
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <gxPL.h>
#include <gxPL/stdio.h>
#include <sysio/tinfo.h>
#include "version-git.h"

/* constants ================================================================ */

/* structures =============================================================== */
struct xTiContext {
  time_t ulInterval;  // configurable
  unsigned long ulBaudrate; // configurable
  time_t ulAdpsDelay; // configurable
  unsigned long ulIndexGap; // configurable

  xTinfo * xTi;
  xTinfoFrame xFrame;
  unsigned long ulLastFrmIndex;
  struct timeval xLastFrmTime;
  struct timeval xFrmTime;
  time_t ulAdpsTime;
  const char * sSerialPort;
  // Variables pour le calcul de la consommation
  unsigned long ulPappSimPrevIndex;
  struct timeval xPappSimPrevTime;
  unsigned uPappSimInterval;

  gxPLMessage * xMsg;
  struct {
    int bTinfoUpdated: 1;
  };
};

/* types ==================================================================== */
typedef struct xTiContext xTiContext;

/* public variables ========================================================= */
/*
 * Configurable parameters
 */
extern xTiContext xCtx;

/* internal public functions ================================================ */
/*
 * Main Task
 */
void vMain (gxPLSetting * setting);
void vParseAdditionnalOptions (int argc, char ** argv);

/*
 * xPL Device
 */
gxPLDevice * xDeviceCreate (gxPLSetting * setting);

/*
 * Ti Interface
 */
int iTinfoMonOpen (gxPLDevice * device);
int iTinfoMonClose (gxPLDevice * device);
int iTinfoMonPoll (gxPLDevice * device);

/* ========================================================================== */
#endif /* _DMG_TINFOMON_HEADER_ defined */
