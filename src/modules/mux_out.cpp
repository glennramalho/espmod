/*******************************************************************************
 * mux_out.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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
#include "soc/gpio_sig_map.h"
#include "mux_out.h"

void mux_out::mux(int funcsel) {
   if (function != funcsel) {
      function = funcsel;
      fchange_ev.notify();
   }
}

void mux_out::transfer() {
   /* These default to low. */
   min_o.write(false);
   men_o.write(false);

   while(true) {
      switch(function) {
         case U0TXD_OUT_IDX:
            min_o.write(uart0tx_i.read()); men_o.write(true);
            wait(fchange_ev | uart0tx_i.default_event());
            break;
         case U1TXD_OUT_IDX:
            min_o.write(uart1tx_i.read()); men_o.write(true);
            wait(fchange_ev | uart1tx_i.default_event());
            break;
         case U2TXD_OUT_IDX:
            min_o.write(uart2tx_i.read()); men_o.write(true);
            wait(fchange_ev | uart2tx_i.default_event());
            break;
         /* function 256 is logic 0 and function 257 is logic 1. This is
          * different from the spec, so the GPIO matrix selects the correct
          * one depending on the value to drive.
          */
         case MUXOUT_DRIVE_0: /* Function unknown and function 256 */
            min_o.write(false); men_o.write(true);
            wait(fchange_ev);
            break;
         case MUXOUT_DRIVE_1:
            min_o.write(true); men_o.write(true);
            wait(fchange_ev);
            break;
         default: /* Function unknown and function 256 */
            min_o.write(false); men_o.write(false);
            wait(fchange_ev);
            break;
      }
   }
}
