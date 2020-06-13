/*******************************************************************************
 * doitesp32devkitv1.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of the doitESP32devkitV1 board.
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

#ifndef _DOITESP32DEVKITV1_H
#define _DOITESP32DEVKITV1_H

#include <systemc.h>
#include <Arduino.h>
#include "uart.h"
#include "cchan.h"
#include "pcntbus.h"
#include "gpio_matrix.h"
#include "pcntmod.h"
#include "ledcmod.h"
#include "espintr.h"
#include "adc_types.h"
#include "gn_mixed.h"
#include "ctrlregs.h"
#include "spimod.h"

SC_MODULE(doitesp32devkitv1) {
   /* Pins */
   sc_inout<gn_mixed> d0_a11 {"d0_a11"}; /* BOOT button */
   sc_inout<gn_mixed> d1 {"d1"};
   sc_inout<gn_mixed> d2_a12 {"d2_a12"}; /* LED */
   sc_inout<gn_mixed> d3 {"d3"};
   sc_inout<gn_mixed> d4_a10 {"d4_a10"};
   sc_inout<gn_mixed> d5 {"d5"};
   sc_inout<gn_mixed> d12_a15 {"d12_a15"};
   sc_inout<gn_mixed> d13_a14 {"d13_a14"};
   sc_inout<gn_mixed> d14_a16 {"d14_a16"};
   sc_inout<gn_mixed> d15_a13 {"d15_a13"};
   sc_inout<gn_mixed> d16 {"d16"};
   sc_inout<gn_mixed> d17 {"d17"};
   sc_inout<gn_mixed> d18 {"d18"};
   sc_inout<gn_mixed> d19 {"d19"};
   sc_inout<gn_mixed> d21 {"d21"};
   sc_inout<gn_mixed> d22 {"d22"};
   sc_inout<gn_mixed> d23 {"d23"};
   sc_inout<gn_mixed> d25_a18 {"d25_a18"};
   sc_inout<gn_mixed> d26_a19 {"d26_a19"};
   sc_inout<gn_mixed> d27_a17 {"d27_a17"};
   sc_inout<gn_mixed> d32_a4 {"d32_a4"};
   sc_inout<gn_mixed> d33_a5 {"d33_a5"};
   sc_inout<gn_mixed> d34_a6 {"d34_a6"};
   sc_inout<gn_mixed> d35_a7 {"d35_a7"};
   sc_inout<gn_mixed> d36_a0 {"d36_a0"};
   sc_inout<gn_mixed> d37_a1 {"d37_a1"};
   sc_inout<gn_mixed> d38_a2 {"d38_a2"};
   sc_inout<gn_mixed> d39_a3 {"d39_a3"};

   /* These take the place of GPIO 6-11 */
   sc_in<unsigned int> frx {"frx"};
   sc_out<unsigned int> ftx {"trx"};
   sc_in<unsigned int> wrx {"wrx"};
   sc_out<unsigned int> wtx {"wtx"};
   /* These take place of GPIO21 and 22 */
   sc_in<bool> irx {"irx"};
   sc_out<bool> itx {"itx"};

   /* Submodules */
   gpio_matrix i_gpio_matrix {"i_gpio_matrix"};
   pcntmod i_pcnt{"i_pcnt"};
   ledcmod i_ledc{"i_ledc"};
   spimod i_vspi{"i_vspi"};
   spimod i_hspi{"i_hspi"};
   adc1 i_adc1{"i_adc1"};
   adc2 i_adc2{"i_adc2"};
   uart i_uart0 {"i_uart0", 64, 64};
   uart i_uart1 {"i_uart1", 64, 64};
   uart i_uart2 {"i_uart2", 64, 64};
   cchan i_uflash {"i_uflash", 256*4, 256*4}; /* Actually this is a QSPI */
   uart i_ui2c {"i_ui2c", 32*8, 32*8};   /* to replace with an I2C */
   espintr i_espintr{"i_espintr"};

   /* Not sure what is the real interface, but this one works as it is not
    * important exactly what the net packets look like going to the module.
    * We then use a CCHAN to talk to the wifi. */
   cchan i_uwifi {"i_uwifi", 2048, 2048}; /* The actual buffer varies 1k-8k */

   /* signals */
   sc_signal<bool> uart0rx {"uart0rx"};
   sc_signal<bool> uart0tx {"uart0tx"};
   sc_signal<bool> uart1rx {"uart1rx"};
   sc_signal<bool> uart1tx {"uart1tx"};
   sc_signal<bool> uart2rx {"uart2rx"};
   sc_signal<bool> uart2tx {"uart2tx"};
   sc_signal<pcntbus_t> pcntbus_0 {"pcntbus_0"};
   sc_signal<pcntbus_t> pcntbus_1 {"pcntbus_1"};
   sc_signal<pcntbus_t> pcntbus_2 {"pcntbus_2"};
   sc_signal<pcntbus_t> pcntbus_3 {"pcntbus_3"};
   sc_signal<pcntbus_t> pcntbus_4 {"pcntbus_4"};
   sc_signal<pcntbus_t> pcntbus_5 {"pcntbus_5"};
   sc_signal<pcntbus_t> pcntbus_6 {"pcntbus_6"};
   sc_signal<pcntbus_t> pcntbus_7 {"pcntbus_7"};
   sc_signal<bool> ledc_sig_hs_0{"ledc_sig_hs_0"};
   sc_signal<bool> ledc_sig_hs_1{"ledc_sig_hs_1"};
   sc_signal<bool> ledc_sig_hs_2{"ledc_sig_hs_2"};
   sc_signal<bool> ledc_sig_hs_3{"ledc_sig_hs_3"};
   sc_signal<bool> ledc_sig_hs_4{"ledc_sig_hs_4"};
   sc_signal<bool> ledc_sig_hs_5{"ledc_sig_hs_5"};
   sc_signal<bool> ledc_sig_hs_6{"ledc_sig_hs_6"};
   sc_signal<bool> ledc_sig_hs_7{"ledc_sig_hs_7"};
   sc_signal<bool> ledc_sig_ls_0{"ledc_sig_ls_0"};
   sc_signal<bool> ledc_sig_ls_1{"ledc_sig_ls_1"};
   sc_signal<bool> ledc_sig_ls_2{"ledc_sig_ls_2"};
   sc_signal<bool> ledc_sig_ls_3{"ledc_sig_ls_3"};
   sc_signal<bool> ledc_sig_ls_4{"ledc_sig_ls_4"};
   sc_signal<bool> ledc_sig_ls_5{"ledc_sig_ls_5"};
   sc_signal<bool> ledc_sig_ls_6{"ledc_sig_ls_6"};
   sc_signal<bool> ledc_sig_ls_7{"ledc_sig_ls_7"};
   sc_signal<bool> ledc_intr{"ledc_intr"};
   sc_signal<bool> hspi_d_out{"hspi_d_out"};
   sc_signal<bool> hspi_d_in{"hspi_d_in"};
   sc_signal<bool> hspi_d_oe{"hspi_d_oe"};
   sc_signal<bool> hspi_q_out{"hspi_q_out"};
   sc_signal<bool> hspi_q_in{"hspi_q_in"};
   sc_signal<bool> hspi_q_oe{"hspi_q_oe"};
   sc_signal<bool> hspi_hd_out{"hspi_hd_out"};
   sc_signal<bool> hspi_hd_in{"hspi_hd_in"};
   sc_signal<bool> hspi_hd_oe{"hspi_hd_oe"};
   sc_signal<bool> hspi_wp_out{"hspi_wp_out"};
   sc_signal<bool> hspi_wp_in{"hspi_wp_in"};
   sc_signal<bool> hspi_wp_oe{"hspi_wp_oe"};
   sc_signal<bool> hspi_clk_out{"hspi_clk_out"};
   sc_signal<bool> hspi_clk_in{"hspi_clk_in"};
   sc_signal<bool> hspi_clk_oe{"hspi_clk_oe"};
   sc_signal<bool> hspi_cs0_out{"hspi_cs0_out"};
   sc_signal<bool> hspi_cs0_oe{"hspi_cs0_oe"};
   sc_signal<bool> hspi_cs0_in{"hspi_cs0_in"};
   sc_signal<bool> hspi_cs1_out{"hspi_cs1_out"};
   sc_signal<bool> hspi_cs1_oe{"hspi_cs1_oe"};
   sc_signal<bool> hspi_cs2_out{"hspi_cs2_out"};
   sc_signal<bool> hspi_cs2_oe{"hspi_cs2_oe"};
   sc_signal<bool> vspi_d_out{"vspi_d_out"};
   sc_signal<bool> vspi_d_in{"vspi_d_in"};
   sc_signal<bool> vspi_d_oe{"vspi_d_oe"};
   sc_signal<bool> vspi_q_out{"vspi_q_out"};
   sc_signal<bool> vspi_q_in{"vspi_q_in"};
   sc_signal<bool> vspi_q_oe{"vspi_q_oe"};
   sc_signal<bool> vspi_hd_out{"vspi_hd_out"};
   sc_signal<bool> vspi_hd_in{"vspi_hd_in"};
   sc_signal<bool> vspi_hd_oe{"vspi_hd_oe"};
   sc_signal<bool> vspi_wp_out{"vspi_wp_out"};
   sc_signal<bool> vspi_wp_in{"vspi_wp_in"};
   sc_signal<bool> vspi_wp_oe{"vspi_wp_oe"};
   sc_signal<bool> vspi_clk_out{"vspi_clk_out"};
   sc_signal<bool> vspi_clk_in{"vspi_clk_in"};
   sc_signal<bool> vspi_clk_oe{"vspi_clk_oe"};
   sc_signal<bool> vspi_cs0_out{"vspi_cs0_out"};
   sc_signal<bool> vspi_cs0_oe{"vspi_cs0_oe"};
   sc_signal<bool> vspi_cs0_in{"vspi_cs0_in"};
   sc_signal<bool> vspi_cs1_out{"vspi_cs1_out"};
   sc_signal<bool> vspi_cs1_oe{"vspi_cs1_oe"};
   sc_signal<bool> vspi_cs1_in{"vspi_cs1_in"};
   sc_signal<bool> vspi_cs2_out{"vspi_cs2_out"};
   sc_signal<bool> vspi_cs2_oe{"vspi_cs2_oe"};
   sc_signal<bool> vspi_cs2_in{"vspi_cs2_in"};
   sc_signal<bool> logic_0 {"logic_0", false};

   /* Processes */
   void dut(void);

   /* Other state variables. */
   ctrlregs_t ctrlregs;

   // Constructor
   SC_CTOR(doitesp32devkitv1) {
      /* Pin Hookups */
      i_uart0.rx(uart0rx); i_uart0.tx(uart0tx);
      i_uart1.rx(uart1rx); i_uart1.tx(uart1tx);
      i_uart2.rx(uart2rx); i_uart2.tx(uart2tx);
      i_uflash.rx(frx);
      i_uflash.tx(ftx);
      i_uwifi.rx(wrx);
      i_uwifi.tx(wtx);
      i_ui2c.rx(irx);
      i_ui2c.tx(itx);

      i_gpio_matrix.d0_a11(d0_a11);
      i_gpio_matrix.d1(d1);
      i_gpio_matrix.d2_a12(d2_a12);
      i_gpio_matrix.d3(d3);
      i_gpio_matrix.d4_a10(d4_a10);
      i_gpio_matrix.d5(d5);
      i_gpio_matrix.d12_a15(d12_a15);
      i_gpio_matrix.d13_a14(d13_a14);
      i_gpio_matrix.d14_a16(d14_a16);
      i_gpio_matrix.d15_a13(d15_a13);
      i_gpio_matrix.d16(d16);
      i_gpio_matrix.d17(d17);
      i_gpio_matrix.d18(d18);
      i_gpio_matrix.d19(d19);
      i_gpio_matrix.d21(d21);
      i_gpio_matrix.d22(d22);
      i_gpio_matrix.d23(d23);
      i_gpio_matrix.d25_a18(d25_a18);
      i_gpio_matrix.d26_a19(d26_a19);
      i_gpio_matrix.d27_a17(d27_a17);
      i_gpio_matrix.d32_a4(d32_a4);
      i_gpio_matrix.d33_a5(d33_a5);
      i_gpio_matrix.d34_a6(d34_a6);
      i_gpio_matrix.d35_a7(d35_a7);
      i_gpio_matrix.d36_a0(d36_a0);
      i_gpio_matrix.d37_a1(d37_a1);
      i_gpio_matrix.d38_a2(d38_a2);
      i_gpio_matrix.d39_a3(d39_a3);
      i_gpio_matrix.uart0rx_o(uart0rx); i_gpio_matrix.uart0tx_i(uart0tx);
      i_gpio_matrix.uart1rx_o(uart1rx); i_gpio_matrix.uart1tx_i(uart1tx);
      i_gpio_matrix.uart2rx_o(uart2rx); i_gpio_matrix.uart2tx_i(uart2tx);
      i_gpio_matrix.pcntbus_o(pcntbus_0); i_gpio_matrix.pcntbus_o(pcntbus_1);
      i_gpio_matrix.pcntbus_o(pcntbus_2); i_gpio_matrix.pcntbus_o(pcntbus_3);
      i_gpio_matrix.pcntbus_o(pcntbus_4); i_gpio_matrix.pcntbus_o(pcntbus_5);
      i_gpio_matrix.pcntbus_o(pcntbus_6); i_gpio_matrix.pcntbus_o(pcntbus_7);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_0);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_1);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_2);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_3);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_4);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_5);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_6);
      i_gpio_matrix.ledc_sig_hs_i(ledc_sig_hs_7);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_0);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_1);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_2);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_3);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_4);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_5);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_6);
      i_gpio_matrix.ledc_sig_ls_i(ledc_sig_ls_7);

      /* We connect the ADCs to the channels. */
      i_adc1.channel_0(d36_a0);
      i_adc1.channel_1(d37_a1);
      i_adc1.channel_2(d38_a2);
      i_adc1.channel_3(d39_a3);
      i_adc1.channel_4(d32_a4);
      i_adc1.channel_5(d33_a5);
      i_adc1.channel_6(d34_a6);
      i_adc1.channel_7(d35_a7);
      i_adc2.channel_0(d4_a10);
      i_adc2.channel_1(d0_a11);
      i_adc2.channel_2(d2_a12);
      i_adc2.channel_3(d15_a13);
      i_adc2.channel_4(d13_a14);
      i_adc2.channel_5(d12_a15);
      i_adc2.channel_6(d14_a16);
      i_adc2.channel_7(d27_a17);
      i_adc2.channel_8(d26_a19);
      i_adc2.channel_9(d25_a18);

      /* And the SPIs. */
      i_hspi.d_o(hspi_d_out); i_gpio_matrix.hspi_d_out_i(hspi_d_out);
      i_hspi.d_oen_o(hspi_d_oe); i_gpio_matrix.hspi_d_oen_i(hspi_d_oe);
      i_hspi.d_i(hspi_d_in); i_gpio_matrix.hspi_d_in_o(hspi_d_in);
      i_hspi.q_o(hspi_q_out); i_gpio_matrix.hspi_q_out_i(hspi_q_out);
      i_hspi.q_oen_o(hspi_q_oe); i_gpio_matrix.hspi_q_oen_i(hspi_q_oe);
      i_hspi.q_i(hspi_q_in); i_gpio_matrix.hspi_q_in_o(hspi_q_in);
      i_hspi.clk_o(hspi_clk_out); i_gpio_matrix.hspi_clk_out_i(hspi_clk_out);
      i_hspi.clk_oen_o(hspi_clk_oe); i_gpio_matrix.hspi_clk_oen_i(hspi_clk_oe);
      i_hspi.clk_i(hspi_clk_in); i_gpio_matrix.hspi_clk_in_o(hspi_clk_in);
      i_hspi.hd_o(hspi_hd_out); i_gpio_matrix.hspi_hd_out_i(hspi_hd_out);
      i_hspi.hd_oen_o(hspi_hd_oe); i_gpio_matrix.hspi_hd_oen_i(hspi_hd_oe);
      i_hspi.hd_i(hspi_hd_in); i_gpio_matrix.hspi_hd_in_o(hspi_hd_in);
      i_hspi.wp_o(hspi_wp_out); i_gpio_matrix.hspi_wp_out_i(hspi_wp_out);
      i_hspi.wp_oen_o(hspi_wp_oe); i_gpio_matrix.hspi_wp_oen_i(hspi_wp_oe);
      i_hspi.wp_i(hspi_wp_in); i_gpio_matrix.hspi_wp_in_o(hspi_wp_in);
      i_hspi.cs0_o(hspi_cs0_out); i_gpio_matrix.hspi_cs0_out_i(hspi_cs0_out);
      i_hspi.cs0_oen_o(hspi_cs0_oe); i_gpio_matrix.hspi_cs0_oen_i(hspi_cs0_oe);
      i_hspi.cs0_i(hspi_cs0_in); i_gpio_matrix.hspi_cs0_in_o(hspi_cs0_in);
      /* These don't exist, so we connect them to dummies. */
      i_hspi.cs1_o(hspi_cs1_out); i_hspi.cs1_oen_o(hspi_cs1_oe);
      i_hspi.cs1_i(logic_0);
      i_hspi.cs2_o(hspi_cs2_out); i_hspi.cs2_oen_o(hspi_cs2_oe);
      i_hspi.cs2_i(logic_0);

      i_vspi.d_o(vspi_d_out); i_gpio_matrix.vspi_d_out_i(vspi_d_out);
      i_vspi.d_oen_o(vspi_d_oe); i_gpio_matrix.vspi_d_oen_i(vspi_d_oe);
      i_vspi.d_i(vspi_d_in); i_gpio_matrix.vspi_d_in_o(vspi_d_in);
      i_vspi.q_o(vspi_q_out); i_gpio_matrix.vspi_q_out_i(vspi_q_out);
      i_vspi.q_oen_o(vspi_q_oe); i_gpio_matrix.vspi_q_oen_i(vspi_q_oe);
      i_vspi.q_i(vspi_q_in); i_gpio_matrix.vspi_q_in_o(vspi_q_in);
      i_vspi.clk_o(vspi_clk_out); i_gpio_matrix.vspi_clk_out_i(vspi_clk_out);
      i_vspi.clk_oen_o(vspi_clk_oe); i_gpio_matrix.vspi_clk_oen_i(vspi_clk_oe);
      i_vspi.clk_i(vspi_clk_in); i_gpio_matrix.vspi_clk_in_o(vspi_clk_in);
      i_vspi.hd_o(vspi_hd_out); i_gpio_matrix.vspi_hd_out_i(vspi_hd_out);
      i_vspi.hd_oen_o(vspi_hd_oe); i_gpio_matrix.vspi_hd_oen_i(vspi_hd_oe);
      i_vspi.hd_i(vspi_hd_in); i_gpio_matrix.vspi_hd_in_o(vspi_hd_in);
      i_vspi.wp_o(vspi_wp_out); i_gpio_matrix.vspi_wp_out_i(vspi_wp_out);
      i_vspi.wp_oen_o(vspi_wp_oe); i_gpio_matrix.vspi_wp_oen_i(vspi_wp_oe);
      i_vspi.wp_i(vspi_wp_in); i_gpio_matrix.vspi_wp_in_o(vspi_wp_in);
      i_vspi.cs0_o(vspi_cs0_out); i_gpio_matrix.vspi_cs0_out_i(vspi_cs0_out);
      i_vspi.cs0_oen_o(vspi_cs0_oe); i_gpio_matrix.vspi_cs0_oen_i(vspi_cs0_oe);
      i_vspi.cs0_i(vspi_cs0_in); i_gpio_matrix.vspi_cs0_in_o(vspi_cs0_in);
      i_vspi.cs1_o(vspi_cs1_out); i_gpio_matrix.vspi_cs1_out_i(vspi_cs1_out);
      i_vspi.cs1_oen_o(vspi_cs1_oe); i_gpio_matrix.vspi_cs1_oen_i(vspi_cs1_oe);
      i_vspi.cs1_i(vspi_cs1_in); i_gpio_matrix.vspi_cs1_in_o(vspi_cs1_in);
      i_vspi.cs2_o(vspi_cs2_out); i_gpio_matrix.vspi_cs2_out_i(vspi_cs2_out);
      i_vspi.cs2_oen_o(vspi_cs2_oe); i_gpio_matrix.vspi_cs2_oen_i(vspi_cs2_oe);
      i_vspi.cs2_i(vspi_cs2_in); i_gpio_matrix.vspi_cs2_in_o(vspi_cs2_in);

      /* And we connect the PCNT. */
      i_pcnt.pcntbus_i(pcntbus_0);
      i_pcnt.pcntbus_i(pcntbus_1);
      i_pcnt.pcntbus_i(pcntbus_2);
      i_pcnt.pcntbus_i(pcntbus_3);
      i_pcnt.pcntbus_i(pcntbus_4);
      i_pcnt.pcntbus_i(pcntbus_5);
      i_pcnt.pcntbus_i(pcntbus_6);
      i_pcnt.pcntbus_i(pcntbus_7);

      /* And the ledc. */
      i_ledc.sig_out_hs_o(ledc_sig_hs_0); i_ledc.sig_out_hs_o(ledc_sig_hs_1);
      i_ledc.sig_out_hs_o(ledc_sig_hs_2); i_ledc.sig_out_hs_o(ledc_sig_hs_3);
      i_ledc.sig_out_hs_o(ledc_sig_hs_4); i_ledc.sig_out_hs_o(ledc_sig_hs_5);
      i_ledc.sig_out_hs_o(ledc_sig_hs_6); i_ledc.sig_out_hs_o(ledc_sig_hs_7);
      i_ledc.sig_out_ls_o(ledc_sig_ls_0); i_ledc.sig_out_ls_o(ledc_sig_ls_1);
      i_ledc.sig_out_ls_o(ledc_sig_ls_2); i_ledc.sig_out_ls_o(ledc_sig_ls_3);
      i_ledc.sig_out_ls_o(ledc_sig_ls_4); i_ledc.sig_out_ls_o(ledc_sig_ls_5);
      i_ledc.sig_out_ls_o(ledc_sig_ls_6); i_ledc.sig_out_ls_o(ledc_sig_ls_7);
      i_ledc.intr_o(ledc_intr);

      i_espintr.ledc_intr_i(ledc_intr);

      SC_THREAD(dut);
   }

   void pininit();
   void start_of_simulation();
   void trace(sc_trace_file *tf);
};

#endif
