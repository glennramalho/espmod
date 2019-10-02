/*******************************************************************************
 * TestSerial.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Generic Serial Class for communicating with a set of SystemC ports.
 *   This class tries to mimic the behaviour used by the Hardware Serial
 *   and SoftwareSerial classes used by the Arduino IDE.
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
#include "info.h"
#include "TestSerial.h"
#include "Arduino.h"

TestSerial::TestSerial() {
   taken = false;
   waiting = '\0';
   uartno = -1;
   pinmodeset = false;
}
TestSerial::TestSerial(int _u) {
   if(_u < 0 || _u > 2) {
      printf("Invalid UART %d", _u);
      uartno = 0;
   }
   uartno = _u;
   taken = false;
   waiting = '\0';
   pinmodeset = false;
}

TestSerial::TestSerial(int _rxpin, int _rxmode, int _txpin, int _txmode) {
   taken = false; waiting = '\0'; pinmodeset = true;
   rxpin = _rxpin;
   rxmode = _rxmode;
   txpin = _txpin;
   txmode = _txmode;
}

TestSerial::~TestSerial() {}

void TestSerial::begin(int baudrate, int mode, int pinrx, int pintx) {
   if (baudrate <= 0) baudrate = 9600;
   PRINTF_INFO("TSER", "Setting UART %d Baud rate to %0d", uartno, baudrate);

   /* We set the pin modes. If a value was given in the arguments, we take it.
    * If not, we check the pinmodeset list. If it was set, we leave it as is.
    * If not, we go with the UART number.
    */
   if (pinrx >= 0) rxpin = pinrx;
   else if (!pinmodeset) switch(uartno) {
      case 0: rxpin = RX; break;
      case 1: rxpin = 9; break;
      case 2: rxpin = 16; break;
   }
   if (pintx >= 0) txpin = pintx;
   else if (!pinmodeset) switch(uartno) {
      case 0: txpin = TX; break;
      case 1: txpin = 10; break;
      case 2: txpin = 17; break;
   }
   /* For the modes, we do the same. */
   if (mode < 0 && !pinmodeset) {
      rxmode = INPUT;
      txmode = OUTPUT;
   }

   /* We first drive the TX level high. It is now an input, but when we change
    * the pin to be an output, it will not drive a zero.
    */
   pinMode(rxpin, rxmode);
   pinMatrixInAttach(rxpin,
      (uartno == 0)?U0RXD_IN_IDX:
      (uartno == 1)?U1RXD_IN_IDX:
      (uartno == 2)?U2RXD_IN_IDX:0, false);
   pinMode(txpin, txmode);
   pinMatrixOutAttach(txpin,
      (uartno == 0)?U0TXD_OUT_IDX:
      (uartno == 1)?U1TXD_OUT_IDX:
      (uartno == 2)?U2TXD_OUT_IDX:0, false, false);
}

void TestSerial::setports(sc_fifo<unsigned char> *_to,
      sc_fifo<unsigned char> *_from) {
   to = _to;
   from = _from;
}

bool TestSerial::isinit() {
   if (from == NULL || to == NULL) return false;
   else return true;
}

void TestSerial::end() { PRINTF_INFO("TSER", "Ending Serial");}

int TestSerial::printf(const char *fmt, ...) {
   char buff[128], *ptr;
   int resp;
   va_list argptr;
   va_start(argptr, fmt);
   resp = vsnprintf(buff, 127, fmt, argptr);
   va_end(argptr);
   ptr = buff;
   if (to == NULL) return 0;
   while(*ptr != '\0') {
      del1cycle();
      to->write(*ptr++);
   }
   return resp;
}

size_t TestSerial::write(uint8_t ch) {
   del1cycle();
   if (to == NULL) return 0;
   to->write(ch);
   return 1;
}

size_t TestSerial::write(const char* buf) {
   size_t sent = 0;
   del1cycle();
   while(*buf != '\0') {
      write(*buf);
      sent = sent + 1; 
      buf = buf + 1;
   }
   return sent;
}

size_t TestSerial::write(const char* buf, size_t len) {
   size_t sent = 0;
   del1cycle();
   while(sent < len) {
      write(*buf);
      sent = sent + 1; 
      buf = buf + 1;
   }
   return sent;
}

int TestSerial::availableForWrite() {
   /* We return if there is space in the buffer.  */
   if (to == NULL) return 0;
   return to->num_free();
}

int TestSerial::available() {
   /* If we have something taken due to a peek, we need to inform the
    * requester that we still have something to return. So we have at
    * least one. If taken is false, then it depends on the number
    * available.
    */
   if (from == NULL) return 0;
   if (taken) return 1 + from->num_available();
   else return from->num_available();
}
void TestSerial::flush() {
   /* For flush we get rid of the taken character and then we use the FIFO's
    * read to dump the rest.
    */
   taken = false;
   while(from != NULL && from->num_available()>0) from->read();
}

int TestSerial::read() {
   /* Anytime we leave the CPU we need to do a dummy delay. This makes sure
    * time advances, in case we happen to be in a timeout loop.
    */
   del1cycle();
   if (from == NULL) return -1;
   if (taken) {
      taken = false;
      return waiting;
   }
   else if (!from->num_available()) return -1;
   return (int)from->read();
}

unsigned char TestSerial::bl_read() {
   /* Just like the one above but it does a blocking read. Useful to cut
    * on polled reads.
    */
   del1cycle();
   if (from == NULL) return -1;
   if (taken) {
      taken = false;
      return waiting;
   }
   return from->read();
}

int TestSerial::peek() {
   del1cycle();
   if (from == NULL) return -1;
   if (taken) return waiting;
   else if (!from->num_available()) return -1;
   else {
      taken = true;
      waiting = from->read();
      return waiting;
   }
}

unsigned char TestSerial::bl_peek() {
   del1cycle();
   if (from == NULL) return -1;
   if (taken) return waiting;
   else {
      taken = true;
      waiting = from->read();
      return waiting;
   }
}
