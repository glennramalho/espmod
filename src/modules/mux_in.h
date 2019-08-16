/*******************************************************************************
 * mux_in.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Model for the input signal mux for the GPIO Matrix.
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

#ifndef _MUX_IN_H
#define _MUX_IN_H

#include <systemc.h>

SC_MODULE(mux_in) {
   sc_out<bool> out_o {"out_o"};
   sc_port<sc_signal_in_if<bool>,0> mout_i {"mout_i"};

   int function;
   sc_event fchange_ev;

   /* Functions */
   void mux(int gpiosel);

   /* Threads */
   void transfer(void);

   mux_in(sc_module_name name, int initialfunc) {
      function = initialfunc;
      SC_THREAD(transfer);
   }
   SC_HAS_PROCESS(mux_in);
};

#endif
