/*******************************************************************************
 * cd4097_channel.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple model for a single channel of a CD4079 dual analog Mux.
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

#ifndef _CD4097_H
#define _CD4097_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(cd4097_channel) {
   sc_in<bool> inh {"inh"};
   sc_in<bool> a {"a"};
   sc_in<bool> b {"b"};
   sc_in<bool> c {"c"};
   sc_port<sc_signal_inout_if<gn_mixed>,8> channel;
   sc_inout<gn_mixed> x {"x"};

   private:
   int sel;
   bool debug;

   public:
   /* Tasks */
   void process_th();

   /* Functions */
   void set_debug(bool _d) { debug = _d; }
   bool get_debug() { return debug; }
   int get_selector();

   SC_CTOR(cd4097_channel) {
      sel = 0;
      debug = false;
      SC_THREAD(process_th);
   }

   void trace(sc_trace_file *tf);
};

#endif
