/*******************************************************************************
 * ledcmod.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 PWM LEDC module.
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

#ifndef _LEDCMOD_H
#define _LEDCMOD_H

#include <systemc.h>

SC_MODULE(ledcmod) {
   public:
   sc_out<bool> sig_out[16];
   sc_signal<uint32_t> conf0[16];
   sc_signal<uint32_t> conf1[16];
   sc_signal<uint32_t> hpoint[16];
   sc_signal<uint32_t> duty[16];
   sc_signal<uint32_t> duty_r[16];
   sc_signal<uint32_t> timer_cnt[8];
   sc_signal<uint32_t> timer_lim[8];
   sc_signal<uint32_t> int_st {"int_st"};
   sc_signal<uint32_t> int_ena {"int_ena"};
   sc_signal<uint32_t> int_clr {"int_clr"};

   /* Functions */
   void update();
   void initstruct();
   void calc_points(int un, bool start_dither);

   /* Threads */
   void channel(int ch);
   void timer(int tim);
   void updateth(void);
   void returnth(void);

   /* Variables */
   int thislp[16];
   int thiscyc[16];
   sc_event update_ev;

   SC_CTOR(ledcmod) {
      initstruct();

      SC_THREAD(updateth);
      sesitive << update_ev;

      SC_THREAD(returnth);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
extern ledcmod *ledcpntr;
