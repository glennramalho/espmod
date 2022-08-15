/*******************************************************************************
 * pn532_hsu.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a crude model for the PN532 with firmware v1.6. It will work for
 *   basic work with a PN532 system. This is the HSU version.
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
#include "pn532_hsu.h"
#include "info.h"

void pn532_hsu::rx_th(void) {
   int cnt;
   unsigned char msg;
   unsigned char pos;
   gn_mixed intok;
   bool incomming;

   /* First we wait for the signal to rise. */
   while (!rx.read().ishigh()) wait(rx.value_changed_event());

   /* Now we can listen for a packet. */
   while(true) {
      /* We wait for the RX to go low. */
      wait(rx.value_changed_event());
      if (!rx.read().islow()) continue;

      /* We jump halfway into the start pulse. */
      wait(baudperiod/2);

      /* And we advance to the next bit. */
      wait(baudperiod);

      /* And we take one bit at a time and merge them together. */
      msg = 0;
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         intok = rx.read();
         if (intok != GN_LOGIC_1 && intok != GN_LOGIC_0) {
            PRINTF_WARN("PN532", "[%s/%d]: received lvl %c", name(), cnt,
               intok.to_char());
            incomming = false;
         }
         else {
            if (intok == GN_LOGIC_1) incomming = true;
            else incomming = false;
            if (debug) {
               PRINTF_INFO("PN532", "[%s/%d]: received lvl %c", name(), cnt,
                  (incomming)?'h':'l');
            }
         }
         if (incomming == true) msg = msg | pos;
         wait(baudperiod);
      }

      /* Now that we have a char, we push it to the processing unit. */
      if (from.num_free() == 0) {
         PRINTF_WARN("PN532", "Buffer overflow on PN532 %s", name());
      }
      else {
         from.write(msg);
      }
   }
}

void pn532_hsu::tx_th() {
   int cnt;
   unsigned char msg;
   unsigned char pos;
   tx.write(GN_LOGIC_1);
   while(true) {
      /* We block until we receive something to send. */
      msg = to.read();
      outtoken.write(msg);
      if (debug && isprint(msg)) {
         PRINTF_INFO("PN532","[%s] sending-'%c'/%02x", name(), msg, msg);
      }
      else if (debug) {
         PRINTF_INFO("PN532","[%s] sending-%02x", name(), msg);
      }
      /* Then we send the packet asynchronously. */
      tx.write(GN_LOGIC_0);
      wait(baudperiod);
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         if(debug) {
            PRINTF_INFO("PN532", "[%s/%d]: sent '%c'", name(), cnt,
               ((pos & msg)>0)?'h':'l');
         }
         if ((pos & msg)>0) tx.write(GN_LOGIC_1);
         else tx.write(GN_LOGIC_0);
         wait(baudperiod);
      }
      /* And we send the stop bit. */
      tx.write(GN_LOGIC_1);
      if (stopbits < 2) wait(baudperiod);
      else if (stopbits == 2) wait(baudperiod + baudperiod / 2);
      else wait(baudperiod * 2);
   }
}

void pn532_hsu::set_baud(unsigned int rate) {
   baudrate = rate;
   baudperiod = sc_time(8680, SC_NS) * (115200 / rate);
}
