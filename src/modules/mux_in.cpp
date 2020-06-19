/*******************************************************************************
 * mux_in.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Model for the PCNT mux in the GPIO matrix.
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
#include "info.h"
#include "mux_in.h"

void mux_in::mux(int gpiosel) {
   if (gpiosel >= min_i.size()) {
      PRINTF_WARN("MUXPCNT",
        "Attempting to set GPIO Matrix to illegal pin %d.", gpiosel);
      return;
   }
   function = gpiosel;
   fchange_ev.notify();
}

void mux_in::transfer() {
   bool nxval;
   while(true) {
      /* We simply copy the input onto the output. */
      nxval = min_i[function]->read();
      out_o.write(nxval);
      if (debug) {
         PRINTF_INFO("MUXIN", "mux_in %s is driving %c, function %d", name(),
            (nxval)?'H':'L', function);
      }

      /* We wait for a function change or a signal change. */
      wait(fchange_ev | min_i[function]->value_changed_event());
   }
}
