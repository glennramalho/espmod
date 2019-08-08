/*
 WiFi.h - esp32 Wifi support.
 Based on WiFi.h from Arduino WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014
 Modified on 29 July 2019 by Glenn Ramalho - RFIDo Design

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef WiFi_h
#define WiFi_h

#include <stdint.h>

#include "Arduino.h"
#include "TestSerial.h"
#include <systemc.h>

#include "Print.h"
#include "esp32-hal-log.h"
#include "IPAddress.h"
/*#include "IPv6Address.h"*/
#include "WiFiType.h"
#include "WiFiSTA.h"
#include "WiFiAP.h"
/*#include "WiFiScan.h"*/
#include "WiFiGeneric.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "WiFiUdp.h"

class WiFiClass : public WiFiGenericClass, public WiFiSTAClass, /*public WiFiScanClass,*/ public WiFiAPClass
{
public:
    using WiFiGenericClass::channel;

    using WiFiSTAClass::SSID;
    using WiFiSTAClass::RSSI;
    using WiFiSTAClass::BSSID;
    using WiFiSTAClass::BSSIDstr;

    /*using WiFiScanClass::SSID;
    using WiFiScanClass::encryptionType;
    using WiFiScanClass::RSSI;
    using WiFiScanClass::BSSID;
    using WiFiScanClass::BSSIDstr;
    using WiFiScanClass::channel;*/

public:
   void printDiag(Print& dest);
   void setports(sc_fifo<unsigned char> *_to, sc_fifo<unsigned char> *_from);
   friend class WiFiClient;
   friend class WiFiServer;
   friend class WiFiUDP;
};

extern WiFiClass WiFi;
extern TestSerial WiFiSerial;

#endif
