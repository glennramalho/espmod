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

void i2c::transfer_th() {
   unsigned char p;
   bool lastbit;

   sda_en_o.write(false);
   scl_en_o.write(false);
   state = IDLE;

   while(true) {
      p = to.read();
      if (p == 'S') {
         wait(625, SC_NS); sda_en_o.write(true);
         wait(625, SC_NS); scl_en_o.write(true);
         wait(1250, SC_NS);
         state = DEVID;
      }
      else if (p == 'P') {
         wait(1250, SC_NS); scl_en_o.write(false);
         wait(625, SC_NS); sda_en_o.write(true);
         wait(1250, SC_NS);
         state = IDLE;
      }
      else if (state == DEVID || state == WRITING) {
         if (p == '1') {
            wait(625, SC_NS); sda_en_o.write(false);
            wait(625, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
            lastbit = true;
         }
         else if (p == '0') {
            wait(1250, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
            lastbit = false;
         }
         else if (p == 'Z') {
            wait(1250, SC_NS); scl_en_o.write(false);
            if (sda_i.read()) {
               from.write('N');
               state = IDLE;
            }
            else {
               from.write('A');
               if (lastbit || state == WRITING) state = WRITING;
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
            wait(1250, SC_NS); scl_en_o.write(false);
            wait(1250, SC_NS); scl_en_o.write(true);
         }
         else if (p == 'Z') {
            wait(1250, SC_NS); scl_en_o.write(false);
            if (sda_i.read()) from.write('1');
            else from.write('0');
            wait(1250, SC_NS); scl_en_o.write(true);
         }
         else wait(2500, SC_NS);
      }
   }
}
