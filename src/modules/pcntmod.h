/*******************************************************************************
 * pcntmod.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 Pulse Counter
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

#ifndef _PCNTMOD_H
#define _PCNTMOD_H

#include <systemc.h>
#include "pcntbus.h"
struct pcntmod;
#include "soc/pcnt_struct.h"

SC_MODULE(pcntmod) {
   sc_signal<uint32_t> conf0[8];
   sc_signal<uint32_t> conf1[8];
   sc_signal<uint32_t> conf2[8];
   sc_signal<uint16_t> cnt_unit[8];
   sc_signal<uint32_t> int_raw[8];
   sc_signal<uint32_t> int_st {"int_st"};
   sc_signal<uint32_t> int_ena {"int_ena"};
   sc_signal<uint32_t> int_clr {"int_clr"};
   sc_signal<uint32_t> status_unit[8];
   sc_signal<uint32_t> ctrl {"ctrl"};
   pcnt_dev_t sv;
   
   sc_port<sc_signal_in_if<pcntbus_t>,0> pcntbus_i;

   /* Functions */
   void docnt(int un, bool siglvl, bool ctrllvl, int ch);
   void update();
   void initstruct();

   /* Threads */
   void capture(int un);
   void count(int un);
   void updateth(void);
   void returnth(void);

   /* Variables */
   sc_time fctrl0[8];
   sc_time fctrl1[8];
   sc_event filtered_sig0[8];
   sc_event filtered_sig1[8];
   sc_event reset_un[8];
   sc_event update_ev;

   /* Sets initial drive condition. */
   SC_CTOR(pcntmod) {
      initstruct();

      SC_THREAD(updateth);
      sensitive << update_ev;

      SC_THREAD(returnth);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
extern pcntmod *pcntptr;

#endif
