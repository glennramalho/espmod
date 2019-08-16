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
   bool lin, len;

   /* These default to low. */
   min_o.write(false);
   men_o.write(false);

   while(true) {
      switch(function) {
         case U0TXD_OUT_IDX:
            wait(fchange_ev | uart0tx_i.default_event());
            lin = uart0tx_i.read(); len = true;
            break;
         case U1TXD_OUT_IDX:
            wait(fchange_ev | uart1tx_i.default_event());
            lin = uart1tx_i.read(); len = true;
            break;
         case U2TXD_OUT_IDX:
            wait(fchange_ev | uart2tx_i.default_event());
            lin = uart2tx_i.read(); len = true;
            break;
         default:
            wait(fchange_ev); lin = false; len = true; break;;
      }

      /* Not all signals have an output enable. These always output high. */
      min_o.write(lin);
      men_o.write(len);
   }
}
