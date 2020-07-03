/*******************************************************************************
 * btclient.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include <stdarg.h>
#include "btclient.h"
#include "info.h"
#include <sys/types.h>

void btclient::send(void *msg, int len) {
   const unsigned char *c = (const unsigned char *)msg;
   int pos;

   for(pos = 0; pos < len; pos = pos + 1) {
      i_btchan.to.write((int)(c[pos]));
   }
}
