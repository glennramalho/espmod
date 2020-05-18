/*******************************************************************************
 * uart.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Model for a UART.
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

#ifndef _UART_H
#define _UART_H

#include <systemc.h>

SC_MODULE(uart) {
   sc_in<bool> rx {"rx"};
   sc_out<bool> tx {"tx"};

   sc_fifo<unsigned char> from;
   sc_fifo<unsigned char> to;

   void intake();
   void outtake();

   private:
   sc_time baudperiod;
   int baudrate;
   bool debug;
   bool autodetect;
   int stopbits; /* 0: invalid, 1: one bit, 2: 1.5 bits, 3: 2 bits */

   public:
   void set_baud(unsigned int baudrate);
   void set_debug(bool on) { debug = on; }
   bool get_debug() { return debug; }
   bool get_baud() { return baudrate; }
   void set_stop(int _sp) { stopbits = _sp; }
   int get_stop() { return stopbits; }

   /* This enables the autodetect. It will take the first message and discard
    * it. Only the start bit will be used.
    */
   void setautodetect() { autodetect = true; }
   int getautorate();

   uart(sc_module_name name, int tx_buffer_size, int rx_buffer_size):
         rx("rx"), tx("tx"),
         from("fromfifo", rx_buffer_size), to("tofifo", tx_buffer_size) {
      SC_THREAD(intake);
      sensitive << rx;
      SC_THREAD(outtake); /* Active when something is in the fifo. */
      set_baud(115200); /* The default is 115200. */

      autodetect = false;
      debug = false;
      stopbits = 1;
   }
   SC_HAS_PROCESS(uart);
};

#endif
