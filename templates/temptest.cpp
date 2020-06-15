/*******************************************************************************
 * <DIRNAME>test.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the <DIRNAME>.ino test.
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
#include "<DIRNAME>test.h"
#include <string>
#include <vector>
#include "info.h"
#include "main.h"

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
void <DIRNAME>test::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   i_esp.trace(tf);
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
void <DIRNAME>test::serflush() {
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void <DIRNAME>test::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");
   wait(1, SC_MS); sc_stop();

   PRINTF_INFO("TEST", "Waiting for power-up");
   wait(500, SC_MS);
}

void <DIRNAME>test::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
