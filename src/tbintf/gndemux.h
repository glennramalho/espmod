/*******************************************************************************
 * gndemux.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple gnmuxin for GN logic use in testbenches.
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

#ifndef _GNDEMUX_H
#define _GNDEMUX_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(gndemux) {
   sc_port< sc_signal_out_if<gn_mixed>,0 > pin {"pin"};
   sc_in<int> pinS {"pinS"};
   sc_in<gn_mixed> pinY {"pinY"};

   void drivepin_th();

   gndemux (sc_module_name name, gn_mixed _def = GN_LOGIC_Z) {
     def = _def;
     SC_THREAD(drivepin_th);
   }

   SC_HAS_PROCESS(gndemux);


   protected:
   gn_mixed def;
};

#endif
