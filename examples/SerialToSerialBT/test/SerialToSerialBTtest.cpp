/*******************************************************************************
 * SerialToSerialBTtest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the SerialToSerialBT.ino test.
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
#include "SerialToSerialBTtest.h"
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
void SerialToSerialBTtest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
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
void SerialToSerialBTtest::start_of_simulation() {
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
void SerialToSerialBTtest::serflush() {
   //i_uartclient.i_uart.set_debug(true);
   //i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void SerialToSerialBTtest::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");

   PRINTF_INFO("TEST", "Waiting for power-up");
   wait(500, SC_MS);

   // We expect first for the program to send us a high.
   std::string msg = i_uartclient.get();
   PRINTF_INFO("TEST", "Received '%s' from serial.", msg.c_str());

   // We should be ready now. We then send something on the Serial and
   // expect it to show up on the bluetooth interface.
   i_uartclient.send("From Serial to BT\r\n");
   i_btclient.expect("From Serial to BT");

   // And we do the same for the reverse direction too.
   i_btclient.send("From BT to Serial\r\n");
   i_uartclient.expect("From BT to Serial");
}

void SerialToSerialBTtest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   SC_REPORT_INFO("TEST", "Simulation Finished.");

   sc_stop();
}
