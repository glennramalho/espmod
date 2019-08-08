/*******************************************************************************
 * uart.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "uart.h"

void uart::intake() {
   char buffer[100];
   int cnt;
   unsigned char msg;
   unsigned char pos;
   bool incomming;

   /* First we wait for the signal to rise. */
   while (rx.read() != true) wait();

   /* Now we can listen for a packet. */
   while(true) {
      /* We wait for the RX to go low. */
      wait();
      if (rx.read() != false) continue;

      /* In the real circuit we should do a larger delay to make sure clock
       * jitter and other errors do not kill us. In this simulation
       * environment though, it is ok to wait a short time as we know
       * the clocks are all lined up perfectly.
       */
      wait(125, SC_NS);
      /* And we advance to the next bit. */
      wait(baudperiod);
      /* And we take one bit at a time and merge them together. */
      msg = 0;
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         /*printf("[%s/%d]: r %s\n", name(), cnt,
            sc_time_stamp().to_string().c_str()); */
         incomming = rx.read();
         if (incomming == true) msg = msg | pos;
         else if (incomming != false) {
            snprintf(buffer, 100, "Got an '%c' on %s",
               incomming?('1'):('0'), name());
            SC_REPORT_WARNING("UART", buffer);
         }
         wait(baudperiod);
      }
      /* And we send the char. If the buffer is filled, we discard it and
       * warn the user. If it is free, then we take it. This is needed as
       * the sender has no way to know if the buffer has space or not.
       */
      if (from.num_free() == 0) {
         snprintf(buffer, 100, "Buffer overflow on UART %s", name());
         SC_REPORT_WARNING("UART", buffer);
      }
      else {
         from.write(msg);
         /*printf("[%s] received-%c/%x @ %s\n", name(), msg, msg,
            sc_time_stamp().to_string().c_str()); */
      }
   }
}

void uart::outtake() {
   int cnt;
   unsigned char msg;
   unsigned char pos;
   tx.write(true);
   while(true) {
      /* We block until we receive something to send. */
      msg = to.read();
      /*printf("[%s] sending-%c/%x @ %s\n", name(), msg, msg,
         sc_time_stamp().to_string().c_str()); */
      /* Then we send the packet asynchronously. */
      tx.write(false);
      wait(baudperiod);
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         /*printf("[%s/%d]: s %s\n", name(), cnt,
            sc_time_stamp().to_string().c_str());*/
         if ((pos & msg)>0) tx.write(true);
         else tx.write(false);
         wait(baudperiod);
      }
      /* And we send the stop bit. */
      tx.write(true);
      wait(baudperiod);
   }
}

void uart::set_baud(unsigned int rate) {
   baudperiod = sc_time(8680, SC_NS) * (115200 / rate);
}
