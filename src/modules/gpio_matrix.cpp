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
#include "soc/gpio_sig_map.h"
#include "clockpacer.h"

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

   /* We need to get the powerup defaults of the GPIOs. */
   io_mux *gpin;
   GPIO.enable = 0;
   GPIO.enable1.data = 0;
   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) {
      gpin = getgpio(g);
      if (gpin != NULL) {
         GPIO.pin[g].pad_driver = gpin->get_od();
         if (gpin->get_ie() && g < 32) GPIO.enable = GPIO.enable | (1 << g);
         else if (gpin->get_ie())
            GPIO.enable1.data = GPIO.enable1.data | (1 << (g-32));
      }
   }

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

void gpio_matrix::applycsbits() {
   GPIO.out = GPIO.out & ~GPIO.out_w1tc | GPIO.out_w1ts;
   GPIO.out1.data = GPIO.out1.data & ~GPIO.out1_w1tc.data | GPIO.out1_w1ts.data;
   GPIO.out_w1ts = 0; GPIO.out_w1tc = 0;
   GPIO.out1_w1ts.data = 0; GPIO.out1_w1tc.data = 0;
}

void gpio_matrix::applyoecsbits() {
   GPIO.enable = GPIO.enable & ~GPIO.enable_w1tc | GPIO.enable_w1ts;
   GPIO.enable1.data =
      GPIO.enable1.data & ~GPIO.enable1_w1tc.data | GPIO.enable1_w1ts.data;
   GPIO.enable_w1ts = 0; GPIO.enable_w1tc = 0;
   GPIO.enable1_w1ts.data = 0; GPIO.enable1_w1tc.data = 0;
}

void gpio_matrix::updateth() {
   int bit;
   mux_out *gmux;
   while(true) {
      wait();
      /* If one of these registers was changed and the other fields are not
       * checked.
       */
      if (updategpioreg_ev.triggered()) {
         applycsbits();
         setbits(GPIO.out1.data, GPIO.out);
         continue;
      }
      if (updategpiooe_ev.triggered()) {
         applyoecsbits();
         setoebits(GPIO.enable1.data, GPIO.enable);
         continue;
      }

      /* Any other notify will check every field. */
      applycsbits();
      setbits(GPIO.out1.data, GPIO.out);
      applyoecsbits();
      setoebits(GPIO.enable1.data, GPIO.enable);
      /* IN is handled by the return thread. */
      /* Strapping not yet implemented. */
      /* Interrupts not yet implemented. */
      /* RTC Out not yet implemented. */
      io_mux *gpin;
      i_mux_uart0.mux(GPIO.func_in_sel_cfg[U0RXD_IN_IDX].func_sel);
      i_mux_uart1.mux(GPIO.func_in_sel_cfg[U1RXD_IN_IDX].func_sel);
      i_mux_uart2.mux(GPIO.func_in_sel_cfg[U2RXD_IN_IDX].func_sel);

      /* VSPI */
      i_mux_vspi_d.mux(GPIO.func_in_sel_cfg[VSPID_IN_IDX].func_sel);
      i_mux_vspi_q.mux(GPIO.func_in_sel_cfg[VSPIQ_IN_IDX].func_sel);
      i_mux_vspi_clk.mux(GPIO.func_in_sel_cfg[VSPICLK_IN_IDX].func_sel);
      i_mux_vspi_hd.mux(GPIO.func_in_sel_cfg[VSPIHD_IN_IDX].func_sel);
      i_mux_vspi_wp.mux(GPIO.func_in_sel_cfg[VSPIWP_IN_IDX].func_sel);
      i_mux_vspi_cs0.mux(GPIO.func_in_sel_cfg[VSPICS0_IN_IDX].func_sel);
      i_mux_vspi_cs1.mux(GPIO.func_in_sel_cfg[VSPICS1_IN_IDX].func_sel);
      i_mux_vspi_cs2.mux(GPIO.func_in_sel_cfg[VSPICS2_IN_IDX].func_sel);

      /* HSPI */
      i_mux_hspi_d.mux(GPIO.func_in_sel_cfg[HSPID_IN_IDX].func_sel);
      i_mux_hspi_q.mux(GPIO.func_in_sel_cfg[HSPIQ_IN_IDX].func_sel);
      i_mux_hspi_clk.mux(GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].func_sel);
      i_mux_hspi_hd.mux(GPIO.func_in_sel_cfg[HSPIHD_IN_IDX].func_sel);
      i_mux_hspi_wp.mux(GPIO.func_in_sel_cfg[HSPIWP_IN_IDX].func_sel);
      i_mux_hspi_cs0.mux(GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel);

      /* I2C */
      i_mux_i2c_sda0.mux(GPIO.func_in_sel_cfg[I2CEXT0_SDA_IN_IDX].func_sel);
      i_mux_i2c_sda1.mux(GPIO.func_in_sel_cfg[I2CEXT1_SDA_IN_IDX].func_sel);
      i_mux_i2c_scl0.mux(GPIO.func_in_sel_cfg[I2CEXT0_SCL_IN_IDX].func_sel);
      i_mux_i2c_scl1.mux(GPIO.func_in_sel_cfg[I2CEXT1_SCL_IN_IDX].func_sel);

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
   if(clockpacer.is_thread()) clockpacer.wait_next_apb_clk();
}

void gpio_matrix::updategpioreg() {
   updategpioreg_ev.notify();
   if(clockpacer.is_thread()) clockpacer.wait_next_apb_clk();
}

void gpio_matrix::updategpiooe() {
   updategpiooe_ev.notify();
   if(clockpacer.is_thread()) clockpacer.wait_next_apb_clk();
}

void gpio_matrix::trace(sc_trace_file *tf) {
   int g;
   std::string sigb = name();
   std::string sign;

   i_mux_out0.trace(tf);
   i_mux_out1.trace(tf);
   i_mux_out2.trace(tf);
   i_mux_out3.trace(tf);
   i_mux_out4.trace(tf);
   i_mux_out5.trace(tf);
   i_mux_out12.trace(tf);
   i_mux_out13.trace(tf);
   i_mux_out14.trace(tf);
   i_mux_out15.trace(tf);
   i_mux_out16.trace(tf);
   i_mux_out17.trace(tf);
   i_mux_out18.trace(tf);
   i_mux_out19.trace(tf);
   i_mux_out21.trace(tf);
   i_mux_out22.trace(tf);
   i_mux_out23.trace(tf);
   i_mux_out25.trace(tf);
   i_mux_out26.trace(tf);
   i_mux_out27.trace(tf);
   i_mux_out32.trace(tf);
   i_mux_out33.trace(tf);
   i_mux_out34.trace(tf);
   i_mux_out35.trace(tf);
   i_mux_out36.trace(tf);
   i_mux_out37.trace(tf);
   i_mux_out38.trace(tf);
   i_mux_out39.trace(tf);

   sc_trace(tf, gpio_out, gpio_out.name());
   sc_trace(tf, d_u0rx_s, d_u0rx_s.name());
   sc_trace(tf, d_u1rx_s, d_u1rx_s.name());
   sc_trace(tf, d_u2rx_s, d_u2rx_s.name());
   sc_trace(tf, d_hspi_d_in_s, d_hspi_d_in_s.name());
   sc_trace(tf, d_hspi_q_in_s, d_hspi_q_in_s.name());
   sc_trace(tf, d_hspi_clk_in_s, d_hspi_clk_in_s.name());
   sc_trace(tf, d_hspi_hd_in_s, d_hspi_hd_in_s.name());
   sc_trace(tf, d_hspi_wp_in_s, d_hspi_wp_in_s.name());
   sc_trace(tf, d_hspi_cs0_in_s, d_hspi_cs0_in_s.name());
   sc_trace(tf, d_vspi_d_in_s, d_vspi_d_in_s.name());
   sc_trace(tf, d_vspi_q_in_s, d_vspi_q_in_s.name());
   sc_trace(tf, d_vspi_clk_in_s, d_vspi_clk_in_s.name());
   sc_trace(tf, d_vspi_hd_in_s, d_vspi_hd_in_s.name());
   sc_trace(tf, d_vspi_wp_in_s, d_vspi_wp_in_s.name());
   sc_trace(tf, d_vspi_cs0_in_s, d_vspi_cs0_in_s.name());
   sc_trace(tf, d_vspi_cs1_in_s, d_vspi_cs1_in_s.name());
   sc_trace(tf, d_vspi_cs2_in_s, d_vspi_cs2_in_s.name());

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
