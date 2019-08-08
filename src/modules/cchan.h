/*******************************************************************************
 * cchan.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC module for a character wide UART like protocol.
 *   This is intended mainly as a way to simplify more complex communication
 *   protocols and does not represent any existing protocol.
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

#ifndef _CCHAN_H
#define _CCHAN_H

#include <systemc.h>

SC_MODULE(cchan) {
   sc_in<unsigned int> rx;
   sc_out<unsigned int> tx;

   sc_fifo<unsigned char> from;
   sc_fifo<unsigned char> to;

   void intake();
   void outtake();

   sc_time baudperiod;
   void set_baud(unsigned int baudrate);

   cchan(sc_module_name name, int tx_buffer_size, int rx_buffer_size):
         rx("rx"), tx("tx"),
         from("fromfifo", rx_buffer_size), to("tofifo", tx_buffer_size) {
      SC_THREAD(intake);
      sensitive << rx;
      SC_THREAD(outtake); /* Active when something is in the fifo. */
      set_baud(2000000); /* The default is 2MHz. */
   }
   SC_HAS_PROCESS(cchan);
};

#endif
