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
#include "gpio_simple.h"
#include "gpio_mf.h"
#include "gpio_mix.h"
#include "gpio_mfmix.h"
#include "clkgen.h"
#include "pcntmod.h"
#include "netcon.h"
#include "adc_types.h"
#include "gn_mixed.h"

/* To make the functions easier to read, we make a shortcut to connecting
 * a function.
 */
#define CONNECTFUNC(gpiodg,fins,fens,fouts) { \
   gpiodg.fin(fins); \
   gpiodg.fen(fens); \
   gpiodg.fout(fouts); \
}

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
   clkgen i_clkgen{"i_clkgen"};
   pcntmod i_pcnt{"i_pcnt"};
   adc1 i_adc1{"i_adc1"};
   adc2 i_adc2{"i_adc2"};
   uart i_uart0 {"i_uart0", 64, 64};
   /* UART1 is dead as it shares pins with the flash. */
   uart i_uart2 {"i_uart2", 64, 64};
   /* Actually this is a QSPI */
   cchan i_uflash {"i_uflash", 256*4, 256*4};
   /* Not sure what is the real interface */
   cchan i_uwifi {"i_uwifi", 2048, 2048}; /* The actual buffer varies 1k-8k */
   uart i_ui2c {"i_ui2c", 32*8, 32*8};   /* to replace with an I2C */

   /* GPIOs */
   gpio_mfmix i_gpio_d0 {"i_gpio_d0",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU};
   gpio_mfmix i_gpio_d1 {"i_gpio_d1",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU};
   gpio_mix i_gpio_d2 {"i_gpio_d2",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD};
   gpio_mfmix i_gpio_d3 {"i_gpio_d3",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU};
   gpio_mfmix i_gpio_d4 {"i_gpio_d4",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD};
   gpio_mix i_gpio_d5 {"i_gpio_d5",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_NONE};
   gpio_mfmix i_gpio_d12 {"i_gpio_d12",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD};
   gpio_mfmix i_gpio_d13{"i_gpio_d13",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU}; /* Not quite, but ok for now */
   gpio_mfmix i_gpio_d14 {"i_gpio_d14",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPD}; /* Not quite, but ok for now */
   gpio_mfmix i_gpio_d15 {"i_gpio_d15",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT | GPIOMODE_WPU};
   gpio_mfmix i_gpio_d16 {"i_gpio_d16",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE};
   gpio_mfmix i_gpio_d17 {"i_gpio_d17",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_NONE};
   gpio_mix i_gpio_d18 {"i_gpio_d18",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT};
   gpio_mix i_gpio_d19 {"i_gpio_d19",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT};
   /* There is no GPIO D20 */
   gpio_mix i_gpio_d21 {"i_gpio_d21",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
            GPIOMODE_INPUT};
   gpio_mix i_gpio_d22 {"i_gpio_d22",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_INPUT};
   gpio_mix i_gpio_d23 {"i_gpio_d23",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_INPUT};
   /* There is no GPIO D24 */
   gpio_mix i_gpio_d25 {"i_gpio_d25",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_NONE};
   gpio_mix i_gpio_d26 {"i_gpio_d26",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_NONE};
   gpio_mix i_gpio_d27 {"i_gpio_d27",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_INPUT};
   /* There is no GPIO d28 */
   /* There is no GPIO d29 */
   /* There is no GPIO d30 */
   /* There is no GPIO d31 */
   gpio_mix i_gpio_d32 {"i_gpio_d32",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d33 {"i_gpio_d33",
            GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
            GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d34 {"i_gpio_d34",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d35 {"i_gpio_d35",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d36 {"i_gpio_d36",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d37 {"i_gpio_d37",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d38 {"i_gpio_d38",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};
   gpio_mix i_gpio_d39 {"i_gpio_d39",
      GPIOMODE_NONE | GPIOMODE_INPUT, GPIOMODE_NONE, true};

   /* signals */
   sc_signal<bool> uart0rx {"uart0rx"};
   sc_signal<bool> uart0tx {"uart0tx"};
   sc_signal<bool> uart2rx {"uart2rx"};
   sc_signal<bool> uart2tx {"uart2tx"};
   sc_signal<bool> apb_clk;
   sc_signal<bool> sig_ch0_un0 {"sig_ch0_un0"};
   sc_signal<bool> sig_ch1_un0 {"sig_ch1_un0"};
   sc_signal<bool> ctrl_ch0_un0 {"ctrl_ch0_un0"};
   sc_signal<bool> ctrl_ch1_un0 {"ctrl_ch1_un0"};
   sc_signal<bool> sig_ch0_un1 {"sig_ch0_un1"};
   sc_signal<bool> sig_ch1_un1 {"sig_ch1_un1"};
   sc_signal<bool> ctrl_ch0_un1 {"ctrl_ch0_un1"};
   sc_signal<bool> ctrl_ch1_un1 {"ctrl_ch1_un1"};
   sc_signal<bool> sig_ch0_un2 {"sig_ch0_un2"};
   sc_signal<bool> sig_ch1_un2 {"sig_ch1_un2"};
   sc_signal<bool> ctrl_ch0_un2 {"ctrl_ch0_un2"};
   sc_signal<bool> ctrl_ch1_un2 {"ctrl_ch1_un2"};
   sc_signal<bool> sig_ch0_un3 {"sig_ch0_un3"};
   sc_signal<bool> sig_ch1_un3 {"sig_ch1_un3"};
   sc_signal<bool> ctrl_ch0_un3 {"ctrl_ch0_un3"};
   sc_signal<bool> ctrl_ch1_un3 {"ctrl_ch1_un3"};
   sc_signal<bool> sig_ch0_un4 {"sig_ch0_un4"};
   sc_signal<bool> sig_ch1_un4 {"sig_ch1_un4"};
   sc_signal<bool> ctrl_ch0_un4 {"ctrl_ch0_un4"};
   sc_signal<bool> ctrl_ch1_un4 {"ctrl_ch1_un4"};
   sc_signal<bool> sig_ch0_un5 {"sig_ch0_un5"};
   sc_signal<bool> sig_ch1_un5 {"sig_ch1_un5"};
   sc_signal<bool> ctrl_ch0_un5 {"ctrl_ch0_un5"};
   sc_signal<bool> ctrl_ch1_un5 {"ctrl_ch1_un5"};
   sc_signal<bool> sig_ch0_un6 {"sig_ch0_un6"};
   sc_signal<bool> sig_ch1_un6 {"sig_ch1_un6"};
   sc_signal<bool> ctrl_ch0_un6 {"ctrl_ch0_un6"};
   sc_signal<bool> ctrl_ch1_un6 {"ctrl_ch1_un6"};
   sc_signal<bool> sig_ch0_un7 {"sig_ch0_un7"};
   sc_signal<bool> sig_ch1_un7 {"sig_ch1_un7"};
   sc_signal<bool> ctrl_ch0_un7 {"ctrl_ch0_un7"};
   sc_signal<bool> ctrl_ch1_un7 {"ctrl_ch1_un7"};

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


   /* Processes */
   void dut(void);

   // Constructor
   SC_CTOR(doitesp32devkitv1) {
      /* Pin Hookups */
      i_uart0.rx(uart0rx); i_uart0.tx(uart0tx);
      i_uart2.rx(uart2rx); i_uart2.tx(uart2tx);
      i_uflash.rx(frx);
      i_uflash.tx(ftx);
      i_uwifi.rx(wrx);
      i_uwifi.tx(wtx);
      i_ui2c.rx(irx);
      i_ui2c.tx(itx);
      /* GPIO 0 */
      i_gpio_d0.pin(d0_a11);
      CONNECTFUNC(i_gpio_d0,  l0_f1, l1_f1, sig_open); /* F1 GPIO -- built-in*/
      CONNECTFUNC(i_gpio_d0,  l0_f2, l1_f2, sig_open); /* F2 CLK_OUT1 - notsup*/
      CONNECTFUNC(i_gpio_d0,  l0_f3, l1_f3, sig_open); /* F3 GPIO -- built-in*/
      CONNECTFUNC(i_gpio_d0,  l0_f4, l1_f4, sig_open); /* F4 -- not used */
      CONNECTFUNC(i_gpio_d0,  l0_f5, l1_f5, sig_open); /* F5 -- not used */
      CONNECTFUNC(i_gpio_d0,  l0_f6, l1_f6, sig_open); /* F6 -- EMAC_TX_CLR */
      /* Commented out for now as the I2C is still using a CCHAN. */
      CONNECTFUNC(i_gpio_d0,  l0_f7, l1_f7, sig_open); /* RTCF2 -- I2C_SDA */
      //CONNECTFUNC(i_gpio_d0, logic_1, i2c_sda_tx, i2c_sda_rx); /* F7 */
      /* GPIO 1 */
      i_gpio_d1.pin(d1);
      CONNECTFUNC(i_gpio_d1, uart0tx, l1_f1, sig_open); /* F1 */
      /* F2: CLK_OUT3 -- not supported. */
      /* F3: GPIO -- built-in to GPIO. */
      /* GPIO 2 */
      i_gpio_d2.pin(d2_a12);
      /* F1: GPIO -- built-in */
      /* F2: HSPIWP -- not yet supported */
      /* F3: GPIO -- built-in */
      /* F4: HS2_DATA0 -- not yet supported */
      /* F5: SD_DATA0 -- not yet supported */
      /* GPIO 3 */
      i_gpio_d3.pin(d3);
      CONNECTFUNC(i_gpio_d3,  logic_0, l1_f1, uart0rx); /* F1 */
      /* F2: CLK_OUT2 -- not supported. */
      /* F3: GPIO -- built-in to GPIO. */
      /* GPIO 4 */
      i_gpio_d4.pin(d4_a10);
      CONNECTFUNC(i_gpio_d4,  l0_f1, l1_f1, sig_open); /* F1: GPIO -- builtin */
      CONNECTFUNC(i_gpio_d4,  l0_f2, l1_f2, sig_open); /* F2: HSPID */
      CONNECTFUNC(i_gpio_d4,  l0_f3, l1_f3, sig_open); /* F3: GPIO -- builtin */
      CONNECTFUNC(i_gpio_d4,  l0_f4, l1_f4, sig_open); /* F4: HS2_DATA1 */
      CONNECTFUNC(i_gpio_d4,  l0_f5, l1_f5, sig_open); /* F5: SD_DATA1 */
      CONNECTFUNC(i_gpio_d4,  l0_f6, l1_f6, sig_open); /* F6: EMAC_TX_ER */
      /* Commented out for now as the I2C is still using a CCHAN. */
      /* RTC FUNC 1: RTC_GPIO10 */
      CONNECTFUNC(i_gpio_d4,  l0_f7, l1_f7, sig_open); /* RTC FUNC 2: I2CSCL */
      //CONNECTFUNC(i_gpio_d4, l0_f7, i2c_sda_tx, i2c_sda_rx); /* F7 */
      i_gpio_d5.pin(d5);
      /* F1: GPIO -- built-in */
      /* F2: VSPICS0 -- not supported. */
      /* F3: GPIO -- build-in */
      /* F4: HS1_DATA6 -- not supported. */
      /* GPIO  6 -- not yet supported, implemented via the flash channel. */
      /* GPIO  7 -- not yet supported, implemented via the flash channel. */
      /* GPIO  8 -- not yet supported, implemented via the flash channel. */
      /* GPIO  9 -- not yet supported, implemented via the flash channel. */
      /* GPIO 10 -- not yet supported, implemented via the flash channel. */
      /* GPIO 11 -- not yet supported, implemented via the flash channel. */
      /* GPIO 12 */
      i_gpio_d12.pin(d12_a15);
      CONNECTFUNC(i_gpio_d12,  l0_f1, l1_f1, sig_open); /* F1 MTDO -- not sup.*/
      CONNECTFUNC(i_gpio_d12,  l0_f1, l1_f1, sig_ch0_un0); /* TODO REmove .*/
      /* F2: HSPIQ -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS2_DATA2 -- not supported. */
      /* F5: SD_DATA2 -- not supported. */
      /* F6: EMAC_TXD3 */
      i_gpio_d13.pin(d13_a14);
      CONNECTFUNC(i_gpio_d13,  l0_f1, l1_f1, sig_open); /* F1 MTCK -- not sup.*/
      CONNECTFUNC(i_gpio_d13,  l0_f1, l1_f1, sig_ch1_un0); /* TODO REmove .*/
      /* F2: HSPID -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS2_DATA3 -- not supported. */
      /* F5: SD_DATA3 -- not supported. */
      /* F6: EMAC_RX_ER */
      i_gpio_d14.pin(d14_a16);
      CONNECTFUNC(i_gpio_d14,  l0_f1, l1_f1, sig_open); /* F1 MTMS -- not sup.*/
      CONNECTFUNC(i_gpio_d14,  l0_f1, l1_f1, ctrl_ch0_un0); /* TODO REmove .*/
      /* F2: HSPICLK -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS2_CLK -- not supported. */
      /* F5: SD_CLK -- not supported. */
      /* F6: EMAC_TXD2 */
      i_gpio_d15(d15_a13);
      CONNECTFUNC(i_gpio_d15,  l0_f1, l1_f1, sig_open); /* F1 MTDO -- not sup.*/
      CONNECTFUNC(i_gpio_d15,  l0_f1, l1_f1, ctrl_ch1_un0); /* TODO REmove .*/
      /* F2: HSPICS0 -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS2_CMD -- not supported. */
      /* F5: SD_CMD -- not supported. */
      /* F6: EMAC_RXD3 */
      i_gpio_d16.pin(d16);
      CONNECTFUNC(i_gpio_d16, l0_f1, l1_f1, sig_open); /* F1 GPIO -- built-in */
      CONNECTFUNC(i_gpio_d16, l0_f2, l1_f2, sig_open); /* F2 -- not used */
      CONNECTFUNC(i_gpio_d16, l0_f3, l1_f3, sig_open); /* F3 GPIO -- built-in */
      CONNECTFUNC(i_gpio_d16, l0_f4, l1_f4, sig_open); /* F4 HS1_DATA4 not sup*/
      CONNECTFUNC(i_gpio_d16, l0_f5, l1_f5, uart2rx);  /* F5 U2RXD */
      /* F6: EMAC_CLK_OUT */
      /* GPIO 17 */
      i_gpio_d17.pin(d17);
      CONNECTFUNC(i_gpio_d17, l0_f1, l1_f1, sig_open); /* F1 GPIO -- built-in */
      CONNECTFUNC(i_gpio_d17, l0_f2, l1_f2, sig_open); /* F2 -- not used*/
      CONNECTFUNC(i_gpio_d17, l0_f3, l1_f3, sig_open); /* F3 GPIO -- built-in */
      CONNECTFUNC(i_gpio_d17, l0_f4, l1_f4, sig_open); /* F4 HS1_DATA5 not sup*/
      CONNECTFUNC(i_gpio_d17, uart2tx,l1_f5,sig_open); /* F5 U2TXD */
      /* F6: EMAC_CLK_OUT_180 */
      i_gpio_d18.pin(d18);
      /* F1: GPIO -- built-in */
      /* F2: VSPICLK -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS1_DATA7 -- not supported. */
      i_gpio_d19.pin(d19);
      /* F1: GPIO -- built-in */
      /* F2: VSPIQ -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: U0CTS -- not supported. */
      /* F5: -- not used. */
      /* F6: EMAC TXDO */
      i_gpio_d21.pin(d21);
      /* F1: GPIO -- built-in */
      /* F2: VSPIHD -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC TX EN */
      i_gpio_d22.pin(d22);
      /* F1: GPIO -- built-in */
      /* F2: VSPIWP -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: U0RTS -- not supported. */
      /* F5: -- not used. */
      /* F6: EMAC TXD1 */
      i_gpio_d23.pin(d23);
      /* F1: GPIO -- built-in */
      /* F2: VSPID -- not supported. */
      /* F3: GPIO -- built-in */
      /* F4: HS1_STROBE -- not supported. */
      /* F5: -- not used. */
      /* F6: -- not used. */
      i_gpio_d25.pin(d25_a18);
      /* F1: GPIO -- built-in */
      /* F2: -- not used. */
      /* F3: GPIO -- built-in */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RXD0 */
      i_gpio_d26.pin(d26_a19);
      /* F1: GPIO -- built-in */
      /* F2: -- not used. */
      /* F3: GPIO -- built-in */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RXD1 */
      i_gpio_d27.pin(d27_a17);
      /* F1: GPIO -- built-in */
      /* F2: -- not used. */
      /* F3: GPIO -- built-in */
      /* F4: -- not used. */
      /* F5: -- not used. */
      /* F6: EMAC RX_DV */
      /* These below, asside from the ADC do not have any supported additional
       * functions.
       */
      i_gpio_d32.pin(d32_a4);
      i_gpio_d33.pin(d33_a5);
      i_gpio_d34.pin(d34_a6);
      i_gpio_d35.pin(d35_a7);
      i_gpio_d36.pin(d36_a0);
      i_gpio_d37.pin(d37_a1);
      i_gpio_d38.pin(d38_a2);
      i_gpio_d39.pin(d39_a3);

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

      /* And we connect the PCNT. */
      i_pcnt.sig_ch0_un(sig_ch0_un0); i_pcnt.sig_ch1_un(sig_ch1_un0);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un0); i_pcnt.ctrl_ch1_un(ctrl_ch1_un0);
      i_pcnt.sig_ch0_un(sig_ch0_un1); i_pcnt.sig_ch1_un(sig_ch1_un1);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un1); i_pcnt.ctrl_ch1_un(ctrl_ch1_un1);
      i_pcnt.sig_ch0_un(sig_ch0_un2); i_pcnt.sig_ch1_un(sig_ch1_un2);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un2); i_pcnt.ctrl_ch1_un(ctrl_ch1_un2);
      i_pcnt.sig_ch0_un(sig_ch0_un3); i_pcnt.sig_ch1_un(sig_ch1_un3);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un3); i_pcnt.ctrl_ch1_un(ctrl_ch1_un3);
      i_pcnt.sig_ch0_un(sig_ch0_un4); i_pcnt.sig_ch1_un(sig_ch1_un4);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un4); i_pcnt.ctrl_ch1_un(ctrl_ch1_un4);
      i_pcnt.sig_ch0_un(sig_ch0_un5); i_pcnt.sig_ch1_un(sig_ch1_un5);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un5); i_pcnt.ctrl_ch1_un(ctrl_ch1_un5);
      i_pcnt.sig_ch0_un(sig_ch0_un6); i_pcnt.sig_ch1_un(sig_ch1_un6);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un6); i_pcnt.ctrl_ch1_un(ctrl_ch1_un6);
      i_pcnt.sig_ch0_un(sig_ch0_un7); i_pcnt.sig_ch1_un(sig_ch1_un7);
      i_pcnt.ctrl_ch0_un(ctrl_ch0_un7); i_pcnt.ctrl_ch1_un(ctrl_ch1_un7);

      SC_THREAD(dut);
   }

   void pininit();
   void start_of_simulation();
   void trace(sc_trace_file *tf);
};

#endif
