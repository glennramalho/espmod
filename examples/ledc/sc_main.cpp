/*******************************************************************************
 * sc_main.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   The sc_main is the first function always called by the SystemC environment.
 * This one takes as arguments a test name and an option:
 *
 *    testname.x [testnumber] [+waveform]
 *
 *    testnumber - test to run, 0 for t0, 1 for t1, etc. Default = 0.
 *    +waveform - generate VCD file for top level signals.
 *
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

ledctest i_ledctest("i_ledctest");

int sc_main(int argc, char *argv[]) {
   int tn;
   bool wv = false;
   if (argc > 3) {
      SC_REPORT_ERROR("MAIN", "Too many arguments used in call");
      return 1;
   }
   /* If no arguments are given, argc=1, there is no waveform and we run
    * test 0.
    */
   else if (argc == 1) {
      tn = 0;
      wv = false;
   }
   /* If we have at least one argument, we check if it is the waveform command.
    * If it is, we set a tag. If we have two arguments, we assume the second
    * one is the test number.
    */
   else if (strcmp(argv[1], "+waveform")==0) {
      wv = true;
      /* If two arguments were given (argc=3) we assume the second one is the
       * test name. If the testnumber was not given, we run test 0.
       */
      if (argc == 3) tn = atoi(argv[2]);
      else tn = 0;
   }
   /* For the remaining cases, we check. If there are two arguments, the first
    * one must be the test number and the second one might be the waveform
    * option. If we see 2 arguments, we do that. If we do not, then we just
    * have a testcase number.
    */
   else {
      if (argc == 3 && strcmp(argv[2], "+waveform")==0) wv = true;
      else wv = false;
      tn = atoi(argv[1]);
   }

   /* We start the wave tracing. */
   sc_trace_file *tf = NULL;
   if (wv) {
      tf = sc_create_vcd_trace_file("waves");
      i_ledctest.trace(tf);
   }

   /* We need to connect the Arduino pin library to the gpios. */
   i_ledctest.i_esp.pininit();

   /* Set the test number */
   i_ledctest.tn = tn;

   /* And run the simulation. */
   sc_start();
   if (wv) sc_close_vcd_trace_file(tf);
   exit(0);
}
