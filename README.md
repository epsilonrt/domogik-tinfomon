# dmg-tinfomon

*Moniteur xPL de téléinformation ERDF*

---
Copyright 2016-2017 (c), epsilonRT

<a href="http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.html">
  <img src="https://raw.githubusercontent.com/epsilonrt/gxPL/master/doc/images/osi.png" alt="osi.png" align="right" valign="top">
</a>

dmg-tinfomon est un daemon qui surveille une liaison de téléinformation par
l'intermédiaire d'une liaison série et transmets ses informations sur le 
réseau xPL.

Il a été conçu pour être installé sur un [Raspberry Pi](https://www.raspberrypi.org/) 
mais peut très être installé sur n'importe quelle cible suportée par SysIo et gxPL.

L'implémentation est conforme au document
[ERDF-NOI-CPT_02E](http://www.erdf.fr/sites/default/files/ERDF-NOI-CPT_02E.pdf)
et s'appuie sur le module 
[téléinformation de SysIo](http://www.epsilonrt.fr/sysio/group__sysio__tinfo.html).

Il est compatible avec les trames émises par: 

 * le compteur « Bleu » électronique monophasé multitarif (CBEMM : 2 paliers différents),
 * le compteur « Bleu » électronique triphasé multitarif (CBETM).

## Installation

### Pré-requis

Il faut installer au prélable [SysIo](https://github.com/epsilonrt/sysio) et 
[gxPL](https://github.com/epsilonrt/gxPL).

### Compilation et Installation
 
Il suffit alors d'exécuter les commandes suivantes :

        git clone https://github.com/epsilonrt/domogik-tinfomon.git
        cd domogik-tinfomon
        make
        sudo make install

### Installation script de démarrage

Pour automatiser le démarrage et l'arrêt, il est possible d'installer le script de démarrage:

        cd init
        sudo make install

## Commande

    dmg-tinfomon - xPL Teleinformation monitor
    Copyright (c) 2016 epsilonRT

    Usage: dmg-tinfomon [-i interface] [-n iolayer] [-W timeout]  [-D] [-d] [-h]
      -i interface - use interface named interface (i.e. eth0) as network interface
      -n iolayer   - use hardware abstraction layer to access the network
                     (i.e. udp, xbeezb... default: udp)
      -W timeout   - set the timeout at the opening of the io layer
      -D           - do not daemonize -- run from the console
      -p           - serial port to which the electricity meter sends its data.
                     default: /dev/ttyAMA0
      -d           - enable debugging, it can be doubled or tripled to
                     increase the level of debug.
      -h           - print this message

Pour lancer le daemon en mode débugage:

    dmg-tinfomon -D -ddd -p /dev/ttyUSB0

Un script de lancement dmg-tinfomon peut être installé dans /etc/init.d et peut être
lancé à l'aide de la commande:

    sudo /etc/init.d/dmg-tinfomon start

Il peut être lancé automatiquement au démarrage du système:

    sudo insserv dmg-tinfomon


## Paramètres configurables

Les paramètres ci-dessous sont configurables par l'intermédiaire du protocole
de configuration prévu par xPL :

* **stat-interval** délai en secondes entre 2 envois de trame XPL-STAT
* **adps-delay** délai  en secondes entre la disparition du dépassement de 
  puissance souscrite et l'envoi du message xPL signalant cette disparition.
* **baudrate** vitesse de la liaison série reliée au compteur électrique en 
  bauds. Elle dépend du modèle de compteur, généralement 1200 Bd, 9600 Bd 
  pour Linky.
* interval interval en minutes entre 2 battements de coeur
* newconf nom de la configuration
* group groupes dont dmg-tinfomon fait partie, voir 
  [Devices Groups](http://xplproject.org.uk/wiki/XPL_Specification_Document.html#Device_Groups)
* filter filtre de messages utilisés, voir 
  [Installer and User Defined Filters](http://xplproject.org.uk/wiki/XPL_Specification_Document.html#Installer_and_User_Defined_Filters)

## Messages xPL

### XPL-STAT Structure

Ces trames sont émises à un intervale correspondant au paramètre configurable
*stat-interval* . Elles sont compatibles avec le plugin teleinfo de Domogik
en version 1.1.

    teleinfo.basic
    {
    adco=<value>
    optarif=<value>
    isousc=<value>
    [base=<value>]    # Option tarifaire de base
    [hchc=<value>]    # Option tarifaire heures creuses
    [hchp=<value>]    # Option tarifaire heures creuses
    [ejphn=<value>]   # Option tarifaire EJP (Effacecement des Jours de Pointe)
    [ejphpm=<value>]  # Option tarifaire EJP (Effacecement des Jours de Pointe)
    [pejp=<value>]    # Option tarifaire EJP (Effacecement des Jours de Pointe)
    [bbrhcjb=<value>] # Option tarifaire Tempo
    [bbrhpjb=<value>] # Option tarifaire Tempo
    [bbrhcjw=<value>] # Option tarifaire Tempo
    [bbrhpjw=<value>] # Option tarifaire Tempo
    [bbrhcjr=<value>] # Option tarifaire Tempo
    [bbrhpjr=<value>] # Option tarifaire Tempo
    ptec=<value>
    [demain=<value>]  # Option tarifaire Tempo
    [iinst1=<value>]  # triphasé
    [iinst2=<value>]  # triphasé
    [iinst3=<value>]  # triphasé
    [imax1=<value>]   # triphasé
    [imax2=<value>]   # triphasé
    [imax3=<value>]   # triphasé
    [pmax=<value>]    # triphasé
    [iinst=<value>]   # monophasé
    [imax=<value>]    # monophasé
    [papp=<value>]
    [hhphc=<value>]   # Option tarifaire heures creuses
    [motdetat=<value>]
    [ppot=<value>]    # triphasé
    }

### XPL-TRIG Structure

Ces messages sont émis lors d'un événement particulier, ils peuvent traités
par un moteur de scénario pour déclencher des réactions (arrêt du chauffage, 
démarrage du chauffe-eau électrique ...).

*adco* désigne l'identifiant du compteur électrique à l'origine de l'information.

    teleinfo.basic
    {
    adco=<adco>
    type=<ptec>
    current=<value>
    }

Trame émise lors d'un changement de période tarifaire en cours. Le paramètre
current fait partie de la liste  { "th", "hc", "hp", "hn", "hpm", "hcjb", 
"hcjw", "hcjr", "hpjb", "hpjw", "hpjr" }.

    teleinfo.basic
    {
    adco=<adco>
    type=<demain>
    current=<value>
    }

Trame émise lors d'un changement de couleur tempo du lendemain. Le paramètre
current fait partie de la liste  { "bleu", "blanc", "rouge" }.

    teleinfo.basic
    {
    adco=<adco>
    type=<motdetat>
    current=<value>
    }

Trame émise lors d'un changement de mot d'état du compteur. Le paramètre
current est codé avec 6 caractères hexadécimaux constituant 3 octets. 
L'interprétation de ses 3 octets est spécifique au modèle de compteur utilisé.

    teleinfo.basic
    {
    adco=<adco>
    type=<adps>
    current=<on/off>
    iinst=<value>  # monophasé
    iinst1=<value> # triphasé
    iinst2=<value> # triphasé
    iinst3=<value> # triphasé
    }

Trame émise lors d'un dépassement de puissance souscrite. Le paramètre
current fait partie de la liste  { "on", "off" }. Le paramètre iinst, iinst1, 
iinst2, iinst3 correspondent aux courants instantanés en ampères (A). iinst est
émis si le compteur est monophasé; iinst1, iinst2, iinst3 sont émis uniquement
dans le cas d'un compteur triphasé.

### XPL-CMND Structure

Aucun message traité.
