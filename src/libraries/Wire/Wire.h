/*
  TwoWire.h - CCHAN I2C - Glenn Ramalho RFIDo
  Copyright (c) 2019 Nicholas Zambetti.  All right reserved.

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

  This file was bassed off the work covered by the license below:

     TwoWire.h - TWI/I2C library for Arduino & Wiring
     Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

 */

#ifndef TWOWIRE_H
#define TWOWIRE_H

#include <inttypes.h>
#include "TestSerial.h"

class TwoWire : public TestSerial
{
  private:
    uint8_t txAddress;
    int size;
    bool transmitting;
  public:
    TwoWire();
    void begin(int sda, int scl);
    void pins(int sda, int scl) __attribute__((deprecated));
    void begin();
    void begin(uint8_t);
    void begin(int);
    void setClock(uint32_t);
    void setClockStretchLimit(uint32_t);
    void beginTransmission(uint8_t);
    void beginTransmission(int);
    uint8_t endTransmission(void);
    uint8_t endTransmission(uint8_t);
    size_t requestFrom(uint8_t address, size_t size, bool sendStop);
    size_t requestFrom(uint8_t address, size_t size) {
       return requestFrom(address, size, true);
    }
    bool writebin(uint8_t data, int bits);
    size_t write(uint8_t);
    size_t write(const uint8_t *, size_t);
    size_t send(uint8_t i) { return write(i); }
    int receive(void) { return read(); }
    int read();
    int peek();
    void flush(void);
    size_t write(unsigned long n) { return write((uint8_t)n); }
    size_t write(long n) { return write((uint8_t)n); }
    size_t write(unsigned int n) { return write((uint8_t)n); }
    size_t write(int n) { return write((uint8_t)n); }
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
extern TwoWire Wire;
#endif

#endif
