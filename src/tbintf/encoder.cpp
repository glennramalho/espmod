/*******************************************************************************
 * encoder.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a testbench module to emulate a rotary quad encoder with a button.
 *   Two pins are provided for the encoder function and a third one handles
 *   the button.
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
#include "encoder.h"

void encoder::press(bool pb) {
   if (pb) pinC.write(GN_LOGIC_1);
   else pinC.write(GN_LOGIC_Z);
}

void encoder::turnleft(int pulses, bool pressbutton) {
   /* We start by raising the button, if requested. */
   if (pressbutton) press(true);

   /* If the last was a left turn, we need to do the direction change glitch.
    * Note that if the button is pressed, we only toggle pin B. */
   if (!lastwasright) {
      wait(speed, SC_MS);
      pinB.write(GN_LOGIC_1);
      wait(speed, SC_MS);
      pinB.write(GN_LOGIC_Z);
   }

   /* And we apply the pulses, again, watching for the button. */
   wait(phase, SC_MS);
   while(pulses > 0) {
      wait(speed-phase, SC_MS);
      pinA.write(GN_LOGIC_1);
      wait(phase, SC_MS);
      pinB.write(GN_LOGIC_1);
      wait(speed-phase, SC_MS);
      pinA.write(GN_LOGIC_Z);
      if (edges == 2) wait(phase, SC_MS);
      pinB.write(GN_LOGIC_Z);
      if (edges != 2) wait(phase, SC_MS);
      pulses = pulses - 1;
   }

   /* If the customer requested us to press the button with a turn, we release
    * it now.
    */
   if (pressbutton) {
      wait(speed, SC_MS);
      press(false);
   }

   /* And we tag that the last was a right turn as when we shift to the left
    * we can have some odd behaviour.
    */
   lastwasright = true;
}

void encoder::turnright(int pulses, bool pressbutton) {
   /* We start by raising the button, if requested. */
   if (pressbutton) press(true);

   /* If the last was a right turn, we need to do the direction change glitch.
    * Note that if the button is pressed, we only toggle pin A. */
   if (lastwasright) {
      wait(speed, SC_MS);
      pinA.write(GN_LOGIC_1);
      wait(speed, SC_MS);
      pinB.write(GN_LOGIC_1);
      wait(speed, SC_MS);
      pinB.write(GN_LOGIC_Z);
      wait(speed, SC_MS);
      pinA.write(GN_LOGIC_Z);
   }

   /* And we apply the pulses, again, watching for the button. */
   wait(phase, SC_MS);
   while(pulses > 0) {
      wait(speed-phase, SC_MS);
      pinB.write(GN_LOGIC_1);
      wait(phase, SC_MS);
      pinA.write(GN_LOGIC_1);
      wait(speed-phase, SC_MS);
      pinB.write(GN_LOGIC_Z);
      if (edges == 2) wait(phase, SC_MS);
      pinA.write(GN_LOGIC_Z);
      if (edges != 2) wait(phase, SC_MS);
      pulses = pulses - 1;
   }

   /* If the customer requested us to press the button with a turn, we release
    * it now.
    */
   if (pressbutton) {
      wait(speed, SC_MS);
      press(false);
   }

   /* And we tag that the last was a left turn, so we can do the left turn to
    * right turn glitch.
    */
   lastwasright = false;
}

void encoder::start_of_simulation() {
   pinA.write(GN_LOGIC_Z);
   pinB.write(GN_LOGIC_Z);
   pinC.write(GN_LOGIC_Z);
}

void encoder::trace(sc_trace_file *tf) {
   sc_trace(tf, pinA, pinA.name());
   sc_trace(tf, pinB, pinB.name());
   sc_trace(tf, pinC, pinC.name());
}
