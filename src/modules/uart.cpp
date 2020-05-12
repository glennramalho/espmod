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
#include "info.h"

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

      /* If we are in autodetect mode, we will keep looking for a rise. */
      if (autodetect) {
         sc_time start = sc_time_stamp();
         wait(sc_time(2, SC_MS), rx.value_changed_event());
         sc_time delta = start - sc_time_stamp();
         if (delta >= sc_time(2, SC_MS)) {
            PRINTF_WARN("UART", "%s: autodetect timed out", name());
            set_baud(300);
         }
         else if (delta >= sc_time(3, SC_MS)) set_baud(300);
         else if (delta >= sc_time(1, SC_MS)) set_baud(600);
         else if (delta >= sc_time(800, SC_US)) set_baud(1200);
         else if (delta >= sc_time(400, SC_US)) set_baud(2400);
         else if (delta >= sc_time(200, SC_US)) set_baud(4800);
         else if (delta >= sc_time(100, SC_US)) set_baud(9600);
         else if (delta >= sc_time(50, SC_US)) set_baud(19200);
         else if (delta >= sc_time(20, SC_US)) set_baud(38400);
         else if (delta >= sc_time(15, SC_US)) set_baud(57600);
         else if (delta >= sc_time(12, SC_US)) set_baud(74880);
         else if (delta >= sc_time(7, SC_US)) set_baud(115200);
         else if (delta >= sc_time(4, SC_US)) set_baud(230400);
         else if (delta >= sc_time(3, SC_US)) set_baud(256000);
         else if (delta >= sc_time(2, SC_US)) set_baud(460800);
         else if (delta >= sc_time(900, SC_NS)) set_baud(921600);
         else if (delta >= sc_time(400, SC_NS)) set_baud(1843200);
         else if (delta >= sc_time(200, SC_NS)) set_baud(3686400);
         else {
            PRINTF_WARN("UART", "rate too fast on UART %s", name());
            set_baud(3686400);
         }
         /* We wait now for the packet to end, assuming 2 stop bits and some
          * extra time.
          */
         wait(baudperiod * 10);
         /* And we clear the flag. */
         autodetect = false;

         continue;
      }

      /* We jump halfway into the start pulse. */
      wait(baudperiod/2);
      /* And we advance to the next bit. */
      wait(baudperiod);
      /* And we take one bit at a time and merge them together. */
      msg = 0;
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         incomming = rx.read();
         if (debug) {
            PRINTF_INFO("UART", "[%s/%d]: received lvl %c", name(), cnt,
               (incomming)?'h':'l');
         }
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
         PRINTF_WARN("UART", "Buffer overflow on UART %s", name());
      }
      else {
         from.write(msg);
         if (debug && isprint(msg)) {
            PRINTF_INFO("UART", "[%s] received-'%c'/%02x\n", name(), msg, msg);
         }
         else if (debug) {
            PRINTF_INFO("UART", "[%s] received-%02x\n", name(), msg);
         }
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
      if (debug && isprint(msg)) {
         PRINTF_INFO("UART","[%s] sending-'%c'/%02x", name(), msg, msg);
      }
      else if (debug) {
         PRINTF_INFO("UART","[%s] sending-%02x", name(), msg);
      }
      /* Then we send the packet asynchronously. */
      tx.write(false);
      wait(baudperiod);
      for(cnt = 0, pos = 1; cnt < 8; cnt = cnt + 1, pos = pos << 1) {
         if(debug) {
            PRINTF_INFO("UART", "[%s/%d]: sent '%c'", name(), cnt,
               ((pos & msg)>0)?'h':'l');
         }
         if ((pos & msg)>0) tx.write(true);
         else tx.write(false);
         wait(baudperiod);
      }
      /* And we send the stop bit. */
      tx.write(true);
      if (stopbits < 2) wait(baudperiod);
      else if (stopbits == 2) wait(baudperiod + baudperiod / 2);
      else wait(baudperiod * 2);
   }
}

void uart::set_baud(unsigned int rate) {
   baudrate = rate;
   baudperiod = sc_time(8680, SC_NS) * (115200 / rate);
}

int uart::getautorate() {
      /* If autodetect is still true, it is not done, we return 0. */
      if (autodetect) { return 0; }
      /* if it is false, we return the rate. */
      else { return get_baud(); }
   }

