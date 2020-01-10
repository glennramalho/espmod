/*******************************************************************************
 * gn_pullupdn.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Creates two weak drivers for the GN type.
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

#ifndef _GN_PULLUPDN_H
#define _GN_PULLUPDN_H

#include <systemc.h>
#include "gn_mixed.h"

SC_MODULE(gn_pullup) {
   sc_inout<gn_mixed> o {"o"};
   SC_CTOR(gn_pullup) {
   }
   void start_of_simulation();
};

SC_MODULE(gn_pulldn) {
   sc_inout<gn_mixed> o {"o"};
   SC_CTOR(gn_pulldn) {
   }
   void start_of_simulation();
};

#endif
