/*******************************************************************************
 * clkgen.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Emulates the clocks on the ESP32.
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

#ifndef _CLKGEN_H
#define _CLKGEN_H

#include <systemc.h>

SC_MODULE(clkgen) {
   sc_out<bool> apb_clk;

   /* Threads */
   void gen(void);

   /* Sets initial drive condition. */
   SC_CTOR(clkgen) {
      SC_THREAD(gen);
   }
};

#endif
