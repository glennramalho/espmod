/*******************************************************************************
 * tpencoder.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a testbench module to emulate a rotary quad encoder with a button.
 *   This encoder encodes the button by forcing pin B high.
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

#ifndef _TPENCODER_H
#define _TPENCODER_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(tpencoder) {
   sc_inout<gn_mixed> pinA {"pinA"};
   sc_inout<gn_mixed> pinB {"pinB"};
   sc_signal<bool> buttonpressed {"buttonpressed", false};

   private:
   int speed; /* in miliseconds */
   bool lastwasright;

   public:
   void set_speed(int ns) { speed = ns; }
   int get_speed() { return speed; }
   void press(bool pb);
   void turnright(int pulses, bool pressbutton = false);
   void turnleft(int pulses, bool pressbutton = false);
   void invertturn(int pulses);

   SC_CTOR(tpencoder) {
      /* We start off setting the default to 200ms and floating the two pins. */
      speed = 200;
      lastwasright = false;
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};

#endif
