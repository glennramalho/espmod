/*******************************************************************************
 * netcon.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Simple block that connects two sc_signals, one being bool and one being
 *   sc_signal_resolved.
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

#ifndef _NETCON_H
#define _NETCON_H

#include <systemc.h>
#include "gn_mixed.h"

static struct {
   template < typename T >
   operator sc_core::sc_signal_inout_if<T> & () const {
     return
        *(new sc_core::sc_signal<T>(sc_core::sc_gen_unique_name("sig_open")));
   }
} const sig_open = {};

SC_MODULE(netcon_rvtobool) {
   sc_in_resolved a;
   sc_out<bool> b;

   void transport();

   SC_CTOR(netcon_rvtobool): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

SC_MODULE(netcon_booltorv) {
   sc_in<bool> a;
   sc_out_resolved b;

   void transport();

   SC_CTOR(netcon_booltorv): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

SC_MODULE(netcon_mixtobool) {
   sc_in<gn_mixed> a;
   sc_out<bool> b;

   void transport();

   SC_CTOR(netcon_mixtobool): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

SC_MODULE(netcon_booltomix) {
   sc_in<bool> a;
   sc_out<gn_mixed> b;

   void transport();

   SC_CTOR(netcon_booltomix): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

SC_MODULE(netcon_mixtoana) {
   sc_in<gn_mixed> a;
   sc_out<float> b;

   void transport();

   SC_CTOR(netcon_mixtoana): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

SC_MODULE(netcon_anatomix) {
   sc_in<float> a;
   sc_out<gn_mixed> b;

   void transport();

   SC_CTOR(netcon_anatomix): a("a"), b("b") {
      SC_THREAD(transport);
      sensitive << a;
   }
};

#endif
