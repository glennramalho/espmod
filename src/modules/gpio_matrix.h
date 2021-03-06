/*******************************************************************************
 * gpio_matrix.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Models the ESP32 GPIO Matrix
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

#ifndef _GPIO_MATRIX_H
#define _GPIO_MATRIX_H

#include <systemc.h>
#include "netcon.h"
#include "mux_in.h"
#include "mux_out.h"
#include "pcntbus.h"
#include "mux_pcnt.h"
#include "gn_mixed.h"
#include "io_mux.h"

struct gpio_matrix;
#include "soc/gpio_struct.h"

#define GPIOMATRIX_CNT 40
#define GPIOMATRIX_DIRECT 40
#define GPIOMATRIX_LOGIC0 41
#define GPIOMATRIX_LOGIC1 42
#define GPIOMATRIX_ALL 43

/* To make the functions easier to read, we make a shortcut to connecting
 * a function.
 */
#define CONNECTFUNC(iomuxdg,fins,fens,fouts) { \
   iomuxdg.fin(fins); \
   iomuxdg.fen(fens); \
   iomuxdg.fout(fouts); \
}
/* The out muxes too must have the exact same inputs in and each is connected
 * to one GPIO.
 */
#define CONNECTOUTMUX(dg) { \
   i_mux_out##dg.mout_o(mout_s[dg]); \
   i_mux_out##dg.men_o(men_s[dg]); \
   i_mux_out##dg.uart0tx_i(uart0tx_i); \
   i_mux_out##dg.uart1tx_i(uart1tx_i); \
   i_mux_out##dg.uart2tx_i(uart2tx_i); \
   i_mux_out##dg.ledc_sig_hs_i(ledc_sig_hs_i); \
   i_mux_out##dg.ledc_sig_ls_i(ledc_sig_ls_i); \
   i_mux_out##dg.hspi_d_oen_i(hspi_d_oen_i); \
   i_mux_out##dg.hspi_d_out_i(hspi_d_out_i); \
   i_mux_out##dg.hspi_q_oen_i(hspi_q_oen_i); \
   i_mux_out##dg.hspi_q_out_i(hspi_q_out_i); \
   i_mux_out##dg.hspi_cs0_oen_i(hspi_cs0_oen_i); \
   i_mux_out##dg.hspi_cs0_out_i(hspi_cs0_out_i); \
   i_mux_out##dg.hspi_clk_oen_i(hspi_clk_oen_i); \
   i_mux_out##dg.hspi_clk_out_i(hspi_clk_out_i); \
   i_mux_out##dg.hspi_wp_oen_i(hspi_wp_oen_i); \
   i_mux_out##dg.hspi_wp_out_i(hspi_wp_out_i); \
   i_mux_out##dg.hspi_hd_oen_i(hspi_hd_oen_i); \
   i_mux_out##dg.hspi_hd_out_i(hspi_hd_out_i); \
   i_mux_out##dg.vspi_d_oen_i(vspi_d_oen_i); \
   i_mux_out##dg.vspi_d_out_i(vspi_d_out_i); \
   i_mux_out##dg.vspi_q_oen_i(vspi_q_oen_i); \
   i_mux_out##dg.vspi_q_out_i(vspi_q_out_i); \
   i_mux_out##dg.vspi_cs0_oen_i(vspi_cs0_oen_i); \
   i_mux_out##dg.vspi_cs0_out_i(vspi_cs0_out_i); \
   i_mux_out##dg.vspi_cs1_oen_i(vspi_cs1_oen_i); \
   i_mux_out##dg.vspi_cs1_out_i(vspi_cs1_out_i); \
   i_mux_out##dg.vspi_cs2_oen_i(vspi_cs2_oen_i); \
   i_mux_out##dg.vspi_cs2_out_i(vspi_cs2_out_i); \
   i_mux_out##dg.vspi_clk_oen_i(vspi_clk_oen_i); \
   i_mux_out##dg.vspi_clk_out_i(vspi_clk_out_i); \
   i_mux_out##dg.vspi_wp_oen_i(vspi_wp_oen_i); \
   i_mux_out##dg.vspi_wp_out_i(vspi_wp_out_i); \
   i_mux_out##dg.vspi_hd_oen_i(vspi_hd_oen_i); \
   i_mux_out##dg.vspi_hd_out_i(vspi_hd_out_i); \
   i_mux_out##dg.sda0_en_i(sda0_en_i); \
   i_mux_out##dg.scl0_en_i(scl0_en_i); \
   i_mux_out##dg.sda1_en_i(sda1_en_i); \
   i_mux_out##dg.scl1_en_i(scl1_en_i); \
}

#define CONNECTINMUX(block, outsig, directsig) \
   block.out_o(outsig); \
   for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) block.min_i(min_s[g]); \
   block.min_i(directsig); \
   block.min_i(logic_0); \
   block.min_i(logic_1);

SC_MODULE(gpio_matrix) {
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

   /* PCNT and LEDC */
   sc_port<sc_signal_out_if<pcntbus_t>,8> pcntbus_o;
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_hs_i;
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_ls_i;

   /* UARTS */
   sc_out<bool> uart0rx_o {"uart0rx_o"};
   sc_in<bool> uart0tx_i {"uart0tx_i"};
   sc_out<bool> uart1rx_o {"uart1rx_o"};
   sc_in<bool> uart1tx_i {"uart1tx_i"};
   sc_out<bool> uart2rx_o {"uart2rx_o"};
   sc_in<bool> uart2tx_i {"uart2tx_i"};

   /* SPI */
   sc_in<bool> hspi_d_oen_i {"hspi_d_oen_i"};
   sc_in<bool> hspi_d_out_i {"hspi_d_out_i"};
   sc_out<bool> hspi_d_in_o {"hspi_d_in_o"};
   sc_in<bool> hspi_q_oen_i {"hspi_q_oen_i"};
   sc_in<bool> hspi_q_out_i {"hspi_q_out_i"};
   sc_out<bool> hspi_q_in_o {"hspi_q_in_o"};
   sc_in<bool> hspi_cs0_oen_i {"hspi_cs0_oen_i"};
   sc_in<bool> hspi_cs0_out_i {"hspi_cs0_out_i"};
   sc_out<bool> hspi_cs0_in_o {"hspi_cs0_in_o"};
   sc_in<bool> hspi_clk_oen_i {"hspi_clk_oen_i"};
   sc_in<bool> hspi_clk_out_i {"hspi_clk_out_i"};
   sc_out<bool> hspi_clk_in_o {"hspi_clk_in_o"};
   sc_in<bool> hspi_wp_oen_i {"hspi_wp_oen_i"};
   sc_in<bool> hspi_wp_out_i {"hspi_wp_out_i"};
   sc_out<bool> hspi_wp_in_o {"hspi_wp_in_o"};
   sc_in<bool> hspi_hd_oen_i {"hspi_hd_oen_i"};
   sc_in<bool> hspi_hd_out_i {"hspi_hd_out_i"};
   sc_out<bool> hspi_hd_in_o {"hspi_hd_in_o"};
   sc_in<bool> vspi_d_oen_i {"vspi_d_oen_i"};
   sc_in<bool> vspi_d_out_i {"vspi_d_out_i"};
   sc_out<bool> vspi_d_in_o {"vspi_d_in_o"};
   sc_in<bool> vspi_q_oen_i {"vspi_q_oen_i"};
   sc_in<bool> vspi_q_out_i {"vspi_q_out_i"};
   sc_out<bool> vspi_q_in_o {"vspi_q_in_o"};
   sc_in<bool> vspi_cs0_oen_i {"vspi_cs0_oen_i"};
   sc_in<bool> vspi_cs0_out_i {"vspi_cs0_out_i"};
   sc_out<bool> vspi_cs0_in_o {"vspi_cs0_in_o"};
   sc_in<bool> vspi_cs1_oen_i {"vspi_cs1_oen_i"};
   sc_in<bool> vspi_cs1_out_i {"vspi_cs1_out_i"};
   sc_out<bool> vspi_cs1_in_o {"vspi_cs1_in_o"};
   sc_in<bool> vspi_cs2_oen_i {"vspi_cs2_oen_i"};
   sc_in<bool> vspi_cs2_out_i {"vspi_cs2_out_i"};
   sc_out<bool> vspi_cs2_in_o {"vspi_cs2_in_o"};
   sc_in<bool> vspi_clk_oen_i {"vspi_clk_oen_i"};
   sc_in<bool> vspi_clk_out_i {"vspi_clk_out_i"};
   sc_out<bool> vspi_clk_in_o {"vspi_clk_in_o"};
   sc_in<bool> vspi_wp_oen_i {"vspi_wp_oen_i"};
   sc_in<bool> vspi_wp_out_i {"vspi_wp_out_i"};
   sc_out<bool> vspi_wp_in_o {"vspi_wp_in_o"};
   sc_in<bool> vspi_hd_oen_i {"vspi_hd_oen_i"};
   sc_in<bool> vspi_hd_out_i {"vspi_hd_out_i"};
   sc_out<bool> vspi_hd_in_o {"vspi_hd_in_o"};

   /* I2C */
   sc_in<bool> sda0_en_i {"sda0_en_i"};
   sc_in<bool> sda1_en_i {"sda1_en_i"};
   sc_in<bool> scl0_en_i {"scl0_en_i"};
   sc_in<bool> scl1_en_i {"scl1_en_i"};
   sc_out<bool> scl0_o {"scl0_o"};
   sc_out<bool> sda0_o {"sda0_o"};
   sc_out<bool> scl1_o {"scl1_o"};
   sc_out<bool> sda1_o {"sda1_o"};

   /* Submodules */
   mux_pcnt i_mux_pcnt {"i_mux_pcnt"};
   mux_in i_mux_hspi_d {"i_mux_hspi_d", GPIOMATRIX_LOGIC1};
   mux_in i_mux_hspi_q {"i_mux_hspi_q", GPIOMATRIX_LOGIC1};
   mux_in i_mux_hspi_hd {"i_mux_hspi_hd", GPIOMATRIX_LOGIC1};
   mux_in i_mux_hspi_wp {"i_mux_hspi_wp", GPIOMATRIX_LOGIC1};
   mux_in i_mux_hspi_clk {"i_mux_hspi_clk", GPIOMATRIX_LOGIC1};
   mux_in i_mux_hspi_cs0 {"i_mux_hspi_cs0", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_d {"i_mux_vspi_d", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_q {"i_mux_vspi_q", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_hd {"i_mux_vspi_hd", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_wp {"i_mux_vspi_wp", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_clk {"i_mux_vspi_clk", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_cs0 {"i_mux_vspi_cs0", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_cs1 {"i_mux_vspi_cs1", GPIOMATRIX_LOGIC1};
   mux_in i_mux_vspi_cs2 {"i_mux_vspi_cs2", GPIOMATRIX_LOGIC1};
   mux_in i_mux_i2c_sda0 {"i_mux_i2c_sda0", GPIOMATRIX_LOGIC1};
   mux_in i_mux_i2c_sda1 {"i_mux_i2c_sda1", GPIOMATRIX_LOGIC1};
   mux_in i_mux_i2c_scl0 {"i_mux_i2c_scl0", GPIOMATRIX_LOGIC1};
   mux_in i_mux_i2c_scl1 {"i_mux_i2c_scl1", GPIOMATRIX_LOGIC1};
   mux_in i_mux_uart0 {"i_mux_uart0", GPIOMATRIX_LOGIC1};
   mux_in i_mux_uart1 {"i_mux_uart1", GPIOMATRIX_LOGIC1};
   mux_in i_mux_uart2 {"i_mux_uart2", GPIOMATRIX_LOGIC1};
   mux_out i_mux_out0  {"i_mux_out0"};
   mux_out i_mux_out1  {"i_mux_out1"};
   mux_out i_mux_out2  {"i_mux_out2"};
   mux_out i_mux_out3  {"i_mux_out3"};
   mux_out i_mux_out4  {"i_mux_out4"};
   mux_out i_mux_out5  {"i_mux_out5"};
   mux_out i_mux_out12 {"i_mux_out12"};
   mux_out i_mux_out13 {"i_mux_out13"};
   mux_out i_mux_out14 {"i_mux_out14"};
   mux_out i_mux_out15 {"i_mux_out15"};
   mux_out i_mux_out16 {"i_mux_out16"};
   mux_out i_mux_out17 {"i_mux_out17"};
   mux_out i_mux_out18 {"i_mux_out18"};
   mux_out i_mux_out19 {"i_mux_out19"};
   mux_out i_mux_out21 {"i_mux_out21"};
   mux_out i_mux_out22 {"i_mux_out22"};
   mux_out i_mux_out23 {"i_mux_out23"};
   mux_out i_mux_out25 {"i_mux_out25"};
   mux_out i_mux_out26 {"i_mux_out26"};
   mux_out i_mux_out27 {"i_mux_out27"};
   mux_out i_mux_out32 {"i_mux_out32"};
   mux_out i_mux_out33 {"i_mux_out33"};
   mux_out i_mux_out34 {"i_mux_out34"};
   mux_out i_mux_out35 {"i_mux_out35"};
   mux_out i_mux_out36 {"i_mux_out36"};
   mux_out i_mux_out37 {"i_mux_out37"};
   mux_out i_mux_out38 {"i_mux_out38"};
   mux_out i_mux_out39 {"i_mux_out39"};
   mux_out *muxptr[40];

   /* GPIOs */
   io_mux i_mux_d0 {"i_mux_d0",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU, false, 3};
   io_mux i_mux_d1 {"i_mux_d1",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU, false, 3};
   io_mux i_mux_d2 {"i_mux_d2",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD, false, 3};
   io_mux i_mux_d3 {"i_mux_d3",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU, false, 3};
   io_mux i_mux_d4 {"i_mux_d4",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD, false, 3};
   io_mux i_mux_d5 {"i_mux_d5",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, false, 3};
   io_mux i_mux_d12 {"i_mux_d12",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD, false, 3};
   io_mux i_mux_d13{"i_mux_d13",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU, false, 3}; /* Not quite, but ok for now */
   io_mux i_mux_d14 {"i_mux_d14",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD, false, 3}; /* Not quite, but ok for now */
   io_mux i_mux_d15 {"i_mux_d15",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU, false, 3};
   io_mux i_mux_d16 {"i_mux_d16",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, false, 3};
   io_mux i_mux_d17 {"i_mux_d17",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, false, 3};
   io_mux i_mux_d18 {"i_mux_d18",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   io_mux i_mux_d19 {"i_mux_d19",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   /* There is no GPIO D20 */
   io_mux i_mux_d21 {"i_mux_d21",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   io_mux i_mux_d22 {"i_mux_d22",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   io_mux i_mux_d23 {"i_mux_d23",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   /* There is no GPIO D24 */
   io_mux i_mux_d25 {"i_mux_d25",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, false, 3};
   io_mux i_mux_d26 {"i_mux_d26",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, false, 3};
   io_mux i_mux_d27 {"i_mux_d27",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT, false, 3};
   /* There is no GPIO d28 */
   /* There is no GPIO d29 */
   /* There is no GPIO d30 */
   /* There is no GPIO d31 */
   io_mux i_mux_d32 {"i_mux_d32",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, true, 3};
   io_mux i_mux_d33 {"i_mux_d33",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE, true, 3};
   io_mux i_mux_d34 {"i_mux_d34",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};
   io_mux i_mux_d35 {"i_mux_d35",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};
   io_mux i_mux_d36 {"i_mux_d36",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};
   io_mux i_mux_d37 {"i_mux_d37",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};
   io_mux i_mux_d38 {"i_mux_d38",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};
   io_mux i_mux_d39 {"i_mux_d39",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true, 3};

   /* Signals */
   sc_signal<bool> d_hspi_d_in_s {"d_hspi_d_in_s"};
   sc_signal<bool> d_hspi_q_in_s {"d_hspi_q_in_s"};
   sc_signal<bool> d_hspi_clk_in_s {"d_hspi_clk_in_s"};
   sc_signal<bool> d_hspi_hd_in_s {"d_hspi_hd_in_s"};
   sc_signal<bool> d_hspi_wp_in_s {"d_hspi_wp_in_s"};
   sc_signal<bool> d_hspi_cs0_in_s {"d_hspi_cs0_in_s"};
   sc_signal<bool> d_vspi_d_in_s {"d_vspi_d_in_s"};
   sc_signal<bool> d_vspi_q_in_s {"d_vspi_q_in_s"};
   sc_signal<bool> d_vspi_clk_in_s {"d_vspi_clk_in_s"};
   sc_signal<bool> d_vspi_hd_in_s {"d_vspi_hd_in_s"};
   sc_signal<bool> d_vspi_wp_in_s {"d_vspi_wp_in_s"};
   sc_signal<bool> d_vspi_cs0_in_s {"d_vspi_cs0_in_s"};
   sc_signal<bool> d_vspi_cs1_in_s {"d_vspi_cs1_in_s"};
   sc_signal<bool> d_vspi_cs2_in_s {"d_vspi_cs2_in_s"};
   sc_signal<bool> d_u0rx_s {"d_u0rx_s"};
   sc_signal<bool> d_u1rx_s {"d_u1rx_s"};
   sc_signal<bool> d_u2rx_s {"d_u2rx_s"};

   /* matrix interconnect function signals */
   sc_signal<bool> men_s[GPIOMATRIX_CNT]; /* Output enable */
   sc_signal<bool> mout_s[GPIOMATRIX_CNT]; /* Outgoing signal */
   sc_signal<bool> min_s[GPIOMATRIX_CNT];/* Incomming signal */
   sc_signal<uint64_t> gpio_out {"gpio_out"};

   /* Unconnected signals */
   sc_signal<bool> l0_f1 {"l0_f1", false};
   sc_signal<bool> l0_f2 {"l0_f2", false};
   sc_signal<bool> l0_f3 {"l0_f3", false};
   sc_signal<bool> l0_f4 {"l0_f4", false};
   sc_signal<bool> l0_f5 {"l0_f5", false};
   sc_signal<bool> l0_f6 {"l0_f6", false};
   sc_signal<bool> l0_f7 {"l0_f7", false};
   sc_signal<bool> l1_f1 {"l1_f1", true};
   sc_signal<bool> l1_f2 {"l1_f2", true};
   sc_signal<bool> l1_f3 {"l1_f3", true};
   sc_signal<bool> l1_f4 {"l1_f4", true};
   sc_signal<bool> l1_f5 {"l1_f5", true};
   sc_signal<bool> l1_f6 {"l1_f6", true};
   sc_signal<bool> l1_f7 {"l1_f7", true};
   sc_signal<bool> logic_0 {"logic_0", false};
   sc_signal<bool> logic_1 {"logic_1", false};

   /* Functions */
   void setbits(uint32_t highbits, uint32_t lowbits);
   void setoebits(uint32_t highbits, uint32_t lowbits);
   void applycsbits();
   void applyoecsbits();

   /* Update calls */
   /* Call if the struct was modified. */
   void update();
   /* Checks only OUT bits. Call only if you are sure only the OUT bits
    * were changed.  */
   void updategpioreg();
   /* Checks only OE bits. Call only if you are sure only the OE bits
    * were changed.  */
   void updategpiooe();
   mux_out *getmux(int pin);
   void initptr();

   /* Threads */
   sc_event updategpioreg_ev;
   sc_event updategpiooe_ev;
   sc_event update_ev;
   void updateth(void);

   // Constructor
   SC_CTOR(gpio_matrix) {
      int g;
      /* Pin Hookups */
      /* GPIO 0 */
      i_mux_d0.pin(d0_a11);
      CONNECTFUNC(i_mux_d0,  l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d0,  l0_f2, l1_f2, sig_open);        /* F2 CLK_OUT1 */
      CONNECTFUNC(i_mux_d0,mout_s[0],men_s[0],min_s[0]);     /* F3 GPIO */
      CONNECTFUNC(i_mux_d0,  l0_f4, l1_f4, sig_open);        /* F4 not used */
      CONNECTFUNC(i_mux_d0,  l0_f5, l1_f5, sig_open);        /* F5 not used */
      CONNECTFUNC(i_mux_d0,  l0_f6, l1_f6, sig_open);        /*F6 EMAC_TX_CLR*/
      /* Commented out for now as the I2C is still using a CCHAN. */
      CONNECTFUNC(i_mux_d0,  l0_f7, l1_f7, sig_open); /* RTCF2 -- I2C_SDA */
      //CONNECTFUNC(i_mux_d0, l1_f7, i2c_sda_tx, i2c_sda_rx); /* F7 */
      CONNECTOUTMUX(0);
      /* GPIO 1 */
      i_mux_d1.pin(d1);
      CONNECTFUNC(i_mux_d1, uart0tx_i, l1_f1, sig_open);     /* F1 T0RX */
      CONNECTFUNC(i_mux_d1, l0_f2,     l1_f2, sig_open);     /* F2 CLK_OUT2 */
      CONNECTFUNC(i_mux_d1,mout_s[1],men_s[1],min_s[1]);     /* F3 GPIO */
      CONNECTOUTMUX(1);
      /* GPIO 2 */
      i_mux_d2.pin(d2_a12);
      CONNECTFUNC(i_mux_d2, l0_f1,     l1_f1, sig_open);     /* F1 GPIO */
      CONNECTFUNC(i_mux_d2, hspi_wp_out_i,
                               hspi_wp_oen_i,d_hspi_wp_in_s);/* F2 HSPI-WP*/
      CONNECTFUNC(i_mux_d2,mout_s[2],men_s[2],min_s[2]);     /* F3 GPIO */
      /* F1: GPIO -- built-in */
      /* F2: HSPIWP -- not yet supported */
      /* F3: GPIO -- built-in */
      /* F4: HS2_DATA0 -- not yet supported */
      /* F5: SD_DATA0 -- not yet supported */
      CONNECTOUTMUX(2);
      /* GPIO 3 */
      i_mux_d3.pin(d3);
      CONNECTFUNC(i_mux_d3, l0_f1, l0_f1, d_u0rx_s);         /* F1 U0RX */
      CONNECTFUNC(i_mux_d3, l0_f2, l1_f2, sig_open);         /* F2 CLK_OUT2*/
      CONNECTFUNC(i_mux_d3, mout_s[3], men_s[3], min_s[3]);  /* F3 GPIO */
      /* F2: CLK_OUT2 -- not supported. */
      /* F3: GPIO -- built-in to GPIO. */
      CONNECTOUTMUX(3);
      /* GPIO 4 */
      i_mux_d4.pin(d4_a10);
      CONNECTFUNC(i_mux_d4,  l0_f1, l1_f1, sig_open);        /* F1 skipped */
      CONNECTFUNC(i_mux_d4, hspi_hd_out_i,
                               hspi_hd_oen_i,d_hspi_hd_in_s);/* F2 HSPI-HD*/
      CONNECTFUNC(i_mux_d4,mout_s[4],men_s[4],min_s[4]);     /* F3 GPIO */
      CONNECTFUNC(i_mux_d4,  l0_f4, l1_f4, sig_open);        /* F4 HS2_DATA1 */
      CONNECTFUNC(i_mux_d4,  l0_f5, l1_f5, sig_open);        /* F5 SD_DATA1 */
      CONNECTFUNC(i_mux_d4,  l0_f6, l1_f6, sig_open);        /* F6 EMAC_TX_ER*/
      /* Commented out for now as the I2C is still using a CCHAN. */
      /* RTC FUNC 1: RTC_GPIO10 */
      CONNECTFUNC(i_mux_d4,  l0_f7, l1_f7, sig_open); /* RTC FUNC 2: I2CSCL */
      //CONNECTFUNC(i_mux_d4, l0_f7, i2c_sda_tx, i2c_sda_rx); /* F7 */
      CONNECTOUTMUX(4);
      i_mux_d5.pin(d5);
      CONNECTFUNC(i_mux_d5,  l0_f1, l1_f1, sig_open);        /* F1 skipped */
      CONNECTFUNC(i_mux_d5,  vspi_cs0_out_i,
                             vspi_cs0_oen_i,d_vspi_cs0_in_s);/* F2 VSPICS0 */
      CONNECTFUNC(i_mux_d5,mout_s[5],men_s[5],min_s[5]);     /* F3 GPIO */
      /* F4: HS1_DATA6 -- not supported. */
      CONNECTOUTMUX(5);
      /* GPIO  6 -- not yet supported, implemented via the flash channel. */
      /* GPIO  7 -- not yet supported, implemented via the flash channel. */
      /* GPIO  8 -- not yet supported, implemented via the flash channel. */
      /* GPIO  9 -- not yet supported, implemented via the flash channel. */
      /* GPIO 10 -- not yet supported, implemented via the flash channel. */
      /* GPIO 11 -- not yet supported, implemented via the flash channel. */
      /* GPIO 12 */
      i_mux_d12.pin(d12_a15);
      CONNECTFUNC(i_mux_d12,  l0_f1, l1_f1, sig_open);       /* F1 MTDO */
      CONNECTFUNC(i_mux_d12, hspi_q_out_i,
                               hspi_q_oen_i, d_hspi_q_in_s); /* F2 HSPIQ */
      CONNECTFUNC(i_mux_d12,mout_s[12],men_s[12],min_s[12]); /* F3 GPIO */
      /* F4: HS2_DATA2 -- not supported. */
      /* F5: SD_DATA2 -- not supported. */
      /* F6: EMAC_TXD3 */
      CONNECTOUTMUX(12);
      i_mux_d13.pin(d13_a14);
      CONNECTFUNC(i_mux_d13,  l0_f1, l1_f1, sig_open);       /* F1 MTCK */
      CONNECTFUNC(i_mux_d13, hspi_d_out_i,
                               hspi_d_oen_i, d_hspi_d_in_s); /* F2 HSPID */
      CONNECTFUNC(i_mux_d13,mout_s[13],men_s[13],min_s[13]); /* F3 GPIO */
      /* F4: HS2_DATA3 -- not supported. */
      /* F5: SD_DATA3 -- not supported. */
      /* F6: EMAC_RX_ER */
      CONNECTOUTMUX(13);
      i_mux_d14.pin(d14_a16);
      CONNECTFUNC(i_mux_d14, l0_f1, l1_f1, sig_open);        /* F1 MTMS */
      CONNECTFUNC(i_mux_d14, hspi_clk_out_i,
                            hspi_clk_oen_i, d_hspi_clk_in_s);/* F2 HSPICLK */
      CONNECTFUNC(i_mux_d14,mout_s[14],men_s[14],min_s[14]); /* F3 GPIO */
      /* F4: HS2_CLK -- not supported. */
      /* F5: SD_CLK -- not supported. */
      /* F6: EMAC_TXD2 */
      CONNECTOUTMUX(14);
      i_mux_d15(d15_a13);
      CONNECTFUNC(i_mux_d15, l0_f1, l1_f1, sig_open);        /* F1 MTDO */
      CONNECTFUNC(i_mux_d15, hspi_cs0_out_i,
                            hspi_cs0_oen_i, d_hspi_cs0_in_s);/* F2 HSPICS0 */
      CONNECTFUNC(i_mux_d15,mout_s[15],men_s[15],min_s[15]); /* F3 GPIO */
      /* F4: HS2_CMD -- not supported. */
      /* F5: SD_CMD -- not supported. */
      /* F6: EMAC_RXD3 */
      CONNECTOUTMUX(15);
      i_mux_d16.pin(d16);
      CONNECTFUNC(i_mux_d16, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d16, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d16,mout_s[16],men_s[16],min_s[16]); /* F3 GPIO */
      CONNECTFUNC(i_mux_d16, l0_f4, l1_f4, sig_open);        /* F4 HS1_DATA4 */
      CONNECTFUNC(i_mux_d16, l0_f5, l1_f5, d_u2rx_s);        /* F5 U2RXD */
      /* F6: EMAC_CLK_OUT */
      CONNECTOUTMUX(16);
      /* GPIO 17 */
      i_mux_d17.pin(d17);
      CONNECTFUNC(i_mux_d17, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d17, l0_f2, l1_f2, sig_open);        /* F2 not used*/
      CONNECTFUNC(i_mux_d17,mout_s[17],men_s[17],min_s[17]); /* F3 GPIO */
      CONNECTFUNC(i_mux_d17, l0_f4, l1_f4, sig_open);        /* F4 HS1_DATA5 */
      CONNECTFUNC(i_mux_d17, uart2tx_i,l1_f5,sig_open);      /* F5 U2TXD */
      /* F6: EMAC_CLK_OUT_180 */
      CONNECTOUTMUX(17);
      i_mux_d18.pin(d18);
      CONNECTFUNC(i_mux_d18, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d18, vspi_clk_out_i,
                            vspi_clk_oen_i, d_vspi_clk_in_s);/* F2 VSPICLK */
      CONNECTFUNC(i_mux_d18,mout_s[18],men_s[18],min_s[18]); /* F3 GPIO */
      /* F4: HS1_DATA7 -- not supported. */
      CONNECTOUTMUX(18);
      i_mux_d19.pin(d19);
      CONNECTFUNC(i_mux_d19, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d19, vspi_q_out_i,
                                vspi_q_oen_i, d_vspi_q_in_s);/* F2 VSPIQ */
      CONNECTFUNC(i_mux_d19,mout_s[19],men_s[19],min_s[19]); /* F3 GPIO */
      /* F4: U0CTS -- not supported. */
      /* F5: -- not used. */
      /* F6: EMAC TXDO */
      CONNECTOUTMUX(19);
      i_mux_d21.pin(d21);
      CONNECTFUNC(i_mux_d21, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d21, vspi_hd_out_i,
                              vspi_hd_oen_i, d_vspi_hd_in_s);/* F2 VSPIHD */
      CONNECTFUNC(i_mux_d21,mout_s[21],men_s[21],min_s[21]); /* F3 GPIO */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC TX EN */
      CONNECTOUTMUX(21);
      i_mux_d22.pin(d22);
      CONNECTFUNC(i_mux_d22, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d22, vspi_wp_out_i,
                              vspi_wp_oen_i, d_vspi_wp_in_s);/* F2 VSPIWP */
      CONNECTFUNC(i_mux_d22,mout_s[22],men_s[22],min_s[22]); /* F3 GPIO */
      /* F4: U0RTS -- not supported. */
      /* F5: -- not used. */
      /* F6: EMAC TXD1 */
      CONNECTOUTMUX(22);
      i_mux_d23.pin(d23);
      CONNECTFUNC(i_mux_d23, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d23, vspi_d_out_i,
                                vspi_d_oen_i, d_vspi_d_in_s);/* F2 VSPID */
      CONNECTFUNC(i_mux_d23,mout_s[23],men_s[23],min_s[23]); /* F3 GPIO */
      /* F4: HS1_STROBE -- not supported. */
      /* F5: -- not used. */
      /* F6: -- not used. */
      CONNECTOUTMUX(23);
      i_mux_d25.pin(d25_a18);
      CONNECTFUNC(i_mux_d25, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d25, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d25,mout_s[25],men_s[25],min_s[25]); /* F3 GPIO */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RXD0 */
      CONNECTOUTMUX(25);
      i_mux_d26.pin(d26_a19);
      CONNECTFUNC(i_mux_d26, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d26, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d26,mout_s[26],men_s[26],min_s[26]); /* F3 GPIO */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RXD1 */
      CONNECTOUTMUX(26);
      i_mux_d27.pin(d27_a17);
      CONNECTFUNC(i_mux_d27, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d27, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d27,mout_s[27],men_s[27],min_s[27]); /* F3 GPIO */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RX_DV */
      CONNECTOUTMUX(27);
      /* These below, asside from the ADC do not have any supported additional
       * functions.
       */
      i_mux_d32.pin(d32_a4);
      CONNECTFUNC(i_mux_d32, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d32, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d32,mout_s[32],men_s[32],min_s[32]); /* F3 GPIO */
      CONNECTOUTMUX(32);
      i_mux_d33.pin(d33_a5);
      CONNECTFUNC(i_mux_d33, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d33, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d33,mout_s[33],men_s[33],min_s[33]); /* F3 GPIO */
      CONNECTOUTMUX(33);
      i_mux_d34.pin(d34_a6);
      CONNECTFUNC(i_mux_d34, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d34, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d34,mout_s[34],men_s[34],min_s[34]); /* F3 GPIO */
      CONNECTOUTMUX(34);
      i_mux_d35.pin(d35_a7);
      CONNECTFUNC(i_mux_d35, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d35, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d35,mout_s[35],men_s[35],min_s[35]); /* F3 GPIO */
      CONNECTOUTMUX(35);
      i_mux_d36.pin(d36_a0);
      CONNECTFUNC(i_mux_d36, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d36, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d36,mout_s[36],men_s[36],min_s[36]); /* F3 GPIO */
      CONNECTOUTMUX(36);
      i_mux_d37.pin(d37_a1);
      CONNECTFUNC(i_mux_d37, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d37, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d37,mout_s[37],men_s[37],min_s[37]); /* F3 GPIO */
      CONNECTOUTMUX(37);
      i_mux_d38.pin(d38_a2);
      CONNECTFUNC(i_mux_d38, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d38, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d38,mout_s[38],men_s[38],min_s[38]); /* F3 GPIO */
      CONNECTOUTMUX(38);
      i_mux_d39.pin(d39_a3);
      CONNECTFUNC(i_mux_d39, l0_f1, l1_f1, sig_open);        /* F1 GPIO */
      CONNECTFUNC(i_mux_d39, l0_f2, l1_f2, sig_open);        /* F2 not used */
      CONNECTFUNC(i_mux_d39,mout_s[39],men_s[39],min_s[39]); /* F3 GPIO */
      CONNECTOUTMUX(39);

      /* PCNT Mux */
      i_mux_pcnt.pcntbus_o(pcntbus_o);
      for(g = 0; g < GPIOMATRIX_CNT; g = g + 1) i_mux_pcnt.mout_i(min_s[g]);
      i_mux_pcnt.mout_i(l0_f1); /* Direct signal */
      i_mux_pcnt.mout_i(logic_0);
      i_mux_pcnt.mout_i(logic_1);

      /* HSPI Muxes */
      CONNECTINMUX(i_mux_hspi_d, hspi_d_in_o, d_hspi_d_in_s);
      CONNECTINMUX(i_mux_hspi_q, hspi_q_in_o, d_hspi_q_in_s);
      CONNECTINMUX(i_mux_hspi_clk, hspi_clk_in_o, d_hspi_clk_in_s);
      CONNECTINMUX(i_mux_hspi_hd, hspi_hd_in_o, d_hspi_hd_in_s);
      CONNECTINMUX(i_mux_hspi_wp, hspi_wp_in_o, d_hspi_wp_in_s);
      CONNECTINMUX(i_mux_hspi_cs0, hspi_cs0_in_o, d_hspi_cs0_in_s);
      CONNECTINMUX(i_mux_vspi_d, vspi_d_in_o, d_vspi_d_in_s);
      CONNECTINMUX(i_mux_vspi_q, vspi_q_in_o, d_vspi_q_in_s);
      CONNECTINMUX(i_mux_vspi_clk, vspi_clk_in_o, d_vspi_clk_in_s);
      CONNECTINMUX(i_mux_vspi_hd, vspi_hd_in_o, d_vspi_hd_in_s);
      CONNECTINMUX(i_mux_vspi_wp, vspi_wp_in_o, d_vspi_wp_in_s);
      CONNECTINMUX(i_mux_vspi_cs0, vspi_cs0_in_o, d_vspi_cs0_in_s);
      CONNECTINMUX(i_mux_vspi_cs1, vspi_cs1_in_o, d_vspi_cs1_in_s);
      CONNECTINMUX(i_mux_vspi_cs2, vspi_cs2_in_o, d_vspi_cs2_in_s);

      /* I2C */
      CONNECTINMUX(i_mux_i2c_sda0, sda0_o, l0_f1);
      CONNECTINMUX(i_mux_i2c_sda1, sda1_o, l0_f1);
      CONNECTINMUX(i_mux_i2c_scl0, scl0_o, l0_f1);
      CONNECTINMUX(i_mux_i2c_scl1, scl1_o, l0_f1);

      /* UART Muxes */
      CONNECTINMUX(i_mux_uart0, uart0rx_o, d_u0rx_s);
      CONNECTINMUX(i_mux_uart1, uart1rx_o, d_u1rx_s);
      CONNECTINMUX(i_mux_uart2, uart2rx_o, d_u2rx_s);

      initptr();

      SC_THREAD(updateth);
      sensitive << updategpioreg_ev << updategpiooe_ev << update_ev;
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
extern gpio_matrix *gpiomatrixptr;

#endif
