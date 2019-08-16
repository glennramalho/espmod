/*******************************************************************************
 * gpiomatrix.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a model of the ESP32 GPIO matrix.
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
#include "gpio_matrix.h"
#include "setfield.h"
#include "soc/gpio_reg.h"
#include "gpioset.h"
#include "Arduino.h"

void gpio_matrix::start_of_simulation() {
   /* We need to set all unused GPIO mout_s signals to an initial value. The
    * others will simply store whatever maybe is driven onto them and that
    * is it.
    */
   mout_s[20].write(0); mout_s[24].write(0); mout_s[28].write(0);
   mout_s[29].write(0); mout_s[30].write(0); mout_s[31].write(0);
}

void gpio_matrix::setbits(uint32_t highbits, uint32_t lowbits) {
   int bit;
   gpio *gpin;
   for(bit = 0; bit < GPIOMATRIX_CNT; bit = bit + 1) {
      gpin = getgpio(bit);
      if (gpin == NULL) continue;
      if (bit < 32) gpin->set_val((lowbits & (1<<bit))>0);
      else gpin->set_val((highbits & (1<<bit))>0);
   }
}

void gpio_matrix::updateth() {
   int bit;
   while(true) {
      wait();
      /* If one of these registers was changed, we need to set every pin to
       * follow the setting, low or high.
       */
      if (updategpioreg_ev.triggered()) {
         setbits(sv.out1.data, sv.out);
         continue;
      }
      if (updategpiooe_ev.triggered()) {
         gpio_oe.write(((uint64_t)sv.enable1.data<<32) | sv.enable);
         continue;
      }

      /* The set and clear registers change the direct registers and then
       * do the same operation. If a bit did not change it does not generate
       * a notification, so no harm is done.
       */
      if (sv.out_w1ts>0 || sv.out_w1tc>0 || sv.out1_w1ts.data>0
            || sv.out1_w1tc.data>0) {
         sv.out = (sv.out | sv.out_w1ts) & ~sv.out_w1tc;
         sv.out1.data = (sv.out1.data | sv.out1_w1ts.data) & ~sv.out1_w1tc.data;
         setbits(sv.out1.data, sv.out);
         /* And we clear the set and clear regs as they should be w/o. Plus,
          * this way we do not apply the flags multiple times.
          */
         sv.out_w1ts = 0; sv.out_w1tc = 0;
         sv.out1_w1ts.data = 0; sv.out1_w1tc.data = 0;
      }
      /* For the enables we do the same. */
      if (sv.enable_w1ts>0 || sv.enable_w1tc>0 || sv.enable1_w1ts.data>0 ||
            sv.enable1_w1tc.data>0) {
         sv.enable = (sv.enable | sv.enable_w1ts) & ~sv.enable_w1tc;
         sv.enable1.data = (sv.enable1.data | sv.enable1_w1ts.data)
            & ~sv.enable1_w1tc.data;
         gpio_oe.write(((uint64_t)sv.enable1.data<<32) | sv.enable);
         /* And we clear the set and clear regs as they should be w/o. Plus,
          * this way we do not apply the flags multiple times.
          */
         sv.enable_w1ts = 0; sv.enable_w1tc = 0;
         sv.enable1_w1ts.data = 0; sv.enable1_w1tc.data = 0;
      }
      /* IN is handled by the return thread. */
      /* Strapping not yet implemented. */
      /* Interrupts not yet implemented. */
      /* RTC Out not yet implemented. */
      gpio *gpin;
      for(bit = 0; bit < GPIOMATRIX_CNT; bit = bit + 1) {
         gpin = getgpio(bit);
         if (gpin == NULL) continue;
         if (sv.pin[bit].pad_driver) gpin->set_od();
         else gpin->clr_od();
      }
      for(bit = 0; bit < 255; bit = bit + 1) {
         if (bit == U0RXD_IN_IDX)
            i_mux_uart0.mux(sv.func_in_sel_cfg[bit].func_sel);
         else if (bit == U1RXD_IN_IDX)
            i_mux_uart1.mux(sv.func_in_sel_cfg[bit].func_sel);
         else if (bit == U2RXD_IN_IDX)
            i_mux_uart2.mux(sv.func_in_sel_cfg[bit].func_sel);
         else if (bit >= PCNT_SIG_CH0_IN0_IDX && bit <= PCNT_CTRL_CH1_IN4_IDX ||
               bit >= PCNT_SIG_CH0_IN5_IDX && bit <= PCNT_CTRL_CH1_IN7_IDX)
            i_mux_pcnt.mux(bit, sv.func_in_sel_cfg[bit].func_sel);
      }
      /* add matrix outs */
   }
}

void gpio_matrix::update() {
   update_ev.notify();
   del1cycle();
}

void gpio_matrix::updategpioreg() {
   updategpioreg_ev.notify();
   del1cycle();
}

void gpio_matrix::updategpiooe() {
   updategpiooe_ev.notify();
   del1cycle();
}

void gpio_matrix::trace(sc_trace_file *tf) {
   int g;
   std::string sigb = name();
   std::string sign;

   sc_trace(tf, d_u0rx_s, d_u0rx_s.name());
   sc_trace(tf, d_u1rx_s, d_u1rx_s.name());
   sc_trace(tf, d_u2rx_s, d_u2rx_s.name());
   sc_trace(tf, gpio_oe, gpio_oe.name());

   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) {
      sign = sigb + ".men_s_" + std::to_string(g);
      sc_trace(tf, men_s[g], sign.c_str());
   }
   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) {
      sign = sigb + ".min_s_" + std::to_string(g);
      sc_trace(tf, min_s[g], sign.c_str());
   }
   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) {
      sign = sigb + ".mout_s_" + std::to_string(g);
      sc_trace(tf, mout_s[g], sign.c_str());
   }
}
