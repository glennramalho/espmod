/*******************************************************************************
 * st7735.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a testbench module to emulate the ST7735 display controller.
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
#include "st7735.h"

void st7735::collect() {
   pos = 0;
   collected = 0;
   while(1) {
      wait();

      /* We check the reset. */
      if (!resx.read().islogic()) {
         PRINTF_WARN("ST7735", "RESX is non-logic value %c", resx.read().to_char());
         continue;
      }
      else if (resx.read().islow()) {
         pos = 0;
         collected = 0;
         initialized = true;
         continue;
      }

      // If the CSX is not low, we ignore what came in.
      if (!csx.read().islogic()) {
         PRINTF_WARN("ST7735", "CSX is non-logic value %c", csx.read().to_char());
         continue;
      }
      else if (csx.read().ishigh()) continue;

      // To use the controller, it needs to have been reset.
      if (!initialized) {
         PRINTF_WARN("ST7735", "Module has not been reset");
         continue;
      }

      // Now we check the mode to see what came in. This depends on the mode
      // selection.
      switch(im.read().to_uint()) {
         // SPI mode -- we only react to a chane in the SCL pin.
         default:
            /* We first discard the illegal values. */
            if (!scl_dcx.read().islogic()) {
               PRINTF_WARN("ST7735", "SCL is non-logic value %c",
                     scl_dcx.read().to_char());
               continue;
            }

            /* If we are in readmode, we return the value. */
            if (readmode) {
               if (scl_dcx.read().ishigh()) sda.write(GN_LOGIC_Z);
               else if (pos > 0)
                  sda.write(((collected & (1 << (pos-1)))>0)?GN_LOGIC_1:GN_LOGIC_0);
               continue;
            }

            /* If it is not a read it is a write. We sample on the rising edge. */
            if (scl_dcx.read() != SC_LOGIC_1) continue;

            /* We increment the counter. If we are using the 3wire interface, we
             * need to get the DCX first
             */
            if (!spi4w.read().islogic()) {
               PRINTF_WARN("ST7735",
                     "Tried to latch while spi4w is non-logic value %c",
                     spi4w.read().to_char());
               continue;
            }
            else if (wrx.read().ishigh()) collected = collected | 0x100;
            if (pos <= 0 && spi4w.read().islow()) {
               collected = 0;
               pos = 9;
            }
            else if (pos <= 0) {
               collected = 0;
               pos = 8;
            }
            else pos = pos - 1;

            if (spi4w.read().ishigh()) {
               if (!wrx.read().islogic()) {
                  PRINTF_WARN("ST7735", "Tried to latch non-logic WRX value %c",
                        wrx.read().to_char());
               }
               else if (wrx.read().ishigh()) collected = collected | 0x100;
               else collected = collected & 0x0ff;
            }

            /* If the pin is non-logic we complain and skip it. */

            if(!sda.read().islogic()) {
               PRINTF_WARN("ST7735", "Tried to latch non-logic SDA value %c",
                     sda.read().to_char());
            }
            /* If the logic is valid, we take in the ones. */
            else if (sda.read().ishigh()) collected = collected | (1 << (pos-1));

            if (pos == 1) {
               printf("%s: received %03x\n", sc_time_stamp().to_string().c_str(),
                     collected);
            }
            break;
      }
   }
}

void st7735::start_of_simulation() {
   sda.write(GN_LOGIC_Z);
   osc.write(GN_LOGIC_0);
   te.write(GN_LOGIC_0);
   d0.write(GN_LOGIC_Z);
   d1.write(GN_LOGIC_Z);
   d2.write(GN_LOGIC_Z);
   d3.write(GN_LOGIC_Z);
   d4.write(GN_LOGIC_Z);
   d5.write(GN_LOGIC_Z);
   d6.write(GN_LOGIC_Z);
   d7.write(GN_LOGIC_Z);
   d8.write(GN_LOGIC_Z);
   d9.write(GN_LOGIC_Z);
   d10.write(GN_LOGIC_Z);
   d11.write(GN_LOGIC_Z);
   d12.write(GN_LOGIC_Z);
   d13.write(GN_LOGIC_Z);
   d14.write(GN_LOGIC_Z);
   d15.write(GN_LOGIC_Z);
   d16.write(GN_LOGIC_Z);
   d17.write(GN_LOGIC_Z);
}

void st7735::trace(sc_trace_file *tf) {
   sc_trace(tf, sda, sda.name());
   sc_trace(tf, d0, d0.name());
   sc_trace(tf, d1, d1.name());
   sc_trace(tf, d2, d2.name());
   sc_trace(tf, d3, d3.name());
   sc_trace(tf, d4, d4.name());
   sc_trace(tf, d5, d5.name());
   sc_trace(tf, d6, d6.name());
   sc_trace(tf, d7, d7.name());
   sc_trace(tf, d8, d8.name());
   sc_trace(tf, d9, d9.name());
   sc_trace(tf, d10, d10.name());
   sc_trace(tf, d11, d11.name());
   sc_trace(tf, d12, d12.name());
   sc_trace(tf, d13, d13.name());
   sc_trace(tf, d14, d14.name());
   sc_trace(tf, d15, d15.name());
   sc_trace(tf, d16, d16.name());
   sc_trace(tf, d17, d17.name());
   sc_trace(tf, wrx, wrx.name());
   sc_trace(tf, rdx, rdx.name());
   sc_trace(tf, csx, csx.name());
   sc_trace(tf, scl_dcx, scl_dcx.name());
   sc_trace(tf, spi4w, spi4w.name());
   sc_trace(tf, im, im.name());
}
