/*******************************************************************************
 * btclient.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a model for a net client. It can behave as a web client or server,
 *   a NTP server or a MQTT Broker. This file does not implement all the
 *   details of these protocols, it simply provides a way for a testbench to
 *   generate stimulus to emulate these protocols.
 *******************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************
 */

#ifndef _BTCLIENT_H
#define _BTCLIENT_H

#include <systemc.h>
#include "cchan.h"

SC_MODULE(btclient) {
   sc_in<unsigned int> rx {"rx"};
   sc_out<unsigned int> tx {"tx"};
   cchan i_btchan{"i_btchan", 256, 256};

   SC_CTOR(btclient) {
      i_btchan.tx(tx);
      i_btchan.rx(rx);
   }

   /* Channel send */
   void send(void *msg, int len);
   void send(const char *msg) { send((void *)msg, strlen(msg)); }
   void send(std::string msg) { send((void *)msg.c_str(), msg.length()); }
   std::string get();
   void expect(const char *string);
};

#endif
