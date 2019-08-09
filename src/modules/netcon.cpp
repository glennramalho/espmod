/*******************************************************************************
 * netcon.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Simple block that connects two sc_signals, one being bool and one being
 *   sc_signal_resolved.
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
#include "netcon.h"
#include "info.h"

void netcon_rvtobool::transport() {
   for(;;) {
      wait();
      if (a.read() == SC_LOGIC_0) b.write(false);
      else if (a.read() == SC_LOGIC_1) b.write(true);
      else {
         /* We only warn if we are not at time 0 or we get some dummy
          * warnings.
          */
         if (sc_time_stamp() != sc_time(0, SC_NS)) {
            PRINTF_WARN("NETCON", "Assigning %c to signal %s",
               a.read().to_char(), name());
         }
         b.write(false);
      }
   }
}

void netcon_booltorv::transport() {
   b.write(SC_LOGIC_0);
   for(;;) {
      wait();
      if (a.read() == false) b.write(SC_LOGIC_0);
      else b.write(SC_LOGIC_1);
   }
}

void netcon_mixtobool::transport() {
   for(;;) {
      wait();
      if (a.read() == SC_LOGIC_0) b.write(false);
      else if (a.read() == SC_LOGIC_1) b.write(true);
      else {
         /* We only warn if we are not at time 0 or we get some dummy
          * warnings.
          */
         if (sc_time_stamp() != sc_time(0, SC_NS)) {
            PRINTF_WARN("NETCON", "Assigning %c to signal %s",
               a.read().to_char(), name());
         }
         b.write(false);
      }
   }
}

void netcon_booltomix::transport() {
   b.write(GN_LOGIC_0);
   for(;;) {
      wait();
      if (a.read() == false) b.write(GN_LOGIC_0);
      else b.write(GN_LOGIC_1);
   }
}

void netcon_mixtorv::transport() {
   for(;;) {
      wait();
      b.write(a.read().logic);
   }
}

void netcon_rvtomix::transport() {
   b.write(GN_LOGIC_X);
   for(;;) {
      wait();
      b.write(gn_mixed(a.read()));
   }
}

void netcon_mixtoana::transport() {
   b.write(0.0);
   for(;;) {
      wait();
      b.write(a.read().lvl);
   }
}

void netcon_anatomix::transport() {
   b.write(GN_LOGIC_X);
   for(;;) {
      wait();
      b.write(gn_mixed(a.read()));
   }
}
