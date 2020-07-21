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
#include "esp32-hal-i2c.h"

class TwoWire : public TestSerial
{
protected:
    uint8_t num;
    int8_t sda;
    int8_t scl;
    i2c_t * i2c;

    bool transmitting;
    i2c_err_t last_error; // @stickBreaker from esp32-hal-i2c.h
    uint16_t _timeOutMillis;

public:
    TwoWire(uint8_t bus_num);
    ~TwoWire();
    bool begin(int sda=-1, int scl=-1, uint32_t frequency=0); // returns true, if successful init of i2c bus
      // calling will attemp to recover hung bus

    void setClock(uint32_t frequency); // change bus clock without initing hardware
    size_t getClock(); // current bus clock rate in hz

    void setTimeOut(uint16_t timeOutMillis); // default timeout of i2c transactions is 50ms
    uint16_t getTimeOut();

    uint8_t lastError();
    char * getErrorText(uint8_t err);

    //@stickBreaker for big blocks and ISR model
    i2c_err_t writeTransmission(uint16_t address, uint8_t* buff, uint16_t size, bool sendStop=true);
    i2c_err_t readTransmission(uint16_t address, uint8_t* buff, uint16_t size, bool sendStop=true, uint32_t *readCount=NULL);

    void beginTransmission(uint16_t address);
    void beginTransmission(uint8_t address);
    void beginTransmission(int address);

    uint8_t endTransmission(bool sendStop);
    uint8_t endTransmission(void);

    uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop);
    uint8_t requestFrom(uint16_t address, uint8_t size, uint8_t sendStop);
    uint8_t requestFrom(uint16_t address, uint8_t size);
    uint8_t requestFrom(uint8_t address, uint8_t size, uint8_t sendStop);
    uint8_t requestFrom(uint8_t address, uint8_t size);
    uint8_t requestFrom(int address, int size, int sendStop);
    uint8_t requestFrom(int address, int size);

    size_t write(uint8_t);
    size_t write(const uint8_t *, size_t);
    size_t send(uint8_t i) { return write(i); }
    int receive(void) { return read(); }
    //int available(void);
    int read(void);
    int peek(void);
    void flush(void);

    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t)n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t)n);
    }

    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );

    uint32_t setDebugFlags( uint32_t setBits, uint32_t resetBits);
    bool busy();

private:
    bool writebin(uint8_t data, int bits);
};

extern TwoWire Wire;
extern TwoWire Wire1;

#endif
