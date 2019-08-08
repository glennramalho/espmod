/*******************************************************************************
 * HelloServertest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the HelloServer.ino test.
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

#include "HelloServertest.h"
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
void HelloServertest::start_of_simulation() {
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
void HelloServertest::trace(sc_trace_file *tf) {
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, led, led.name());
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
void HelloServertest::serflush() {
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void HelloServertest::t0(void) {
   SC_REPORT_INFO("TEST", "Running T0: blank flash.");

   /* We expect to receive messages from the Serial. */
   i_uartclient.expect("");
   i_uartclient.expect("");
   i_uartclient.expect("Connected to awifi");
   i_uartclient.expect("IP address: 192.76.0.100");
   i_uartclient.expect("HTTP server started");

   /* Now, we can connect to the server and request some pages. */

   /* First we ask for the root page. */
   i_webclient.connectclient(IPAddress(192,76,0,100), 80);
   i_webclient.requestpage(80, "/");
   /* The LED should switch on. */
   wait(led.posedge_event());
   wait(led.negedge_event());
   i_webclient.printpage(80);

   /* Now we get the inline page. */
   i_webclient.connectclient(IPAddress(192,76,0,100), 80);
   i_webclient.requestpage(80, "/inline");
   /* This time the led does not switch on. */
   i_webclient.printpage(80);

   /* And we try some other page that is not there. */
   i_webclient.connectclient(IPAddress(192,76,0,100), 80);
   i_webclient.requestpage(80, "/wakanda");
   /* The LED should switch on. */
   wait(led.posedge_event());
   wait(led.negedge_event());
   i_webclient.printpage(80);

   /* Ok, enough testing. We wait a bit just to prolong a bit the waveforms
    * on the VCD files. Helps make them more visible there.
    */
   wait(500, SC_MS);
}

void HelloServertest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
