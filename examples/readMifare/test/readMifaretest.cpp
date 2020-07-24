/*******************************************************************************
 * readMifaretest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * File: readMifaretest.cpp
 * Project: IWM testbench
 *******************************************************************************
 * Description:
 *   This is the main testbench for the readMifare.ino test.
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
#include "readMifaretest.h"
#include <string>
#include "info.h"

/**********************
 * Task: serflush():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Dumps everything comming from the serial interface.
 */
void readMifaretest::serflush() {
   //i_uartclient.i_uart.set_debug(true);
   i_uartclient.dump();
}

/*******************************************************************************
** Other ***********************************************************************
*******************************************************************************/

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
void readMifaretest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, i2c_sda, i2c_sda.name());
   sc_trace(tf, i2c_scl, i2c_scl.name());
   sc_trace(tf, pn532_irq, pn532_irq.name());
   sc_trace(tf, pn532_reset, pn532_reset.name());
   i_esp.trace(tf);
   i_pn532.trace(tf);
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
void readMifaretest::start_of_simulation() {
   /* We add a deadtime to the uart client as the ESP sends a glitch down the
    * UART0 at power-up.
    */
   i_uartclient.i_uart.set_deadtime(sc_time(5, SC_US));
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void readMifaretest::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");

   PRINTF_INFO("TEST", "Waiting for power-up");

   /* We now wait for some readouts and then put a card on the sensor. */
   wait(2, SC_SEC);
   i_pn532.setcardpresent();
   wait(1, SC_SEC);
   i_pn532.setcardnotpresent();
   wait(1, SC_SEC);
}

void readMifaretest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
