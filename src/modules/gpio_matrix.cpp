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
#include <string.h>
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
   mux_out *gmux;
   bool thisbit;
   gpio_out.write(((uint64_t)highbits << 32)|lowbits);
   for(bit = 0; bit < GPIOMATRIX_CNT; bit = bit + 1) {
      gmux = getmux(bit);
      if (gmux == NULL) continue;
      if (bit < 32) thisbit = (lowbits & (1<<bit)) > 0;
      else thisbit = (highbits & (1<<(bit-32))) > 0;

      /* If the GPIO function is selected, we drive it, but only if the value
       * is changing.
       */
      if (gmux->function == 256 && thisbit) gmux->mux(257);
      else if (gmux->function == 257 && !thisbit) gmux->mux(256);
   }
}

void gpio_matrix::setoebits(uint32_t highbits, uint32_t lowbits) {
   int bit;
   mux_out *gmux;
   bool thisbit;

   for(bit = 0; bit < GPIOMATRIX_CNT; bit = bit + 1) {
      gmux = getmux(bit);
      if (gmux == NULL) continue;
      if (bit < 32) thisbit = (lowbits & (1<<bit)) > 0;
      else thisbit = (highbits & (1<<(bit-32))) > 0;

      if (gmux->function == 258 && thisbit) {
         if(bit < 32 && (GPIO.out & (1<<bit)) > 0) gmux->mux(257);
         else if(bit >= 32 && (GPIO.out1.data & (1<<bit-32))>0) gmux->mux(257);
         else gmux->mux(256);
      }
      else if ((gmux->function == 256 || gmux->function == 257) && !thisbit)
         gmux->mux(258);
   }
}

mux_out *gpio_matrix::getmux(int pin) {
   if (pin >= GPIOMATRIX_CNT) return NULL;
   return muxptr[pin];
}
void gpio_matrix::initptr() {
   int g;

   muxptr[0] = &i_mux_out0;
   muxptr[1] = &i_mux_out1;
   muxptr[2] = &i_mux_out2;
   muxptr[3] = &i_mux_out3;
   muxptr[4] = &i_mux_out4;
   muxptr[5] = &i_mux_out5;
   muxptr[6] = NULL;
   muxptr[7] = NULL;
   muxptr[8] = NULL;
   muxptr[9] = NULL;
   muxptr[10] = NULL;
   muxptr[11] = NULL;
   muxptr[12] = &i_mux_out12;
   muxptr[13] = &i_mux_out13;
   muxptr[14] = &i_mux_out14;
   muxptr[15] = &i_mux_out15;
   muxptr[16] = &i_mux_out16;
   muxptr[17] = &i_mux_out17;
   muxptr[18] = &i_mux_out18;
   muxptr[19] = &i_mux_out19;
   muxptr[20] = NULL;
   muxptr[21] = &i_mux_out21;
   muxptr[22] = &i_mux_out22;
   muxptr[23] = &i_mux_out23;
   muxptr[24] = NULL;
   muxptr[25] = &i_mux_out25;
   muxptr[26] = &i_mux_out26;
   muxptr[27] = &i_mux_out27;
   muxptr[28] = NULL;
   muxptr[29] = NULL;
   muxptr[30] = NULL;
   muxptr[31] = NULL;
   muxptr[32] = &i_mux_out32;
   muxptr[33] = &i_mux_out33;
   muxptr[34] = &i_mux_out34;
   muxptr[35] = &i_mux_out35;
   muxptr[36] = &i_mux_out36;
   muxptr[37] = &i_mux_out37;
   muxptr[38] = &i_mux_out38;
   muxptr[39] = &i_mux_out39;

   /* We clear the GPIO struct and preset the functions to 256 and output enable
    * low, meaning GPIO selected but driving High-Z.
    */
   memset(&GPIO, 0, sizeof(gpio_dev_t));
   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) {
      GPIO.func_out_sel_cfg[g].func_sel = 256;
   }
   for(g = 0; g < 255; g = g + 1) {
      GPIO.func_in_sel_cfg[g].func_sel = GPIOMATRIX_LOGIC0;
   }
}

void gpio_matrix::updateth() {
   int bit;
   mux_out *gmux;
   while(true) {
      wait();
      /* If one of these registers was changed, we need to set every pin to
       * follow the setting, low or high.
       */
      if (updategpioreg_ev.triggered()) {
         setbits(GPIO.out1.data, GPIO.out);
         continue;
      }
      if (updategpiooe_ev.triggered()) {
         setoebits(GPIO.enable1.data, GPIO.enable);
         continue;
      }

      /* The set and clear registers change the direct registers and then
       * do the same operation. If a bit did not change it does not generate
       * a notification, so no harm is done.
       */
      if (GPIO.out_w1ts>0 || GPIO.out_w1tc>0 || GPIO.out1_w1ts.data>0
            || GPIO.out1_w1tc.data>0) {
         GPIO.out = (GPIO.out | GPIO.out_w1ts) & ~GPIO.out_w1tc;
         GPIO.out1.data =
            (GPIO.out1.data | GPIO.out1_w1ts.data) & ~GPIO.out1_w1tc.data;
         setbits(GPIO.out1.data, GPIO.out);
         /* And we clear the set and clear regs as they should be w/o. Plus,
          * this way we do not apply the flags multiple times.
          */
         GPIO.out_w1ts = 0; GPIO.out_w1tc = 0;
         GPIO.out1_w1ts.data = 0; GPIO.out1_w1tc.data = 0;
      }
      /* For the enables we do the same. */
      if (GPIO.enable_w1ts>0 || GPIO.enable_w1tc>0 || GPIO.enable1_w1ts.data>0||
            GPIO.enable1_w1tc.data>0) {
         GPIO.enable = (GPIO.enable | GPIO.enable_w1ts) & ~GPIO.enable_w1tc;
         GPIO.enable1.data = (GPIO.enable1.data | GPIO.enable1_w1ts.data)
            & ~GPIO.enable1_w1tc.data;
         setoebits(GPIO.enable1.data, GPIO.enable);
         /* And we clear the set and clear regs as they should be w/o. Plus,
          * this way we do not apply the flags multiple times.
          */
         GPIO.enable_w1ts = 0; GPIO.enable_w1tc = 0;
         GPIO.enable1_w1ts.data = 0; GPIO.enable1_w1tc.data = 0;
      }
      /* IN is handled by the return thread. */
      /* Strapping not yet implemented. */
      /* Interrupts not yet implemented. */
      /* RTC Out not yet implemented. */
      io_mux *gpin;
      i_mux_uart0.mux(GPIO.func_in_sel_cfg[U0RXD_IN_IDX].func_sel);
      i_mux_uart1.mux(GPIO.func_in_sel_cfg[U1RXD_IN_IDX].func_sel);
      i_mux_uart2.mux(GPIO.func_in_sel_cfg[U2RXD_IN_IDX].func_sel);
      /* There is a gap in the indexes, so we skip it. */
      for(bit = PCNT_SIG_CH0_IN0_IDX; bit < PCNT_CTRL_CH1_IN4_IDX; bit=bit+1) {
         i_mux_pcnt.mux(bit, GPIO.func_in_sel_cfg[bit].func_sel);
      }
      for(bit = PCNT_SIG_CH0_IN5_IDX; bit < PCNT_CTRL_CH1_IN7_IDX; bit=bit+1) {
         i_mux_pcnt.mux(bit, GPIO.func_in_sel_cfg[bit].func_sel);
      }
      /* Matrix Outputs
       * For these we need to do some translation:
       * - if a direct bypass was requested, we select the corresponding ALT
       *   function.
       * - if the GPIO function is selected we use function 1 as the muxes
       *   handle it.
       */
      for (bit = 0; bit < GPIOMATRIX_CNT; bit = bit + 1) {
         gpin = getgpio(bit);
         if (gpin == NULL) continue;
         if (GPIO.pin[bit].pad_driver) gpin->set_od();
         else gpin->clr_od();
         gmux = getmux(bit);
         if (gmux == NULL) continue;
         /* The function we use to drive the level. So if it is high or low,
          * we need to set the property accordingly.
          */
         if (GPIO.func_out_sel_cfg[bit].func_sel == 256) {
            if (bit < 32 && (GPIO.enable & (1<<bit))==0) gmux->mux(258);
            else if (bit>=32 && (GPIO.enable1.data&(1<<(bit-32)))==0)
               gmux->mux(258);
            else if (bit<32  && (GPIO.out&(1<<bit))>0) gmux->mux(257);
            else if (bit>=32 && (GPIO.out1.data&(1<<(bit-32)))>0)
               gmux->mux(257);
            else gmux->mux(256);
         }
         else gmux->mux(GPIO.func_out_sel_cfg[bit].func_sel);
      }
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

   sc_trace(tf, gpio_out, d_u0rx_s.name());
   sc_trace(tf, d_u0rx_s, d_u0rx_s.name());
   sc_trace(tf, d_u1rx_s, d_u1rx_s.name());
   sc_trace(tf, d_u2rx_s, d_u2rx_s.name());

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