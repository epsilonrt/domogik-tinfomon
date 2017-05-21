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
#include <math.h>
#include <string.h>

#include "dmg-tinfomon.h"
#include "config.h"

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static int
prviSendAdpsMessage (gxPLDevice * device, union xTinfoFrame * f) {
  gxPLMessage * m = xCtx.xMsg;

  xCtx.ulAdpsTime = time (NULL);

  /*  --- Message envoyé ---
        XPL-TRIG
        teleinfo.basic
        {
        adco=<adco>
        type=<adps>
        current=<on/off>
        iinst=<iinst> # monophasé
        iinst1=<iinst1> # triphasé
        iinst2=<iinst2> # triphasé
        iinst3=<iinst3> # triphasé
        }
   */
  gxPLMessageTypeSet (m, gxPLMessageTrigger);
  gxPLMessageSchemaSet (m, "teleinfo", "basic");
  gxPLMessageBodyClear (m);

  gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
  gxPLMessagePairAdd (m, "type", "adps");
  gxPLMessagePairAdd (m, "current", "on");

  if (f->blue.flag & eTinfoFlagShort) {

    gxPLMessagePairAddFormat (m, "iinst1", "%d", f->blue_short.iinst[0]);
    gxPLMessagePairAddFormat (m, "iinst2", "%d", f->blue_short.iinst[1]);
    gxPLMessagePairAddFormat (m, "iinst3", "%d", f->blue_short.iinst[2]);
  }
  else {

    gxPLMessagePairAddFormat (m, "iinst", "%d", f->blue.iinst[0]);
  }

  return gxPLDeviceMessageSend (device, m);
}

// -----------------------------------------------------------------------------
// Gestionnaire de réception de trames normales
static int
prviFrameCB (struct xTinfo * t, union xTinfoFrame * f) {

  if (f->raw.frame == eTinfoFrameBlue)  {

    if (gettimeofday (&xCtx.xFrmTime, NULL) < 0) {

      return -1;
    }
    memcpy (&xCtx.xFrame, f, sizeof (union xTinfoFrame));
    xCtx.bTinfoUpdated = 1;
  }

  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de changement de période tarifaire (HC...)
static int
prviNewPtecCB (struct xTinfo * t, union xTinfoFrame * f) {
  gxPLDevice * device = pvTinfoGetUserContext (t);
  gxPLMessage * m = xCtx.xMsg;

  xCtx.ulPappSimPrevIndex = 0;
  xCtx.ulLastFrmIndex = 0;

  /*  --- Message envoyé ---
        XPL-TRIG
        teleinfo.basic
        {
        adco=<adco>
        type=<ptec>
        current=<PTEC>
        }
   */
  gxPLMessageTypeSet (m, gxPLMessageTrigger);
  gxPLMessageSchemaSet (m, "teleinfo", "basic");
  gxPLMessageBodyClear (m);
  gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
  gxPLMessagePairAdd (m, "type", "ptec");
  gxPLMessagePairAdd (m, "current", sTinfoPtecToStr (f->blue.ptec));

  return gxPLDeviceMessageSend (device, m);
}


// -----------------------------------------------------------------------------
// Gestionnaire de changement de couleur tempo demain
static int
prviNewColorCB (struct xTinfo * t, union xTinfoFrame * f) {
  gxPLDevice * device = pvTinfoGetUserContext (t);
  gxPLMessage * m = xCtx.xMsg;

  /*  --- Message envoyé ---
        XPL-TRIG
        teleinfo.basic
        {
        adco=<adco>
        type=<demain>
        current=<DEMAIN>
        }
   */
  gxPLMessageTypeSet (m, gxPLMessageTrigger);
  gxPLMessageSchemaSet (m, "teleinfo", "basic");
  gxPLMessageBodyClear (m);
  gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
  gxPLMessagePairAdd (m, "type", "demain");
  gxPLMessagePairAdd (m, "current",  sTinfoTempoColorToStr (f->blue.tarif.tempo.demain));

  return gxPLDeviceMessageSend (device, m);
}

// -----------------------------------------------------------------------------
// Gestionnaire de changement de mot d'état
static int
prviNewStateWordCB (struct xTinfo * t, union xTinfoFrame * f) {
  gxPLDevice * device = pvTinfoGetUserContext (t);
  gxPLMessage * m = xCtx.xMsg;

  /*  --- Message envoyé ---
        XPL-TRIG
        teleinfo.basic
        {
        adco=<adco>
        type=<motdetat>
        current=<MOTDETAT>
        }
   */
  gxPLMessageTypeSet (m, gxPLMessageTrigger);
  gxPLMessageSchemaSet (m, "teleinfo", "basic");
  gxPLMessageBodyClear (m);
  gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
  gxPLMessagePairAdd (m, "type", "motdetat");
  gxPLMessagePairAddFormat (m, "current", "%06X", f->blue.motdetat);

  return gxPLDeviceMessageSend (device, m);
}

// -----------------------------------------------------------------------------
// Gestionnaire d'avertissement de Dépassement de Puissance Souscrite
static int
prviAdpsCB (struct xTinfo * t, union xTinfoFrame * f) {
  gxPLDevice * device = pvTinfoGetUserContext (t);

  return prviSendAdpsMessage (device, f);
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
int
iTinfoMonOpen (gxPLDevice * device) {

  // Ouverture de la liaison
  xCtx.xTi = xTinfoOpen (xCtx.sSerialPort, xCtx.ulBaudrate);

  if (xCtx.xTi == NULL) {

    return -1;
  }

  vLog (LOG_DEBUG, "teleinfo link opened on %s, %lu baud", xCtx.sSerialPort,
        xCtx.ulBaudrate);

  // Installation des gestionnaires d'événements
  vTinfoSetCB (xCtx.xTi, eTinfoCbFrame, prviFrameCB);
  vTinfoSetCB (xCtx.xTi, eTinfoCbPtec, prviNewPtecCB);
  vTinfoSetCB (xCtx.xTi, eTinfoCbAdps, prviAdpsCB);
  vTinfoSetCB (xCtx.xTi, eTinfoCbTempo, prviNewColorCB);
  vTinfoSetCB (xCtx.xTi, eTinfoCbMotEtat, prviNewStateWordCB);

  // Permettra l'envoi de messages xPL dans les callbacks tinfo
  vTinfoSetUserContext (xCtx.xTi, device);

  return 0;
}

// -----------------------------------------------------------------------------
int
iTinfoMonClose (gxPLDevice * device) {
  int ret;

  ret = iTinfoClose (xCtx.xTi);

  gxPLMessageDelete (xCtx.xMsg);
  return ret;
}

// -----------------------------------------------------------------------------
int
iTinfoMonPoll (gxPLDevice * device) {
  int ret = 0;

  ret = iTinfoPoll (xCtx.xTi);

  if ( (ret == 0) && (xCtx.bTinfoUpdated) && gxPLDeviceIsHubConfirmed (device)) {
    xTinfoFrame * f = &xCtx.xFrame;
    gxPLMessage * m = xCtx.xMsg;

    xCtx.bTinfoUpdated = 0;

    if (f->blue.flag & eTinfoFlagShort) {

      return prviSendAdpsMessage (device, f);
    }
    else {
      // Trame longue
      unsigned long ulIndex = 0;
      unsigned long ulTimeElapsed;
      struct timeval t;

      if (xCtx.ulAdpsTime) {
        /* Un message de dépassement de puissance souscrite a été envoyé
         * Il faut envoyé un message de fin de dépassement après un certain
         * délai
         */
        time_t ta = time (NULL);

        if ( (ta - xCtx.ulAdpsTime) >= xCtx.ulAdpsDelay) {

          gxPLMessageTypeSet (m, gxPLMessageTrigger);
          gxPLMessageSchemaSet (m, "teleinfo", "basic");
          gxPLMessageBodyClear (m);
          gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
          gxPLMessagePairAdd (m, "type", "adps");
          gxPLMessagePairAdd (m, "current", "off");


          if (f->blue.nph == 3) {

            gxPLMessagePairAddFormat (m, "iinst1", "%d", f->blue.iinst[0]);
            gxPLMessagePairAddFormat (m, "iinst2", "%d", f->blue.iinst[1]);
            gxPLMessagePairAddFormat (m, "iinst3", "%d", f->blue.iinst[2]);
          }
          else {

            gxPLMessagePairAddFormat (m, "iinst", "%d", f->blue.iinst[0]);
          }

          if (gxPLDeviceMessageSend (device, m) < 0) {

            return -1;
          }

          xCtx.ulAdpsTime = 0;
        }
      }

      // Construction du message pour la trame longue
      timersub (&xCtx.xFrmTime, &xCtx.xLastFrmTime, &t);
      ulTimeElapsed = t.tv_sec *  1000000 + t.tv_usec;

      if ( (ulTimeElapsed >= (xCtx.ulInterval * 1000000)) || (xCtx.ulIndexGap)) {

        if (xCtx.ulIndexGap) {

          gxPLMessageTypeSet (m, gxPLMessageTrigger);
        }
        else {

          gxPLMessageTypeSet (m, gxPLMessageStatus);
        }
        gxPLMessageBodyClear (m);
        gxPLMessageSchemaSet (m, "teleinfo", "basic");

        gxPLMessagePairAddFormat (m, "adco", "%012lld", f->blue.adco);
        gxPLMessagePairAdd (m, "optarif", sTinfoOpTarifToStr (f->blue.optarif));
        gxPLMessagePairAddFormat (m, "isousc", "%d", f->blue.isousc);

        switch (f->blue.optarif) {

          case eTinfoOpTarifBase:
            gxPLMessagePairAddFormat (m, "base", "%ld", f->blue.tarif.base.index);
            ulIndex = f->blue.tarif.base.index;
            break;

          case eTinfoOpTarifHc:
            gxPLMessagePairAddFormat (m, "hchc", "%ld", f->blue.tarif.hc.index_hc);
            gxPLMessagePairAddFormat (m, "hchp", "%ld", f->blue.tarif.hc.index_hp);
            switch (f->blue.ptec) {
              case eTinfoPtecHc:
                ulIndex = f->blue.tarif.hc.index_hc;
                break;
              case eTinfoPtecHp:
                ulIndex = f->blue.tarif.hc.index_hp;
                break;
              default:
                break;
            }
            break;

          case eTinfoOpTarifEjp:
            gxPLMessagePairAddFormat (m, "ejphn", "%ld", f->blue.tarif.ejp.index_hn);
            gxPLMessagePairAddFormat (m, "ejphpm", "%ld", f->blue.tarif.ejp.index_hpm);
            gxPLMessagePairAddFormat (m, "pejp", "%d", f->blue.tarif.ejp.pejp);
            switch (f->blue.ptec) {
              case eTinfoPtecHn:
                ulIndex = f->blue.tarif.ejp.index_hn;
                break;
              case eTinfoPtecPm:
                ulIndex = f->blue.tarif.ejp.index_hpm;
                break;
              default:
                break;
            }
            break;

          case eTinfoOpTarifTempo:
            gxPLMessagePairAddFormat (m, "bbrhcjb", "%ld", f->blue.tarif.tempo.index_hcjb);
            gxPLMessagePairAddFormat (m, "bbrhpjb", "%ld", f->blue.tarif.tempo.index_hpjb);
            gxPLMessagePairAddFormat (m, "bbrhcjw", "%ld", f->blue.tarif.tempo.index_hcjw);
            gxPLMessagePairAddFormat (m, "bbrhpjw", "%ld", f->blue.tarif.tempo.index_hpjw);
            gxPLMessagePairAddFormat (m, "bbrhcjr", "%ld", f->blue.tarif.tempo.index_hcjr);
            gxPLMessagePairAddFormat (m, "bbrhpjr", "%ld", f->blue.tarif.tempo.index_hpjr);
            switch (f->blue.ptec) {
              case eTinfoPtecHcJb:
                ulIndex = f->blue.tarif.tempo.index_hcjb;
                break;
              case eTinfoPtecHcJw:
                ulIndex = f->blue.tarif.tempo.index_hcjw;
                break;
              case eTinfoPtecHcJr:
                ulIndex = f->blue.tarif.tempo.index_hcjr;
                break;
              case eTinfoPtecHpJb:
                ulIndex = f->blue.tarif.tempo.index_hpjb;
                break;
              case eTinfoPtecHpJw:
                ulIndex = f->blue.tarif.tempo.index_hpjw;
                break;
              case eTinfoPtecHpJr:
                ulIndex = f->blue.tarif.tempo.index_hpjr;
                break;
              default:
                break;
            }
            break;

          default:
            break;
        }

        if ( (xCtx.ulIndexGap) &&
             ( (ulIndex - xCtx.ulLastFrmIndex) < xCtx.ulIndexGap)) {
          // Utilisation d'un seuil d'index, seuil non franchi, on sort
          return 0;
        }

        gxPLMessagePairAdd (m, "ptec", sTinfoPtecToStr (f->blue.ptec));

        if (f->blue.optarif == eTinfoOpTarifTempo) {

          gxPLMessagePairAdd (m, "demain", sTinfoTempoColorToStr (f->blue.tarif.tempo.demain));
        }

        if (f->blue.nph == 3) {

          gxPLMessagePairAddFormat (m, "iinst1", "%d", f->blue.iinst[0]);
          gxPLMessagePairAddFormat (m, "iinst2", "%d", f->blue.iinst[1]);
          gxPLMessagePairAddFormat (m, "iinst3", "%d", f->blue.iinst[2]);
          gxPLMessagePairAddFormat (m, "imax1", "%d", f->blue.imax[0]);
          gxPLMessagePairAddFormat (m, "imax2", "%d", f->blue.imax[1]);
          gxPLMessagePairAddFormat (m, "imax3", "%d", f->blue.imax[2]);
          gxPLMessagePairAddFormat (m, "pmax", "%d", f->blue.pmax);
        }
        else {

          gxPLMessagePairAddFormat (m, "iinst", "%d", f->blue.iinst[0]);
          if (f->blue.flag & eTinfoFlagAdps) {

            gxPLMessagePairAddFormat (m, "adps", "%d", f->blue.adps);
          }
          gxPLMessagePairAddFormat (m, "imax", "%d", f->blue.imax[0]);
        }

        // La puissance apparente n'est pas présente pour tous les compteurs
        if (f->blue.flag & eTinfoFlagPapp) {

          gxPLMessagePairAddFormat (m, "papp", "%d", f->blue.papp);
        }
        else {
          // Puissance apparente non présente dans la trame

          if ( (xCtx.uPappSimInterval) && (ulIndex)) {

            // Calcul à partir de la différence d'index
            if (xCtx.ulPappSimPrevIndex) {

              if (xCtx.xPappSimPrevTime.tv_sec)  {

                timersub (&xCtx.xFrmTime, &xCtx.xPappSimPrevTime, &t);
                ulTimeElapsed = t.tv_sec * 1000000 + t.tv_usec;

                // Calcul
                if (ulTimeElapsed >= (xCtx.uPappSimInterval * 1000000)) {
                  int p;
                  unsigned long ulCons = ulIndex - xCtx.ulPappSimPrevIndex;

                  p = (int) ( (double) ulCons * 3600e6 / ulTimeElapsed);
                  //PERROR("dsec=%lu - dusec=%lu = ulTimeElapsed=%.2f - dIndex=%lu => "
                  //"padd=%d",t.tv_sec, t.tv_usec, ulTimeElapsed / 1e6, ulCons, p);
                  xCtx.ulPappSimPrevIndex = ulIndex;
                  xCtx.xPappSimPrevTime = xCtx.xFrmTime;

                  gxPLMessagePairAddFormat (m, "papp", "%d", p);
                }
              }
            }
            else {

              xCtx.ulPappSimPrevIndex = ulIndex;
              xCtx.xPappSimPrevTime = xCtx.xFrmTime;
              //PERROR("sec=%lu - usec=%lu - Index=%lu", xCtx.xPappSimPrevTime.tv_sec,
              //xCtx.xPappSimPrevTime.tv_usec, xCtx.ulPappSimPrevIndex);
            }
          }
        }

        if (f->blue.optarif == eTinfoOpTarifHc) {

          gxPLMessagePairAddFormat (m, "hhphc", "%c",  f->blue.tarif.hc.horaire);
        }


        if (f->blue.motdetat) {

          gxPLMessagePairAddFormat (m, "motdetat", "%06X", f->blue.motdetat);
        }

        if (f->blue.nph == 3) {

          gxPLMessagePairAddFormat (m, "ppot", "%02X", f->blue.ppot);
        }
        
        // Broadcast the message
        PDEBUG ("Sending teleinfo message received at %s", ctime (&f->blue.time));
        if (gxPLDeviceMessageSend (device, m) < 0) {

          return -1;
        }

        xCtx.ulLastFrmIndex = ulIndex;
        xCtx.xLastFrmTime = xCtx.xFrmTime;
      }
    }
  }

  return ret;
}

/* ========================================================================== */
