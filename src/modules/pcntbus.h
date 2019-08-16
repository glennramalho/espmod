/*******************************************************************************
 * pcntbus.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Defines the format of the PCNT bus. This is a simple encapsulation bus to
 * gather all signals related to one channel and make the top look a bit
 * simpler.
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

#ifndef _PCNTBUS_H
#define _PCNTBUS_H

#include <systemc.h>

struct pcntbus_t {
   bool sig_ch0;
   bool sig_ch1;
   bool ctrl_ch0;
   bool ctrl_ch1;
};

inline bool operator==(const pcntbus_t &a, const pcntbus_t &b) {
   if (a.sig_ch0 != b.sig_ch0) return false;
   if (a.sig_ch1 != b.sig_ch1) return false;
   if (a.ctrl_ch0 != b.ctrl_ch0) return false;
   if (a.ctrl_ch1 != b.ctrl_ch1) return false;
   return true;
}
inline std::ostream &operator<<(std::ostream &os, const pcntbus_t &a) {
   os << "ch0(" << a.sig_ch0 << "/" << a.sig_ch1 << ") ch1(" << a.ctrl_ch0
      << "/" << a.ctrl_ch1 << ")";
   return os;
}

inline bool operator!=(const pcntbus_t &a, const pcntbus_t &b) {
   return !operator==(a,b);
}

inline void sc_trace(sc_trace_file *tf, const pcntbus_t &object,
      const std::string &name) {
   sc_trace(tf, object.sig_ch0, name + "_sigch0");
   sc_trace(tf, object.sig_ch1, name + "_sigch0");
   sc_trace(tf, object.ctrl_ch0, name + "_sigch0");
   sc_trace(tf, object.ctrl_ch1, name + "_sigch0");
}

#endif
