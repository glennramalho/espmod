/*
 Arduino.cpp - Main Arduino-ESP32 Model File
 Copyright (c) 2019 Glenn Ramalho - RFIDo. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 This file was based off the work covered by the license below:
    Copyright (c) 2005-2013 Arduino Team.  All right reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 */

#include <systemc.h>
#include "Arduino.h"
#include <sys/time.h>
#include <string.h>

uint16_t makeWord(uint16_t w) { return w; }
uint16_t makeWord(byte h, byte l) { return ((h&0xff)<<8)|l&0xff; }

void delay(uint32_t del) {
   wait(del, SC_MS);
}

void delayMicroseconds(uint32_t del) {
   wait(del, SC_US);
}

unsigned long int millis() {
   return (unsigned long int)floor(sc_time_stamp().to_seconds() * 1000);
}

void yield() { wait(125, SC_NS); }
void del1cycle() { wait(125, SC_NS); }
void wait_next_apb_clock() {
   long int nanoseconds;
   nanoseconds = (long int)floor(sc_time_stamp().to_seconds() * 1e9);
   wait(APB_CLOCK_PERIOD - nanoseconds % APB_CLOCK_PERIOD, SC_NS);
}

unsigned int interruptsenabled;
unsigned int xt_rsil(unsigned int lvl) {
   unsigned int oldlvl;
   oldlvl = interruptsenabled;
   interruptsenabled = lvl;
   return oldlvl;
}

bool interruptsEnabled() {
   return interruptsenabled < 15;
}

long map(long value, long fromMin, long fromMax, long toMin, long toMax) {
   return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

long random(long max) {
   return random(0, max);
}
long random(long min, long max) {
   /* We first pick a number. */
   long r = random();
   unsigned long diff;
   long long nr;

   /* Now we cap it to the max width. */
   diff = (unsigned int)(max - 1 - min);
   r = r % (diff + 1);

   /* Now we shift the numbers to the correct window. */
   nr = r + min;

   /* Now we cut down the precision to return the value. */
   return (long)nr;
}
