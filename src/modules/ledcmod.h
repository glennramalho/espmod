/*******************************************************************************
 * ledchschmod.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 PWM High-speed channel
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

#ifndef _LEDCHSCHMOD_H
#define _LEDCHSCHMOD_H

#include <systemc.h>

SC_MODULE(ledchschmod) {
   public:
   sc_out<bool> sig_out {"sig_out"};
   sc_in<uint32_t> timer {"timer"};
   sc_signal<uint32_t> conf0_reg {"conf0_reg", 0};
   sc_signal<uint32_t> hpoint_i {"hpoint_i", 0};
   sc_signal<uint32_t> lpoint_i {"lpoint_i", 0};
   sc_signal<sc_time> timestep_i {"timestep_i", 0};
   sc_signal<uint32_t> conf1_reg {"conf1_reg", 0};
   sc_signal<uint32_t> duty_r_reg {"duty_r_reg", 0};

   sc_time hpoint, lpoint;

   int div;
   void drive() {
      while(1) {
         start = sc_time_stamp();
         wait(div * apb_clk * ticks);
         ov.trigger();
      }
   }
   uint32_t readtimer(int tn) {
      if (timer_running[tn]) {
         start_timer
      }
   }

   SC_CTOR(ledchschmod) {
      s
      thiscyc = 0;
   }

   void start_of_simulation() {
      sig_out.write(false);
   }


