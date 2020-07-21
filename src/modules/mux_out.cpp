/*******************************************************************************
 * mux_out.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "soc/gpio_sig_map.h"
#include "mux_out.h"
#include "info.h"

void mux_out::mux(int funcsel) {
   function.write(funcsel);
}

void mux_out::transfer() {
   /* These default to low. */
   mout_o.write(false);
   men_o.write(false);

   while(true) {
      switch(function) {
         /* SPI */
         case HSPICLK_OUT_IDX:
            mout_o.write(hspi_clk_out_i.read());
            men_o.write(hspi_clk_oen_i.read());
            wait(function.default_event() | hspi_clk_out_i.default_event()
               | hspi_clk_oen_i.default_event());
            break;
         case HSPIQ_OUT_IDX:
            mout_o.write(hspi_q_out_i.read());
            men_o.write(hspi_q_oen_i.read());
            wait(function.default_event() | hspi_q_out_i.default_event()
               | hspi_q_oen_i.default_event());
            break;
         case HSPID_OUT_IDX:
            mout_o.write(hspi_d_out_i.read());
            men_o.write(hspi_d_oen_i.read());
            wait(function.default_event() | hspi_d_out_i.default_event()
               | hspi_d_oen_i.default_event());
            break;
         case HSPIHD_OUT_IDX:
            mout_o.write(hspi_hd_out_i.read());
            men_o.write(hspi_hd_oen_i.read());
            wait(function.default_event() | hspi_hd_out_i.default_event()
               | hspi_hd_oen_i.default_event());
            break;
         case HSPIWP_OUT_IDX:
            mout_o.write(hspi_wp_out_i.read());
            men_o.write(hspi_wp_oen_i.read());
            wait(function.default_event() | hspi_wp_out_i.default_event()
               | hspi_wp_oen_i.default_event());
            break;
         case HSPICS0_OUT_IDX:
            mout_o.write(hspi_cs0_out_i.read());
            men_o.write(hspi_cs0_oen_i.read());
            wait(function.default_event() | hspi_cs0_out_i.default_event()
               | hspi_cs0_oen_i.default_event());
            break;
         case VSPICLK_OUT_IDX:
            mout_o.write(vspi_clk_out_i.read());
            men_o.write(vspi_clk_oen_i.read());
            wait(function.default_event() | vspi_clk_out_i.default_event()
               | vspi_clk_oen_i.default_event());
            break;
         case VSPIQ_OUT_IDX:
            mout_o.write(vspi_q_out_i.read());
            men_o.write(vspi_q_oen_i.read());
            wait(function.default_event() | vspi_q_out_i.default_event()
               | vspi_q_oen_i.default_event());
            break;
         case VSPID_OUT_IDX:
            mout_o.write(vspi_d_out_i.read());
            men_o.write(vspi_d_oen_i.read());
            wait(function.default_event() | vspi_d_out_i.default_event()
               | vspi_d_oen_i.default_event());
            break;
         case VSPIHD_OUT_IDX:
            mout_o.write(vspi_hd_out_i.read());
            men_o.write(vspi_hd_oen_i.read());
            wait(function.default_event() | vspi_hd_out_i.default_event()
               | vspi_hd_oen_i.default_event());
            break;
         case VSPIWP_OUT_IDX:
            mout_o.write(vspi_wp_out_i.read());
            men_o.write(vspi_wp_oen_i.read());
            wait(function.default_event() | vspi_wp_out_i.default_event()
               | vspi_wp_oen_i.default_event());
            break;
         case VSPICS0_OUT_IDX:
            mout_o.write(vspi_cs0_out_i.read());
            men_o.write(vspi_cs0_oen_i.read());
            wait(function.default_event() | vspi_cs0_out_i.default_event()
               | vspi_cs0_oen_i.default_event());
            break;
         case VSPICS1_OUT_IDX:
            mout_o.write(vspi_cs1_out_i.read());
            men_o.write(vspi_cs1_oen_i.read());
            wait(function.default_event() | vspi_cs0_out_i.default_event()
               | vspi_cs1_oen_i.default_event());
            break;
         case VSPICS2_OUT_IDX:
            mout_o.write(vspi_cs2_out_i.read());
            men_o.write(vspi_cs2_oen_i.read());
            wait(function.default_event() | vspi_cs2_out_i.default_event()
               | vspi_cs2_oen_i.default_event());
            break;
         /* UARTS */
         case U0TXD_OUT_IDX:
            mout_o.write(uart0tx_i.read()); men_o.write(true);
            wait(function.default_event() | uart0tx_i.default_event());
            break;
         case U1TXD_OUT_IDX:
            mout_o.write(uart1tx_i.read()); men_o.write(true);
            wait(function.default_event() | uart1tx_i.default_event());
            break;
         case U2TXD_OUT_IDX:
            mout_o.write(uart2tx_i.read()); men_o.write(true);
            wait(function.default_event() | uart2tx_i.default_event());
            break;
         /* LEDC */
         case LEDC_HS_SIG_OUT0_IDX:
         case LEDC_HS_SIG_OUT1_IDX:
         case LEDC_HS_SIG_OUT2_IDX:
         case LEDC_HS_SIG_OUT3_IDX:
         case LEDC_HS_SIG_OUT4_IDX:
         case LEDC_HS_SIG_OUT5_IDX:
         case LEDC_HS_SIG_OUT6_IDX:
         case LEDC_HS_SIG_OUT7_IDX:
            if (ledc_sig_hs_i.size() > function - LEDC_HS_SIG_OUT0_IDX) {
               mout_o.write(ledc_sig_hs_i[function-LEDC_HS_SIG_OUT0_IDX]->read());
               men_o.write(true);
               wait(function.default_event() |
                 ledc_sig_hs_i[function-LEDC_HS_SIG_OUT0_IDX]->default_event());
            }
            else {
               PRINTF_WARN("MUXOUT", "Illegal LEDC HS accessed");
               mout_o.write(false);
               men_o.write(false);
               wait(function.default_event());
            }
            break;
         case LEDC_LS_SIG_OUT0_IDX:
         case LEDC_LS_SIG_OUT1_IDX:
         case LEDC_LS_SIG_OUT2_IDX:
         case LEDC_LS_SIG_OUT3_IDX:
         case LEDC_LS_SIG_OUT4_IDX:
         case LEDC_LS_SIG_OUT5_IDX:
         case LEDC_LS_SIG_OUT6_IDX:
         case LEDC_LS_SIG_OUT7_IDX:
            if (ledc_sig_ls_i.size() > function - LEDC_LS_SIG_OUT0_IDX) {
               mout_o.write(ledc_sig_ls_i[function-LEDC_LS_SIG_OUT0_IDX]->read());
               men_o.write(true);
               wait(function.default_event() |
                 ledc_sig_ls_i[function-LEDC_LS_SIG_OUT0_IDX]->default_event());
            }
            else {
               PRINTF_WARN("MUXOUT", "Illegal LEDC LS accessed");
               mout_o.write(false);
               men_o.write(false);
               wait(function.default_event());
            }
            break;
         /* I2C */
         case I2CEXT0_SDA_OUT_IDX:
            mout_o.write(false);
            men_o.write(sda0_en_i);
            wait(function.default_event() | sda0_en_i.default_event());
            break;
         case I2CEXT1_SDA_OUT_IDX:
            mout_o.write(false);
            men_o.write(sda1_en_i);
            wait(function.default_event() | sda1_en_i.default_event());
            break;
         case I2CEXT0_SCL_OUT_IDX:
            mout_o.write(false);
            men_o.write(scl0_en_i);
            wait(function.default_event() | scl0_en_i.default_event());
            break;
         case I2CEXT1_SCL_OUT_IDX:
            mout_o.write(false);
            men_o.write(scl1_en_i);
            wait(function.default_event() | scl1_en_i.default_event());
            break;
         /* function 256 is logic 0 and function 257 is logic 1. This is
          * different from the spec, so the GPIO matrix selects the correct
          * one depending on the value to drive.
          */
         case MUXOUT_DRIVE_0: /* Function unknown and function 256 */
            mout_o.write(false); men_o.write(true);
            wait(function.default_event());
            break;
         case MUXOUT_DRIVE_1:
            mout_o.write(true); men_o.write(true);
            wait(function.default_event());
            break;
         default: /* Function unknown and function 256 */
            mout_o.write(false); men_o.write(false);
            wait(function.default_event());
            break;
      }
   }
}

void mux_out::trace(sc_trace_file *tf) {
   sc_trace(tf, function, function.name());
}
