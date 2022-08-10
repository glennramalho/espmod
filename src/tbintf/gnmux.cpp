/*******************************************************************************
 * gnmux.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple gnmux for GN logic use in testbenches.
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
#include "gnmux.h"

void gnmux::drivey_th() {
   for(;;) {
      // If the selector is invalid, we drive nothing. We will hang here until
      // something changes.
      if (pinS.read() < 0 || pinS.read() >= pin.size()) {
         pinY.write(def);
         wait(pinS.value_changed_event());
      }

      // If it is valid, we drive the output with the input level.
      else {
         pinY.write(pin[pinS.read()]->read());
         // We now wait until a change in the input or the selector.
         wait(pinS.value_changed_event() |
            pin[pinS.read()]->value_changed_event());
      }
   }
}
