/*
 esp32-hal-gpio.cpp - GPIO SystemC Interface File
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
#include "info.h"
#include "esp32-hal-gpio.h"
#include "gpioset.h"

const int8_t esp32_adc2gpio[20] = {36, 37, 38, 39, 32, 33, 34, 35, -1, -1, 4, 0, 2, 15, 13, 12, 14, 27, 25, 26};

const DRAM_ATTR esp32_gpioMux_t esp32_gpioMux[GPIO_PIN_COUNT]={
    {0x44, 11, 11, 1},
    {0x88, -1, -1, -1},
    {0x40, 12, 12, 2},
    {0x84, -1, -1, -1},
    {0x48, 10, 10, 0},
    {0x6c, -1, -1, -1},
    {0x60, -1, -1, -1},
    {0x64, -1, -1, -1},
    {0x68, -1, -1, -1},
    {0x54, -1, -1, -1},
    {0x58, -1, -1, -1},
    {0x5c, -1, -1, -1},
    {0x34, 15, 15, 5},
    {0x38, 14, 14, 4},
    {0x30, 16, 16, 6},
    {0x3c, 13, 13, 3},
    {0x4c, -1, -1, -1},
    {0x50, -1, -1, -1},
    {0x70, -1, -1, -1},
    {0x74, -1, -1, -1},
    {0x78, -1, -1, -1},
    {0x7c, -1, -1, -1},
    {0x80, -1, -1, -1},
    {0x8c, -1, -1, -1},
    {0, -1, -1, -1},
    {0x24, 6, 18, -1}, //DAC1
    {0x28, 7, 19, -1}, //DAC2
    {0x2c, 17, 17, 7},
    {0, -1, -1, -1},
    {0, -1, -1, -1},
    {0, -1, -1, -1},
    {0, -1, -1, -1},
    {0x1c, 9, 4, 9},
    {0x20, 8, 5, 8},
    {0x14, 4, 6, -1},
    {0x18, 5, 7, -1},
    {0x04, 0, 0, -1},
    {0x08, 1, 1, -1},
    {0x0c, 2, 2, -1},
    {0x10, 3, 3, -1}
};

/* TODO add interrupt handling
typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrArg)(void*);
typedef struct {
    voidFuncPtr fn;
    void* arg;
    bool functional;
} InterruptHandle_t;
static InterruptHandle_t __pinInterruptHandlers[GPIO_PIN_COUNT] = {0,};
*/

// TODO add rtc/io #include "driver/rtc_io.h"

#define GETFUNC(x) ((x)>>5)

/* Sets the mode of a pin. */
void pinMode_nodel(uint8_t pin, uint8_t mode) {

   if(!digitalPinIsValid(pin)) {
      return;
   }

   /* We first need the GPIO pointer. */
   gpio* gpin = getgpio(pin);
   if (gpin == NULL) {
      PRINTF_ERROR("HALGPIO",
         "Attemping to set mode to non existant GPIO%d", pin);
      return;
   }

   /* Now we set the function. We can only do this to MF GPIOs. */
   if (mode == ANALOG) {
      // TODO add RTC io
      //uint32_t rtc_reg = rtc_gpio_desc[pin].reg;
      //if(mode == ANALOG) if(!rtc_reg) return;//not rtc pin
      gpin->set_function(GPIOMF_ANALOG);
      return;
   }

   /* We set the direction. */
   if(mode & (INPUT | OUTPUT)) gpin->set_dir(GPIODIR_INOUT);
   else if(mode & INPUT) gpin->set_dir(GPIODIR_INPUT);
   else if(mode & OUTPUT) gpin->set_dir(GPIODIR_OUTPUT);
   else gpin->set_dir(GPIODIR_NONE);

   /* Pullup */
   if(mode & PULLUP) gpin->set_wpu();
   else gpin->clr_wpu();
   if(mode & PULLDOWN) gpin->set_wpd();
   else gpin->clr_wpd();

   /* We ignore the driver strength */

   /* OD */
   if(mode & OPEN_DRAIN) gpin->set_od();
   else gpin->clr_od();
}

/* Sets the mode of a pin, with a delay. */
void pinMode(uint8_t pin, uint8_t mode) {
   pinMode_nodel(pin, mode);
   del1cycle();
}

/* Drives a digital GPIO pin high or low. It must have a GPIO associated to it
 * or it fails.
 */
void digitalWrite_nodel(uint8_t pin, uint8_t val) {
   char buffer[64];
   gpio *gpin = getgpio(pin);

   /* If the pin passes the valid pins or if there is no GPIO assigned, we
    * issue a warning.
    */
   if (gpin == NULL) {
      sprintf(buffer, "No gpio defined for pin %d", pin);
      SC_REPORT_WARNING("ESP32PINS", buffer);
   }
   else if (val != HIGH && val != LOW) {
      sprintf(buffer, "Only HIGH or LOW can be driven on D%d", pin);
      SC_REPORT_WARNING("ESP32PINS", buffer);
   }
   else {
      /* We now drive a pin high or low. */
      gpin->set_val((val == HIGH)?true:false);
   }
}

/* Same but adds a 1cycle delay. */
void digitalWrite(uint8_t pin, uint8_t val) {
   digitalWrite_nodel(pin, val);
   del1cycle();
}

/* Samples a pin. */
int digitalRead_nodel(uint8_t pin) {
   char buffer[64];
   gpio *gpin = getgpio(pin);

   /* If the pin passes the valid pins or if there is no GPIO assigned, we
    * issue a warning.
    */
   if (gpin == NULL) {
      sprintf(buffer, "No gpio defined for pin %d", pin);
      SC_REPORT_WARNING("ESP32PINS", buffer);
      return LOW;
   }
   else if (gpin->get_val() == true) return HIGH;
   else return LOW;
}

/* Same but also delays one cycle. */
int digitalRead(uint8_t pin) {
   del1cycle();
   return digitalRead_nodel(pin);
}
