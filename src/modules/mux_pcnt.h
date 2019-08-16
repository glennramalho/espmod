/*******************************************************************************
 * mux_pcnt.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Model for the PCNT mux in the GPIO matrix.
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

#ifndef _MUX_PCNT_H
#define _MUX_PCNT_H

#include <systemc.h>
#include "pcntbus.h"

SC_MODULE(mux_pcnt) {
   sc_port<sc_signal_in_if<bool>,0> mout_i {"mout_i"};
   sc_port<sc_signal_out_if<pcntbus_t>,8> pcntbus_o {"pcntbus_o"};

   int function[8][4];
   sc_event fchange_ev;

   /* Functions */
   void mux(int bit, int gpiosel);
   void initialize();

   /* Threads */
   void transfer(int function);

   SC_CTOR(mux_pcnt) { initialize(); }
};

#endif
