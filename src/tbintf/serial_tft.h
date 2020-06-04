/*******************************************************************************
 * serial_tft.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple model for a ST7735 based screen module.
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

#ifndef _SERIAL_TFT
#define _SERIAL_TFT

#include <systemc.h>
#include "gn_mixed.h"
#include "st7735.h"

SC_MODULE(serial_tft) {
   sc_in<gn_mixed> sck {"sck"};
   sc_inout<gn_mixed> sda {"sda"};
   sc_in<gn_mixed> res {"res"};
   sc_in<gn_mixed> rs {"rs"};
   sc_in<gn_mixed> cs {"cs"};

   st7735 i_controller{"i_controller"};
   gn_tie logic_0 {"logic_0", GN_LOGIC_0};
   gn_tie logic_1 {"logic_0", GN_LOGIC_1};
   sc_signal<gn_mixed> osc {"osc"};
   sc_signal<gn_mixed> te {"te"};
   sc_signal<sc_bv<3> > im {"im", 7};

   SC_CTOR(serial_tft) {
      i_controller.scl_dcx(sck);
      i_controller.sda(sda);
      i_controller.csx(cs);
      i_controller.resx(res);
      i_controller.wrx(rs);
      i_controller.im(im);
      i_controller.spi4w(logic_1);
      i_controller.rdx(logic_0);
      i_controller.d0(logic_0);
      i_controller.d1(logic_0);
      i_controller.d2(logic_0);
      i_controller.d3(logic_0);
      i_controller.d4(logic_0);
      i_controller.d5(logic_0);
      i_controller.d6(logic_0);
      i_controller.d7(logic_0);
      i_controller.d8(logic_0);
      i_controller.d9(logic_0);
      i_controller.d10(logic_0);
      i_controller.d11(logic_0);
      i_controller.d12(logic_0);
      i_controller.d13(logic_0);
      i_controller.d14(logic_0);
      i_controller.d15(logic_0);
      i_controller.d16(logic_0);
      i_controller.d17(logic_0);
      i_controller.te(te);
      i_controller.osc(osc);
   }

   void trace(sc_trace_file *tf) {
      trace(tf, sck, sck.name());
      trace(tf, sda, sda.name());
      trace(tf, res, res.name());
      trace(tf, rs, rs.name());
      trace(tf, cs, cs.name());
      i_controller.trace(tf);
   }
};

#endif
