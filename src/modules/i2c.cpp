/*******************************************************************************
 * i2c.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Model for a I2C.
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
#include "i2c.h"
#include "info.h"

void i2c::transfer_th() {
   unsigned char p;
   bool rwbit;

   sda_en_o.write(false);
   scl_en_o.write(false);
   state = IDLE;

   while(true) {
      p = to.read();
      snd.write(p);
      /* For the start and stop bit, we could be in the middle of a command,
       * therefore we cannot assume the bits are correct.
       */
      if (p == 'S') {
         sda_en_o.write(false);
         scl_en_o.write(false);
         wait(625, SC_NS); sda_en_o.write(true);
         wait(625, SC_NS); scl_en_o.write(true);
         wait(1250, SC_NS);
         state = DEVID;
      }
      else if (p == 'P') {
         /* If the SDA is high, we need to take it low first. If not, we cam
          * go straight into the stop bit.
          */
         wait(625, SC_NS); sda_en_o.write(true);
         wait(625, SC_NS); scl_en_o.write(false);
         wait(625, SC_NS); sda_en_o.write(false);
         wait(1250, SC_NS);
         state = IDLE;
      }
      else if (state == DEVID || state == WRITING) {
         if (p == '1') {
            wait(625, SC_NS); sda_en_o.write(false);
            wait(625, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
            rwbit = true;
         }
         else if (p == '0') {
            wait(625, SC_NS); sda_en_o.write(true);
            wait(625, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
            rwbit = false;
         }
         else if (p == 'Z') {
            sda_en_o.write(false);
            /* Then we tick the clock. */
            wait(1250, SC_NS); scl_en_o.write(false);
            /* After the clock, we sample it. */
            if (sda_i.read()) {
               snd.write('N');
               from.write('N');
               state = IDLE;
            }
            else {
               snd.write('A');
               from.write('A');
               /* If the readwrite bit is low and we are in the DEVID state
                * we go to the WRITING state. If we are already in the WRITING
                * state we remain in it.
                */
               if (state == DEVID && !rwbit || state == WRITING) state= WRITING;
               else state = READING;
            }
            wait(1250, SC_NS); scl_en_o.write(true);
         }
         else wait(2500, SC_NS);
      }
      else if (state == READING) {
         if (p == 'N') {
            wait(625, SC_NS); sda_en_o.write(false);
            wait(625, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
            state = IDLE;
         }
         else if (p == 'A') {
            wait(625, SC_NS); sda_en_o.write(true);
            wait(625, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
         }
         else if (p == 'Z') {
            sda_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(false);
            if (sda_i.read()) {
               from.write('1');
               snd.write('1');
            }
            else {
               from.write('0');
               snd.write('0');
            }
            wait(1250, SC_NS); scl_en_o.write(true);
         }
         else wait(2500, SC_NS);
      }
   }
}

void i2c::trace(sc_trace_file *tf) {
   sc_trace(tf, snd, snd.name());
}
