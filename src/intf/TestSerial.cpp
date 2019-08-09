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

#include "TestSerial.h"
#include "Arduino.h"

TestSerial::TestSerial() {taken = false; waiting = '\0'; pinmodeset = false;}
TestSerial::TestSerial(int _rxpin, int _rxmode, int _txpin, int _txmode) {
   taken = false; waiting = '\0'; pinmodeset = true;
   rxpin = _rxpin;
   rxmode = _rxmode;
   txpin = _txpin;
   txmode = _txmode;
}

TestSerial::~TestSerial() {}

void TestSerial::begin(int baudrate) {
   ::printf("Setting Baud rate to %0d\n", baudrate);
   /* If pins were specified to go to some mode we do it. */
   if (pinmodeset) {
      /* We first drive the TX level high. It is now an input, but when we
       * change the pin to be an output, it will not drive a zero.
       */
      digitalWrite(txpin, true);
      pinMode(rxpin, rxmode);
      pinMode(txpin, txmode);
      /* Not quite the code but works for now. */
      gpio_iomux_out(rxpin, 0, false);
      gpio_iomux_out(txpin, 0, false);
   }
}

void TestSerial::begin(int baudrate, int mode, int pinrx, int pintx) {
   ::printf("Setting Baud rate to %0d\n", baudrate);
   /* We first drive the TX level high. It is now an input, but when we change
    * the pin to be an output, it will not drive a zero.
    */
   digitalWrite(txpin, true);
   pinMode(pinrx, rxmode);
   pinMode(pintx, txmode);
   /* Not quite the code but works for now. */
   gpio_iomux_out(pinrx, 0, false);
   gpio_iomux_out(pintx, 0, false);
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

void TestSerial::end() { ::printf("Ending Serial\n");}

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
