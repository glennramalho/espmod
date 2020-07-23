/*
  TwoWire.cpp - CCHAN I2C - Glenn Ramalho RFIDo
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

     TwoWire.cpp - TWI/I2C library for Arduino & Wiring
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

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include "esp32-hal-i2c.h"
#include "Arduino.h"
#include "Wire.h"
#include <systemc.h>
#include "info.h"

//Some boards don't have these pins available, and hence don't support Wire.
//Check here for compile-time error.
#if !defined(PIN_WIRE_SDA) || !defined(PIN_WIRE_SCL)
#error Wire library is not supported on this board
#endif

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire(uint8_t bus_num)
    :num(bus_num & 1)
    ,sda(-1)
    ,scl(-1)
    ,i2c(NULL)
    ,last_error(I2C_ERROR_OK)
    ,_timeOutMillis(50)
{
   transmitting = false;
}

TwoWire::~TwoWire()
{
    //flush();
    if(i2c) {
        i2cRelease(i2c);
        i2c=NULL;
    }
}

// Public Methods //////////////////////////////////////////////////////////////

bool TwoWire::begin(int sdaPin, int sclPin, uint32_t frequency)
{
   PRINTF_INFO("WIRE",
      "Setting I2C %d to Master mode, SDA on pin %s, SCL on pin %s",
      num, pinname(PIN_WIRE_SDA), pinname(PIN_WIRE_SCL));

   if(sdaPin < 0) { // default param passed
        if(num == 0) {
            if(sda==-1) {
                sdaPin = PIN_WIRE_SDA;    //use Default Pin
            } else {
                sdaPin = sda;    // reuse prior pin
            }
        } else {
            if(sda==-1) {
                log_e("no Default SDA Pin for Second Peripheral");
                return false; //no Default pin for Second Peripheral
            } else {
                sdaPin = sda;    // reuse prior pin
            }
        }
    }

    if(sclPin < 0) { // default param passed
        if(num == 0) {
            if(scl == -1) {
                sclPin = PIN_WIRE_SCL;    // use Default pin
            } else {
                sclPin = scl;    // reuse prior pin
            }
        } else {
            if(scl == -1) {
                log_e("no Default SCL Pin for Second Peripheral");
                return false; //no Default pin for Second Peripheral
            } else {
                sclPin = scl;    // reuse prior pin
            }
        }
    }

    sda = sdaPin;
    scl = sclPin;
    i2c = i2cInit(num, sdaPin, sclPin, frequency);
    if(!i2c) {
        return false;
    }

    flush();
    return true;

}

void TwoWire::setClock(uint32_t frequency){
   printf("Setting Frequency to %u\n", frequency);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
    return requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(quantity), static_cast<bool>(sendStop));
}

uint8_t TwoWire::requestFrom(uint16_t address, uint8_t quantity, uint8_t sendStop)
{
    return requestFrom(address, static_cast<size_t>(quantity), static_cast<bool>(sendStop));
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
    return requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(quantity), true);
}

uint8_t TwoWire::requestFrom(uint16_t address, uint8_t quantity)
{
    return requestFrom(address, static_cast<size_t>(quantity), true);
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
    return requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(quantity), true);
}

/*
uint8_t TwoWire::requestFrom(uint16_t address, uint8_t quantity, int sendStop)
{
    return static_cast<uint8_t>(requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(quantity), static_cast<bool>(sendStop)));
}
*/

uint8_t TwoWire::requestFrom(uint16_t address, uint8_t size, bool sendStop){
   unsigned int i, bit;
   /* We are starting a new command, so we dump anything in the fifo from the
    * previous command.
    */
   while(from->num_available()) from->read();

   if (from->num_free() < (int)(size*8) || to->num_free() < 10 ||
         to->num_free() < (int)(size*8)) {
      SC_REPORT_ERROR("WIRE", "I2C Fifo Full");
      return 0;
   }

   /* We send the request. */
   to->write('S');
   writebin((address<<1) + 1, 8); /* Address + R=1 */
   to->write('Z');
   wait(from->data_written_event()); /* Wait for A */
   /* If no Acknowledge came, we return zero */
   if (from->read() != 'A') return 0;

   /* We now collect the data. We send the ACK after each byte except the last
    * one. */
   for(i = 0; i < size; i = i + 1) {
      for(bit = 0; bit < 8; bit = bit + 1) {
         to->write('Z');
         wait(from->data_written_event());
      }
      if ((signed int)i != size-1) to->write('A');
   }
   /* And we send a NACK followed by the Stop */
   if(sendStop) {
      to->write('N');
      to->write('P');
   }

   return size;
}

void TwoWire::beginTransmission(uint8_t address){
   /* We are starting a new command, so we dump anything in the fifo from the
    * previous command.
    */
   while(from->num_available()) from->read();

   if (from->num_free() < 1 || to->num_free() < 10) {
      SC_REPORT_ERROR("WIRE", "I2C Fifo Full");
      return;
   }

   /* We send the Start bit and the ID. */
   to->write('S');
   writebin((address<<1) + 0, 8); /* Address + W=0 */
   to->write('Z');
   wait(sc_time(1, SC_US), from->data_written_event()); /* Wait for A */
   /* If no Acknowledge came, we flag it */
   if (from->read() != 'A') transmitting = false;
   else transmitting = true;
}

void TwoWire::beginTransmission(int address){
   beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(bool sendStop){
  /* We send the stop bit. */
  if (sendStop) to->write('P');
  transmitting = false;
  return I2C_ERROR_OK; // Lacks more to do here. TODO
}

uint8_t TwoWire::endTransmission(void){
  return endTransmission(true);
}

size_t TwoWire::write(uint8_t data) {
   if (!transmitting) return 0;
   if (to->num_free() < 9 || from->num_free() < 1) {
      SC_REPORT_ERROR("WIRE", "write err: I2C Fifo Full");
      return 0;
   }

   do {
      writebin(data, 8);
      to->write('Z');
      wait(from->data_written_event()); /* Wait for A */
      /* If no Acknowledge came, we flag it */
   } while(from->read() != 'A');

   return 1;
}

bool TwoWire::writebin(uint8_t data, int bits) {
   int i;
   if (to->num_free() < 1) {
      SC_REPORT_ERROR("WIRE", "write err: I2C Fifo Full");
      return false;
   }

   for (i = bits-1; i >= 0; i = i - 1) {
      if ((data & (1<<i))>0) to->write('1');
      else to->write('0');
   }
   return true;
}

size_t TwoWire::write(const uint8_t *data, size_t size) {
   unsigned int i;
   for(i = 0; i < size; i = i + 1) {
      writebin(data[i], 8);
   }
   return size;
}

int TwoWire::read() {
   if (taken) {
      taken = false;
      return waiting;
   }
   else if (from->num_available() < 8) return -1;
   int i;
   int ch;
   int data = 0;
   for (i = 7; i >= 0; i = i - 1) {
      ch = from->read();
      if (ch == '0') data = data << 1;
      else if (ch == '1') data = (data << 1) + 1;
      else return -1;
   }
   return data;
}

int TwoWire::peek() {
   if (taken) return waiting;
   else if (from->num_available() < 8) return -1;
   else {
      taken = true;
      int i;
      int ch;
      waiting = 0;
      for (i = 7; i >= 0; i = i - 1) {
         ch = from->read();
         if (ch == '0') waiting = waiting << 1;
         else if (ch == '1') waiting = (waiting << 1) + 1;
         else waiting = -1;
      }
      return waiting;
   }
}

void TwoWire::flush(void){
   while(from->num_available()>0) from->read();
   while(to->num_available()>0) to->read();
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
TwoWire Wire(0);
TwoWire Wire1(1);
#endif
