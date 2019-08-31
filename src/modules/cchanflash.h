/*******************************************************************************
 * cchanflash.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC module for an integer wide UART protocol. It should
 *   eventually be replaced with a real QSPI interface, but for now this works
 *   for the ESPMOD project.
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

#ifndef _QSPIFLASH_H
#define _QSPIFLASH_H

#include <systemc.h>
#include "info.h"
#include "cchan.h"

enum secstate_t {UNK, ERS, PROG};

SC_MODULE(cchanflash) {
   /* Signals */
   sc_in<unsigned int> rx {"rx"};
   sc_out<unsigned int> tx {"tx"};

   /* Receiving Uarts */
   cchan i_uflash {"i_uflash", 256+4, 256+4};

   /* Processes */
   void flash(void);

   /* Methods */
   /* Usefull address to check if an address is valid. It returns true if it
    * is or false if it is not.
    */
   bool checkaddr(unsigned int addr);
   int getrange(unsigned int addr);
   /* For preloading something into the memory */
   bool preerase(unsigned int addr, unsigned int end);
   bool preload(unsigned int addr, void* data, unsigned int size);
   void addrange(unsigned int _rangestart, unsigned int _rangeend);
   void rangeinit();

   // Constructor
   SC_CTOR(cchanflash) {
      secs = NULL;
      pgm = NULL;

      i_uflash.rx(rx); i_uflash.tx(tx);
      SC_THREAD(flash);
   }

   private:

   /* Flash size */
   std::vector<unsigned int> rangestart;
   std::vector<unsigned int> rangeend;
   std::vector<unsigned int> secstart;

   /* We need a list of sectors so that we know what sectors have been
    * programmed, erased, or not yet used.
    */
   secstate_t *secs;
   std::vector<int> *pgm;
};

#endif
