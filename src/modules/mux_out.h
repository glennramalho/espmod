/*******************************************************************************
 * mux_out.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#ifndef _MUX_OUT_H
#define _MUX_OUT_H

#include <systemc.h>

#define MUXOUT_DRIVE_0 256
#define MUXOUT_DRIVE_1 257
#define MUXOUT_DRIVE_Z 258

SC_MODULE(mux_out) {
   sc_out<bool> min_o {"min_o"};
   sc_out<bool> men_o {"men_o"};
   sc_in<bool> uart0tx_i{"uart0tx_i"};
   sc_in<bool> uart1tx_i{"uart1tx_i"};
   sc_in<bool> uart2tx_i{"uart2tx_i"};
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_hs_i{"ledc_sig_hs_i"};
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_ls_i{"ledc_sig_ls_i"};

   int function;
   sc_event fchange_ev;

   /* Functions */
   void mux(int funcsel);

   /* Threads */
   void transfer(void);

   SC_CTOR(mux_out) {
      function = MUXOUT_DRIVE_Z;
      SC_THREAD(transfer);
   }
};

#endif
