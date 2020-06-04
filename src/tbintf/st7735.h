/*******************************************************************************
 * st7735.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple model for the ST7735 display controller model.
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

#ifndef _ST7735_H
#define _ST7735_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(st7735) {
   sc_inout<gn_mixed> sda {"sda"};
   sc_inout<gn_mixed> d0 {"d0"};
   sc_inout<gn_mixed> d1 {"d1"};
   sc_inout<gn_mixed> d2 {"d2"};
   sc_inout<gn_mixed> d3 {"d3"};
   sc_inout<gn_mixed> d4 {"d4"};
   sc_inout<gn_mixed> d5 {"d5"};
   sc_inout<gn_mixed> d6 {"d6"};
   sc_inout<gn_mixed> d7 {"d7"};
   sc_inout<gn_mixed> d8 {"d8"};
   sc_inout<gn_mixed> d9 {"d9"};
   sc_inout<gn_mixed> d10 {"d10"};
   sc_inout<gn_mixed> d11 {"d11"};
   sc_inout<gn_mixed> d12 {"d12"};
   sc_inout<gn_mixed> d13 {"d13"};
   sc_inout<gn_mixed> d14 {"d14"};
   sc_inout<gn_mixed> d15 {"d15"};
   sc_inout<gn_mixed> d16 {"d16"};
   sc_inout<gn_mixed> d17 {"d17"};
   sc_in<gn_mixed> wrx {"wrx"};
   sc_in<gn_mixed> rdx {"rdx"};
   sc_in<gn_mixed> resx {"resx"};
   sc_in<gn_mixed> csx {"csx"};
   sc_in<gn_mixed> scl_dcx {"scl_dcx"};
   sc_out<gn_mixed> te {"te"};
   sc_out<gn_mixed> osc {"osc"};
   sc_in<gn_mixed> spi4w {"spi4w"};
   sc_in<sc_bv<3> > im {"im"};

   private:
   bool readmode;
   bool initialized;
   int collected;
   int pos;

   public:
   void collect();

   SC_CTOR(st7735) {
      initialized = false;
      readmode = false;

      SC_THREAD(collect);
      sensitive << csx << scl_dcx << resx << wrx;
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};

#endif
