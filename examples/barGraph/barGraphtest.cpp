/*******************************************************************************
 * barGraphtest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the graphTest.ino test.
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

#include "barGraphtest.h"
#include "info.h"

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
void barGraphtest::trace(sc_trace_file *tf) {
   sc_trace(tf, led1, led1.name());
   sc_trace(tf, led2, led2.name());
   sc_trace(tf, led3, led3.name());
   sc_trace(tf, led4, led4.name());
   sc_trace(tf, led5, led5.name());
   sc_trace(tf, led6, led6.name());
   sc_trace(tf, led7, led7.name());
   sc_trace(tf, led8, led8.name());
   sc_trace(tf, led9, led9.name());
   sc_trace(tf, led10, led10.name());
   i_esp.trace(tf);
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void barGraphtest::t0(void) {
   float a;
   char lights[11];
   SC_REPORT_INFO("TEST", "Running Test T0.");
   
   lights[10] = '\0';

   for(a = 0.0; a < 1.0; a = a + 0.1) {
      a_in.write(a);
      wait(10, SC_MS);

      lights[0] = (led1.read() == SC_LOGIC_1)?'*':' ';
      lights[1] = (led2.read() == SC_LOGIC_1)?'*':' ';
      lights[2] = (led3.read() == SC_LOGIC_1)?'*':' ';
      lights[3] = (led4.read() == SC_LOGIC_1)?'*':' ';
      lights[4] = (led5.read() == SC_LOGIC_1)?'*':' ';
      lights[5] = (led6.read() == SC_LOGIC_1)?'*':' ';
      lights[6] = (led7.read() == SC_LOGIC_1)?'*':' ';
      lights[7] = (led8.read() == SC_LOGIC_1)?'*':' ';
      lights[8] = (led9.read() == SC_LOGIC_1)?'*':' ';
      lights[9] =(led10.read() == SC_LOGIC_1)?'*':' ';
      PRINTF_INFO("TEST", "A0 at %f, lights = %s", a, lights);
   }

   wait(500, SC_MS);
}

void barGraphtest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
