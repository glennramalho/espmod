/*******************************************************************************
 * gndemux.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple demux for GN logic use in testbenches.
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
#include "gndemux.h"

void gndemux::drivepin_th() {
   for(;;) {
      // If the selector is invalid, we drive nothing. We will hang here until
      // something changes.
      if (pinS.read() < 0 || pinS.read() >= pin.size()) {
         for (int i = 0; i < pin.size(); i = i + 1) {
            pin[i]->write(def);
         }
         wait(pinS.value_changed_event());
      }

      // If it is valid, we drive the output with the input level.
      else {
         for (int i = 0; i < pin.size(); i = i + 1) {
            if (i == pinS.read()) pin[i]->write(pinY.read());
            else pin[i]->write(def);
         }
         wait(pinS.value_changed_event() | pinY.value_changed_event());
      }
   }
}
