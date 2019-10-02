/*******************************************************************************
 * TestSerial.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
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

#ifndef _TESTSERIAL_H
#define _TESTSERIAL_H

#include <inttypes.h>
#include "Stream.h"
#include <stdarg.h>
#include <systemc.h>

#define SERIAL_5N1 0x8000010
#define SERIAL_6N1 0x8000014
#define SERIAL_7N1 0x8000018
#define SERIAL_8N1 0x800001c
#define SERIAL_5N2 0x8000030
#define SERIAL_6N2 0x8000034
#define SERIAL_7N2 0x8000038
#define SERIAL_8N2 0x800003c
#define SERIAL_5E1 0x8000012
#define SERIAL_6E1 0x8000016
#define SERIAL_7E1 0x800001a
#define SERIAL_8E1 0x800001e
#define SERIAL_5E2 0x8000032
#define SERIAL_6E2 0x8000036
#define SERIAL_7E2 0x800003a
#define SERIAL_8E2 0x800003e
#define SERIAL_5O1 0x8000013
#define SERIAL_6O1 0x8000017
#define SERIAL_7O1 0x800001b
#define SERIAL_8O1 0x800001f
#define SERIAL_5O2 0x8000033
#define SERIAL_6O2 0x8000037
#define SERIAL_7O2 0x800003b
#define SERIAL_8O2 0x800003f

class TestSerial: public Stream {
   public:
   TestSerial();
   TestSerial(int uartno);
   TestSerial(int pinrx, int rxmode, int pintx, int txmode);
   virtual ~TestSerial();

   void setports(sc_fifo<unsigned char> *_to, sc_fifo<unsigned char> *from);
   void begin(int baudrate, int mode = -1, int pinrx = -1, int pintx = -1);
   void end();

   int printf(const char *fmt, ...);

   /* We have these to dump some random data for testing. */
   int available() override;
   int availableForWrite();
   void flush();
   void stop() {printf("Stopping Client\n"); }

   int read() override;
   int peek() override;
   size_t write(uint8_t ch) override;
   size_t write(const char* buf);
   size_t write(const char* buf, size_t len);

   unsigned char bl_peek();
   unsigned char bl_read();

   bool isinit();

   protected:
   int uartno;
   sc_fifo<unsigned char> *to;
   sc_fifo<unsigned char> *from;
   bool taken;
   unsigned char waiting;
   bool pinmodeset;
   int rxpin, rxmode, txpin, txmode;
};

#endif
