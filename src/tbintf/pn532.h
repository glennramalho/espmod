/*******************************************************************************
 * pn532.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a crude model for the PN532 with firmware v1.6. It will work for
 *   basic work with a PN532 system.
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

#ifndef _PN532_H
#define _PN532_H

#include <systemc.h>
#include "gn_mixed.h"
#include "pn532_base.h"

struct pn532 : public pn532_base {
   /* Signals */
   sc_inout<gn_mixed> sda {"sda"};
   sc_inout<gn_mixed> scl {"scl"};
   sc_signal<int> icstate {"icstate"};

   /* Functions */
   void trace(sc_trace_file *tf);
   /* Processes */
   void i2c_th(void);

   // Constructor
   pn532(sc_module_name name): pn532_base(name) {
      SC_THREAD(i2c_th);
   }

   SC_HAS_PROCESS(pn532);
};

#endif
