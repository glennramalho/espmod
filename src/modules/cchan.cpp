/*******************************************************************************
 * cchan.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "cchan.h"

void cchan::intake() {
   char buffer[100];
   unsigned char msg;

   /* First we wait for the signal to go to idle. */
   while (rx.read() != 0xffU) wait();

   /* Now we can listen for a packet. */
   while(true) {
      /* We wait for the message to come in. */
      wait();
      /* And we get the message. */
      msg = (unsigned char)(rx.read() & 0xff);

      if (from.num_free() == 0) {
         snprintf(buffer, 100, "Buffer overflow on %s", name());
         SC_REPORT_WARNING("CCHAN", buffer);
      }
      else {
         from.write(msg);
         /*printf("[%s] received-%c/%x @ %s\n", name(), msg, msg,
            sc_time_stamp().to_string().c_str());*/
      }
   }
}

void cchan::outtake() {
   bool clock;
   unsigned char msg;
   tx.write(0xff);
   clock = false;
   while(true) {
      /* We block until we receive something to send. */
      msg = to.read();
      /*printf("[%s] sending-%c/%x @ %s\n", name(), msg, msg,
         sc_time_stamp().to_string().c_str());*/
      /* Then we send the packet asynchronously. We always invert the clock
       * bit. */
      if (!clock) tx.write(0x100U + (unsigned int)msg);
      else tx.write((unsigned int)msg);
      clock = !clock;
      wait(baudperiod);
   }
}

void cchan::set_baud(unsigned int rate) {
   baudperiod = sc_time(25, SC_NS) * (4000000 / rate);
}
