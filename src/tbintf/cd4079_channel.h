/*******************************************************************************
 * cd4079_channel.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a simple model for the CD4079 Analog Mux.
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

#ifndef _CD4079_H
#define _CD4079_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(cd4079_channel) {
   sc_in<bool> inh {"inh"};
   sc_in<bool> a {"a"};
   sc_in<bool> b {"b"};
   sc_in<bool> c {"c"};
   sc_port<sc_signal_inout_if<gn_mixed>,8> channel;
   sc_inout<gn_mixed> x {"x"};

   private:
   int sel;

   public:
   /* Tasks */
   void process_th();

   SC_CTOR(cd4079_channel) {
      SC_THREAD(process_th);
   }

   void trace(sc_trace_file *tf);
};

#endif