/*******************************************************************************
 * Blinktest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the Blink.ino test.
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

#include "Blinktest.h"
#include <string>
#include <vector>
#include "info.h"

/**********************
 * start_of_simulation():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Invokes all commands that must be called before we start simulating.
 */
void Blinktest::start_of_simulation() {
   /* We need logic_0 to always be 0. */
   logic_0.write(SC_LOGIC_0);
}

/**********************
 * trace():
 * inputs: trace file
 * outputs: none
 * return: none
 * globals: none
 *
 * Traces all signals in the design. For a signal to be traced it must be listed
 * here. This function should also call tracing in any subblocks, if desired.
 */
void Blinktest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   i_esp.trace(tf);
}

/**********************
 * serflush():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Dumps everything comming from the serial interface.
 */
void Blinktest::serflush() {
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void Blinktest::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");

   PRINTF_INFO("TEST", "Waiting for power-up");
   wait(led.posedge_event());
   PRINTF_INFO("TEST", "SUCCESS: Led went on");
   wait(led.negedge_event());
   PRINTF_INFO("TEST", "SUCCESS: Led went off");
   wait(led.posedge_event());
   PRINTF_INFO("TEST", "SUCCESS: Led went on");
   wait(led.negedge_event());
   PRINTF_INFO("TEST", "SUCCESS: Led went off");
   wait(500, SC_MS);
}

void Blinktest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
