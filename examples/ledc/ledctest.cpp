/*******************************************************************************
 * ledctest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the ledc.ino test.
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
#include "ledctest.h"
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
void ledctest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, pwm0, pwm0.name());
   sc_trace(tf, pwm1, pwm1.name());
   sc_trace(tf, pwm2, pwm2.name());
   sc_trace(tf, pwm3, pwm3.name());
   sc_trace(tf, ctrl0, ctrl0.name());
   sc_trace(tf, ctrl1, ctrl1.name());
   sc_trace(tf, ctrl2, ctrl2.name());
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
void ledctest::serflush() {
   i_uartclient.dump();
}

/**********************
 * Task: drivewave():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Drives a waveform onto the pwm0 pin.
 */
void ledctest::drivewave() {
   int c;
   pwm0.write(false);
   pwm1.write(false);
   pwm2.write(false);
   pwm3.write(false);

   while(true) {
      ctrl1.write(false);
      for(c = 0; c < 20; c = c + 1) {
         wait(1, SC_MS);
         pwm0.write(true);
         wait(300, SC_NS);
         pwm1.write(true);
         pwm2.write(true);
         wait(200, SC_NS);
         pwm0.write(false);
         wait(300, SC_NS);
         pwm1.write(false);
         pwm2.write(false);
      }
      ctrl1.write(true);
      for(c = 0; c < 20; c = c + 1) {
         wait(1, SC_MS);
         pwm0.write(true);
         wait(300, SC_NS);
         pwm1.write(true);
         pwm3.write(true);
         wait(200, SC_NS);
         pwm0.write(false);
         wait(300, SC_NS);
         pwm1.write(false);
         pwm3.write(false);
      }
   }
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void ledctest::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");

   PRINTF_INFO("TEST", "Waiting for power-up");
   ctrl0.write(false);
   ctrl2.write(false);
   wait(500, SC_MS);
}

void ledctest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
