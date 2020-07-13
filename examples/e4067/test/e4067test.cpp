/*******************************************************************************
 * e4067test.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the e4067.ino test.
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
#include "e4067test.h"
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
void e4067test::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, adc, adc.name());
   sc_trace(tf, a0, a0.name());
   sc_trace(tf, a1, a1.name());
   sc_trace(tf, a2, a2.name());
   sc_trace(tf, d0_bool, d0_bool.name());
   sc_trace(tf, d1_bool, d1_bool.name());
   sc_trace(tf, d2_bool, d2_bool.name());
   sc_trace(tf, d3_bool, d3_bool.name());
   sc_trace(tf, d0, d0.name());
   sc_trace(tf, d1, d1.name());
   sc_trace(tf, d2, d2.name());
   sc_trace(tf, d3, d3.name());
   i_esp.trace(tf);
   i_cd4067.trace(tf);
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
void e4067test::start_of_simulation() {
   /* We add a deadtime to the uart client as the ESP sends a glitch down the
    * UART0 at power-up.
    */
   i_uartclient.i_uart.set_deadtime(sc_time(5, SC_US));
   //i_cd4067.set_debug(true);
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
void e4067test::serflush() {
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void e4067test::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");

   // We start off driving some values. */
   a0.write(2.2F);
   a1.write(GN_LOGIC_Z);
   a2.write(1.0F);

   // Now we wait for some time
   wait(4.4, SC_SEC);

   // Now we change the values in mid flight.
   a0.write(0.3F);
   a1.write(1.7F);
   a2.write(2.0F);

   // Now we wait more.
   wait(4, SC_SEC);
}

void e4067test::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
