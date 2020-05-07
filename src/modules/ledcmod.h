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

#define LEDC_CHANNELS 16
#define LEDC_CYCLES 16
#define LEDC_TIMERS 8
#define LEDC_INTR 24

SC_MODULE(ledcmod) {
   public:
   /* Ports */
   sc_port<sc_signal_out_if<bool>,LEDC_CHANNELS> sig_out_hs_o;
   sc_port<sc_signal_out_if<bool>,LEDC_CHANNELS> sig_out_ls_o;
   sc_out<bool> intr_o;

   /* Regs */
   sc_signal<uint32_t> conf0[LEDC_CHANNELS];
   sc_signal<uint32_t> conf1[LEDC_CHANNELS];
   sc_signal<uint32_t> hpoint[LEDC_CHANNELS];
   sc_signal<uint32_t> duty[LEDC_CHANNELS];
   sc_signal<uint32_t> duty_r[LEDC_CHANNELS];
   sc_signal<sc_time> timerinc[LEDC_TIMERS];
   sc_signal<uint32_t> timer_cnt[LEDC_TIMERS];
   sc_signal<uint32_t> timer_lim[LEDC_TIMERS];
   sc_signal<uint32_t> int_ena {"int_ena"};

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
   int dithtimes[LEDC_CHANNELS];
   int dithcycles[LEDC_CHANNELS];
   int thislp[LEDC_CHANNELS];
   int thiscyc[LEDC_CHANNELS];
   sc_event update_ev;
   sc_event timer_ev[LEDC_TIMERS];
   sc_event int_ev[LEDC_INTR];
   sc_event int_clr_ev;

   SC_CTOR(ledcmod) {
      initstruct();

      SC_THREAD(updateth);
      sensitive << update_ev;

      SC_THREAD(returnth);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
extern ledcmod *ledcptr;

#endif
