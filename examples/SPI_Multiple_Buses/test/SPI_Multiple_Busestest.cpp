/*******************************************************************************
 * SPI_Multiple_Busestest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the SPI_Multiple_Buses.ino test.
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
#include "SPI_Multiple_Busestest.h"
#include <string>
#include <vector>
#include "info.h"


/**********************
 * Function: trace()
 * inputs: trace file
 * outputs: none
 * return: none
 * globals: none
 *
 * Traces all signals in the design. For a signal to be traced it must be listed
 * here. This function should also call tracing in any subblocks, if desired.
 */
void SPI_Multiple_Busestest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, vspi_sck, vspi_sck.name());
   sc_trace(tf, vspi_miso, vspi_miso.name());
   sc_trace(tf, vspi_mosi, vspi_mosi.name());
   sc_trace(tf, vspi_ss, vspi_ss.name());
   sc_trace(tf, hspi_sck, hspi_sck.name());
   sc_trace(tf, hspi_miso, hspi_miso.name());
   sc_trace(tf, hspi_mosi, hspi_mosi.name());
   sc_trace(tf, hspi_ss, hspi_ss.name());
   i_esp.trace(tf);
}

/**********************
 * Task: start_of_simulation():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Runs commands at the beginning of the simulation.
 */
void SPI_Multiple_Busestest::start_of_simulation() {
   /* We add a deadtime to the uart client as the ESP sends a glitch down the
    * UART0 at power-up.
    */
   i_uartclient.i_uart.set_deadtime(sc_time(5, SC_US));
}

/**********************
 * Task: serflush():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Dumps everything comming from the serial interface.
 */
void SPI_Multiple_Busestest::serflush() {
   //i_uartclient.i_uart.set_debug(true);
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void SPI_Multiple_Busestest::t0(void) {
   int i, v, vret;
   SC_REPORT_INFO("TEST", "Running Test T0.");

   hspi_miso.write(GN_LOGIC_0); vspi_miso.write(GN_LOGIC_0);

   v = 0;
   vret = 0x79;
   /* We wait for the VSPI to turn on. For that we look at the SS signal. */
   PRINTF_INFO("TEST", "Waiting for VSPI");
   do {
      wait(sc_time(100, SC_MS), vspi_ss.value_changed_event());
   } while (vspi_ss.read() != GN_LOGIC_0);
   PRINTF_INFO("TEST", "SS went low");

   /* Then we can take the data, one bit at a time. */
   for(i = 7; i >= 0; i = i - 1) {
      vspi_miso.write(((vret & (1 << i))>0)?GN_LOGIC_1:GN_LOGIC_0);
      do {
         wait(sc_time(100, SC_MS), vspi_sck.value_changed_event());
         if (!vspi_sck.event()) {
            PRINTF_ERROR("TEST",
               "Timed out while waiting for bit %d of the VSPI", i);
         }
      } while (vspi_sck.read() != GN_LOGIC_0);
      PRINTF_INFO("TEST", "Sampled bit %d on MOSI at %c", i,
         vspi_mosi.read().to_char());
      if (vspi_mosi.read() == GN_LOGIC_1) v = v | (1<<i);
   }
   PRINTF_INFO("TEST", "Sent %02x and received %02x from VSPI", vret, v);

   PRINTF_INFO("TEST", "Waiting for HSPI");

   v = 0;
   vret = 0xa3;
   for(i = 7; i >= 0; i = i - 1) {
      hspi_miso.write(((vret & (1 << i))>0)?GN_LOGIC_1:GN_LOGIC_0);
      do {
         wait(sc_time(100, SC_MS), hspi_sck.value_changed_event());
         if (!hspi_sck.event()) {
            PRINTF_ERROR("TEST",
               "Timed out while waiting for bit %d of the HSPI", i);
         }
      } while (hspi_sck.read() != GN_LOGIC_0);
      PRINTF_INFO("TEST", "Sampled bit %d on MOSI at %c", i,
         hspi_mosi.read().to_char());
      if (hspi_mosi.read() == GN_LOGIC_1) v = v | (1<<i);
   }
   PRINTF_INFO("TEST", "Sent %02x and received %02x from HSPI", vret, v);

   wait(1, SC_SEC);
}

void SPI_Multiple_Busestest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
