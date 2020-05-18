/*******************************************************************************
 * doitesp32devkitv1.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "info.h"
#include "doitesp32devkitv1.h"
#include "gpioset.h"
#include "lwip/sockets.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "Wire.h"
#include "driver/adc.h"
#include "reset_reason.h"

void doitesp32devkitv1::dut(void) {
   wait(125, SC_NS);
   /* We start running the Arduino Setup Function. */
   setup();

   /* Now we run repeatedly the Arduino loop function. */
   for (;;) {
      loop();

      /* For now we have no way to restart the simulation, so we stop it and
       * a new simulation needs to be ran with the post-software reset flow.
       */
      if (ctrlregs.return_to_start) {
         ctrlregs.return_to_start = false;
         PRINTF_INFO("DUT", "Software reset was called.");
         sc_stop();
      }
   }
}

void doitesp32devkitv1::start_of_simulation() {
   /* We change the behavior of SC_ERROR to call SC_STOP instead of SC_THROW.
    * This helps us because the simulation will stop nicely after an error and
    * any waveforms get closed. In addition, any exit handlers called in the
    * model or the firmware can check to see if an error message has been
    * raised. This is important as, when the exit handlers are called, any
    * structs in the model area are stale and should not be checked.
    *
    * See espm_close() as an example of this check.
    */
   sc_report_handler::set_actions(SC_ERROR,
      SC_LOG | SC_DISPLAY | SC_CACHE_REPORT | SC_STOP);
   /* Before we simulate, we initialize the sockets so that they are ready to
    * be used.
    */
   espm_socket_init();

   /* We initialize the reset reason to the state after the boot loader. */
   ctrlregs.cpu0_reset_reason = RTCWDT_RTC_RESET;
   ctrlregs.cpu1_reset_reason = EXT_CPU_RESET;
   ctrlregs.return_to_start = false;
   esp_reset_reason_init();
}

void doitesp32devkitv1::pininit() {
   /* We set each GPIO to be connected to a pin number in the ESPMOD library. */
   pinset(0, &i_gpio_matrix.i_mux_d0);
   pinset(1, &i_gpio_matrix.i_mux_d1);
   pinset(2, &i_gpio_matrix.i_mux_d2);
   pinset(3, &i_gpio_matrix.i_mux_d3);
   pinset(4, &i_gpio_matrix.i_mux_d4);
   pinset(5, &i_gpio_matrix.i_mux_d5);
   pinset(12, &i_gpio_matrix.i_mux_d12);
   pinset(13, &i_gpio_matrix.i_mux_d13);
   pinset(14, &i_gpio_matrix.i_mux_d14);
   pinset(15, &i_gpio_matrix.i_mux_d15);
   pinset(16, &i_gpio_matrix.i_mux_d16);
   pinset(17, &i_gpio_matrix.i_mux_d17);
   pinset(18, &i_gpio_matrix.i_mux_d18);
   pinset(19, &i_gpio_matrix.i_mux_d19);
   pinset(21, &i_gpio_matrix.i_mux_d21);
   pinset(22, &i_gpio_matrix.i_mux_d22);
   pinset(23, &i_gpio_matrix.i_mux_d23);
   pinset(25, &i_gpio_matrix.i_mux_d25);
   pinset(26, &i_gpio_matrix.i_mux_d26);
   pinset(27, &i_gpio_matrix.i_mux_d27);
   pinset(32, &i_gpio_matrix.i_mux_d32);
   pinset(33, &i_gpio_matrix.i_mux_d33);
   pinset(34, &i_gpio_matrix.i_mux_d34);
   pinset(35, &i_gpio_matrix.i_mux_d35);
   pinset(36, &i_gpio_matrix.i_mux_d36);
   pinset(37, &i_gpio_matrix.i_mux_d37);
   pinset(38, &i_gpio_matrix.i_mux_d38);
   pinset(39, &i_gpio_matrix.i_mux_d39);

   /* We do the same for the ADC units. */
   adc1ptr = &i_adc1;
   adc2ptr = &i_adc2;

   /* Other modules. */
   ledcptr = &i_ledc;
   pcntptr = &i_pcnt;
   gpiomatrixptr = &i_gpio_matrix;
   ctrlregsptr = &ctrlregs;
   espintrptr = &i_espintr;

   /* We configure the serial protocols. Each TestSerial needs to be connected
    * to the channel it controls.
    */
   Serial.setports(0, &i_uart0.to, &i_uart0.from, (void *)&i_uart0);
   Serial1.setports(1, &i_uart1.to, &i_uart1.from, (void *)&i_uart1);
   Serial2.setports(2, &i_uart2.to, &i_uart2.from, (void *)&i_uart2);
   WiFi.setports(&i_uwifi.to, &i_uwifi.from);
   Flashport.setports(&i_uflash.to, &i_uflash.from);
   Wire.setports(&i_ui2c.to, &i_ui2c.from);
}

void doitesp32devkitv1::trace(sc_trace_file *tf) {
   i_ledc.trace(tf);
   i_pcnt.trace(tf);
   i_gpio_matrix.trace(tf);
   sc_trace(tf, uart0rx, uart0rx.name());
   sc_trace(tf, uart0tx, uart0tx.name());
   sc_trace(tf, uart2rx, uart2rx.name());
   sc_trace(tf, uart2tx, uart2tx.name());
   sc_trace(tf, d0_a11, d0_a11.name());
   sc_trace(tf, d1, d1.name());
   sc_trace(tf, d2_a12, d2_a12.name());
   sc_trace(tf, d3, d3.name());
   sc_trace(tf, d4_a10, d4_a10.name());
   sc_trace(tf, d5, d5.name());
   sc_trace(tf, d12_a15, d12_a15.name());
   sc_trace(tf, d13_a14, d13_a14.name());
   sc_trace(tf, d14_a16, d14_a16.name());
   sc_trace(tf, d15_a13, d15_a13.name());
   sc_trace(tf, d16, d16.name());
   sc_trace(tf, d17, d17.name());
   sc_trace(tf, d18, d18.name());
   sc_trace(tf, d19, d19.name());
   sc_trace(tf, d21, d21.name());
   sc_trace(tf, d22, d22.name());
   sc_trace(tf, d23, d23.name());
   sc_trace(tf, d25_a18, d25_a18.name());
   sc_trace(tf, d26_a19, d26_a19.name());
   sc_trace(tf, d27_a17, d27_a17.name());
   sc_trace(tf, d32_a4, d32_a4.name());
   sc_trace(tf, d33_a5, d33_a5.name());
   sc_trace(tf, d34_a6, d34_a6.name());
   sc_trace(tf, d35_a7, d35_a7.name());
   sc_trace(tf, d36_a0, d36_a0.name());
   sc_trace(tf, d37_a1, d37_a1.name());
   sc_trace(tf, d38_a2, d38_a2.name());
   sc_trace(tf, d39_a3, d39_a3.name());
   sc_trace(tf, pcntbus_0, pcntbus_0.name());
   sc_trace(tf, pcntbus_1, pcntbus_1.name());
   sc_trace(tf, pcntbus_2, pcntbus_2.name());
   sc_trace(tf, pcntbus_3, pcntbus_3.name());
   sc_trace(tf, pcntbus_4, pcntbus_4.name());
   sc_trace(tf, pcntbus_5, pcntbus_5.name());
   sc_trace(tf, pcntbus_6, pcntbus_6.name());
   sc_trace(tf, pcntbus_7, pcntbus_7.name());
   sc_trace(tf, ledc_sig_hs_0, ledc_sig_hs_0.name());
   sc_trace(tf, ledc_sig_hs_1, ledc_sig_hs_1.name());
   sc_trace(tf, ledc_sig_hs_2, ledc_sig_hs_2.name());
   sc_trace(tf, ledc_sig_hs_3, ledc_sig_hs_3.name());
   sc_trace(tf, ledc_sig_hs_4, ledc_sig_hs_4.name());
   sc_trace(tf, ledc_sig_hs_5, ledc_sig_hs_5.name());
   sc_trace(tf, ledc_sig_hs_6, ledc_sig_hs_6.name());
   sc_trace(tf, ledc_sig_hs_7, ledc_sig_hs_7.name());
   sc_trace(tf, ledc_sig_ls_0, ledc_sig_ls_0.name());
   sc_trace(tf, ledc_sig_ls_1, ledc_sig_ls_1.name());
   sc_trace(tf, ledc_sig_ls_2, ledc_sig_ls_2.name());
   sc_trace(tf, ledc_sig_ls_3, ledc_sig_ls_3.name());
   sc_trace(tf, ledc_sig_ls_4, ledc_sig_ls_4.name());
   sc_trace(tf, ledc_sig_ls_5, ledc_sig_ls_5.name());
   sc_trace(tf, ledc_sig_ls_6, ledc_sig_ls_6.name());
   sc_trace(tf, ledc_sig_ls_7, ledc_sig_ls_7.name());
}
