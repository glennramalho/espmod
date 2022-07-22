/*******************************************************************************
 * pn532_hsu.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a crude model for the PN532 with firmware v1.6. It will work for
 *   basic work with a PN532 system. This is the HSU version.
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

#ifndef _PN532_HSU_H
#define _PN532_HSU_H

#include <systemc.h>
#include "gn_mixed.h"
#include "pn532_base.h"

struct pn532_hsu : public pn532_base {
   /* Signals */
   sc_in<gn_mixed> rx {"rx"};
   sc_out<gn_mixed> tx {"tx"};

   /* Functions */
   void set_baud(unsigned int rate);
   void set_debug(bool on) { debug = on; }
   bool get_debug() { return debug; }
   void set_stop(int _sp) { stopbits = _sp; }
   int get_stop() { return stopbits; }

   /* Processes */
   void rx_th(void);
   void tx_th(void);

   /* Internal Variables */
   private:
   sc_time baudperiod;
   int stopbits; /* 0: invalid, 1: one bit, 2: 1.5 bits, 3: 2 bits */
   unsigned int baudrate;
   bool debug;

   // Constructor
   public:
   pn532_hsu(sc_module_name name): pn532_base(name) {
      set_baud(115200);
      stopbits = 3;

      SC_THREAD(rx_th);
      SC_THREAD(tx_th);
   }

   SC_HAS_PROCESS(pn532_hsu);
};

#endif
