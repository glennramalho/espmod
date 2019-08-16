/*******************************************************************************
 * mux_pcnt.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc.h>
#include "soc/gpio_sig_map.h"
#include "gpio_matrix.h"
#include "mux_pcnt.h"
#include "gpioset.h"
#include "info.h"

void mux_pcnt::initialize() {
   int unit;
   for (unit = 0; unit < 8; unit = unit + 1) {
      /* We start off pointing all functions to a non-existant GPIO. There
       * is a signal there, but it is tied to logic_0, so we can use it in
       * a wait statement, but it will never be triggered.
       */
      function[unit][0] = GPIOMATRIX_LOGIC0;
      function[unit][1] = GPIOMATRIX_LOGIC0;
      function[unit][2] = GPIOMATRIX_LOGIC0;
      function[unit][3] = GPIOMATRIX_LOGIC0;

      /* And we spawn a thread to monitor the functions for each PCNT unit. */
      sc_spawn(sc_bind(&mux_pcnt::transfer, this, unit));
   }
}

void mux_pcnt::mux(int bit, int gpiosel) {
   int *func;
   /* 0-39 is for GPIO 0-39.
    * 40 is for the direct signal.
    * 41 is logic_0
    * 42 is logic_1
    */
   if (gpiosel >= GPIOMATRIX_ALL || gpiosel<GPIOMATRIX_CNT
         && getgpio(gpiosel)==NULL) {
      PRINTF_WARN("MUXGPIO",
        "Attempting to set GPIO Matrix to illegal pin %d.", gpiosel);
      return;
   }
   /* We need to find the correct function flag */
   switch(bit) {
      case PCNT_SIG_CH0_IN0_IDX: func = &(function[0][0]); break;
      case PCNT_SIG_CH0_IN1_IDX: func = &(function[1][0]); break;
      case PCNT_SIG_CH0_IN2_IDX: func = &(function[2][0]); break;
      case PCNT_SIG_CH0_IN3_IDX: func = &(function[3][0]); break;
      case PCNT_SIG_CH0_IN4_IDX: func = &(function[4][0]); break;
      case PCNT_SIG_CH0_IN5_IDX: func = &(function[5][0]); break;
      case PCNT_SIG_CH0_IN6_IDX: func = &(function[6][0]); break;
      case PCNT_SIG_CH0_IN7_IDX: func = &(function[7][0]); break;
      case PCNT_SIG_CH1_IN0_IDX: func = &(function[0][1]); break;
      case PCNT_SIG_CH1_IN1_IDX: func = &(function[1][1]); break;
      case PCNT_SIG_CH1_IN2_IDX: func = &(function[2][1]); break;
      case PCNT_SIG_CH1_IN3_IDX: func = &(function[3][1]); break;
      case PCNT_SIG_CH1_IN4_IDX: func = &(function[4][1]); break;
      case PCNT_SIG_CH1_IN5_IDX: func = &(function[5][1]); break;
      case PCNT_SIG_CH1_IN6_IDX: func = &(function[6][1]); break;
      case PCNT_SIG_CH1_IN7_IDX: func = &(function[7][1]); break;
      case PCNT_CTRL_CH0_IN0_IDX:func = &(function[0][2]); break;
      case PCNT_CTRL_CH0_IN1_IDX:func = &(function[1][2]); break;
      case PCNT_CTRL_CH0_IN2_IDX:func = &(function[2][2]); break;
      case PCNT_CTRL_CH0_IN3_IDX:func = &(function[3][2]); break;
      case PCNT_CTRL_CH0_IN4_IDX:func = &(function[4][2]); break;
      case PCNT_CTRL_CH0_IN5_IDX:func = &(function[5][2]); break;
      case PCNT_CTRL_CH0_IN6_IDX:func = &(function[6][2]); break;
      case PCNT_CTRL_CH0_IN7_IDX:func = &(function[7][2]); break;
      case PCNT_CTRL_CH1_IN0_IDX:func = &(function[0][3]); break;
      case PCNT_CTRL_CH1_IN1_IDX:func = &(function[1][3]); break;
      case PCNT_CTRL_CH1_IN2_IDX:func = &(function[2][3]); break;
      case PCNT_CTRL_CH1_IN3_IDX:func = &(function[3][3]); break;
      case PCNT_CTRL_CH1_IN4_IDX:func = &(function[4][3]); break;
      case PCNT_CTRL_CH1_IN5_IDX:func = &(function[5][3]); break;
      case PCNT_CTRL_CH1_IN6_IDX:func = &(function[6][3]); break;
      default:                   func = &(function[7][3]); break;
   }
   /* We only raise the flag if the function is changing. We do not want
    * to trigger events for nothing.
    */
   if (*func != gpiosel) {
      *func = gpiosel;
      fchange_ev.notify();
   }
}

void mux_pcnt::transfer(int unit) {
   pcntbus_t p;
   while(true) {
      /* We simply copy the input onto the output. */
      p.sig_ch0 = mout_i[function[unit][0]]->read();
      p.sig_ch1 = mout_i[function[unit][1]]->read();
      p.ctrl_ch0 = mout_i[function[unit][2]]->read();
      p.ctrl_ch1 = mout_i[function[unit][3]]->read();
      pcntbus_o[unit]->write(p);

      /* We wait for a function change or a signal change. */
      wait(fchange_ev | mout_i[function[unit][0]]->value_changed_event() |
         mout_i[function[unit][1]]->value_changed_event() |
         mout_i[function[unit][2]]->value_changed_event() |
         mout_i[function[unit][3]]->value_changed_event());
   }
}
