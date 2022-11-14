/*******************************************************************************
 * tft.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a test bench interface to check the results of the TFT eSPÌ
 *   library.
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

#ifndef _TFT_H
#define _TFT_H

#include <systemc.h>
#include "crccalc.h"
#include "gn_mixed.h"

struct tft_obj_t {
   unsigned int x1, y1, x2, y2, signature;
};

SC_MODULE(tftmod) {
   sc_inout<gn_mixed> cs {"cs"};
   sc_inout<gn_mixed> dc {"dc"};
   sc_inout<gn_mixed> rst {"rst"};
   sc_inout<gn_mixed> wr {"wr"};
   sc_inout<gn_mixed> rd {"rd"};
   sc_inout<gn_mixed> d0 {"d0"};
   sc_inout<gn_mixed> d1 {"d1"};
   sc_inout<gn_mixed> d2 {"d2"};
   sc_inout<gn_mixed> d3 {"d3"};
   sc_inout<gn_mixed> d4 {"d4"};
   sc_inout<gn_mixed> d5 {"d5"};
   sc_inout<gn_mixed> d6 {"d6"};
   sc_inout<gn_mixed> d7 {"d7"};

   sc_signal<int> istate{"state"};
   sc_signal<int> icmd{"cmd"};
   sc_signal<unsigned int> startcol {"startcol"}, endcol {"endcol"};
   sc_signal<unsigned int> startrow {"startrow"}, endrow {"endrow"};
   sc_signal<unsigned int> signature {"signature"};
   sc_signal<unsigned int> explength {"explength"};
   sc_signal<unsigned int> llen {"llen"};
   sc_signal<bool> sleep {"sleep", true};

   /* Threads */
   void write(void);
   void read(void);

   /* Helper Functions */
   void drive(unsigned int val);
   void trace(sc_trace_file *tf);
   void waitcode(tft_obj_t *t);
   void setfgcolor(unsigned int v);
   void set_debug(int lvl) { debug = lvl; }

   SC_CTOR(tftmod) {
      debug = 0;

      SC_THREAD(write);
      sensitive << wr;

      crctable_init();
      setfgcolor(0x0);
   }

   private:
   uint8_t fglow, fghigh;
   int debug;
};

#endif
