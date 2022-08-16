/*******************************************************************************
 * doitesp32devkitv1_bt.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of the doitESP32devkitV1 board with BlueTooth.
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

#ifndef _DOITESP32DEVKITV1_BT_H
#define _DOITESP32DEVKITV1_BT_H

#include <systemc.h>
#include "doitesp32devkitv1.h"
#include "btmod.h"

class doitesp32devkitv1_bt : public doitesp32devkitv1 {
   public:
   /* Pins */
   sc_in<unsigned int> brx {"brx"};
   sc_out<unsigned int> btx {"btx"};

   /* Modules */
   btmod i_bt {"i_bt", 128, 128};

   // Constructor
   doitesp32devkitv1_bt(sc_module_name name): doitesp32devkitv1(name) {
      /* Pin Hookups */
      i_bt.rx(brx);
      i_bt.tx(btx);
   }
   SC_HAS_PROCESS(doitesp32devkitv1_bt);

   void pininit();
   void trace(sc_trace_file *tf);
};

#endif
