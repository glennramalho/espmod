/*
 esp32-hal-gpio.h - GPIO Interface Functions
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

#ifndef _ESP32_HAL_GPIO_H
#define _ESP32_HAL_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "esp32-hal.h"

#define LOW               0x0
#define HIGH              0x1

//GPIO FUNCTIONS
#define INPUT             0x01
#define OUTPUT            0x02
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define SPECIAL           0xF0
#define FUNCTION_1        0x00
#define FUNCTION_2        0x20
#define FUNCTION_3        0x40
#define FUNCTION_4        0x60
#define FUNCTION_5        0x80
#define FUNCTION_6        0xA0
#define ANALOG            0xC0

//Interrupt Modes
#define DISABLED  0x00
#define RISING    0x01
#define FALLING   0x02
#define CHANGE    0x03
#define ONLOW     0x04
#define ONHIGH    0x05
#define ONLOW_WE  0x0C
#define ONHIGH_WE 0x0D

#define NUM_GPIOS 40

typedef struct {
    uint8_t reg;      /*!< GPIO register offset from DR_REG_IO_MUX_BASE */
    int8_t rtc;       /*!< RTC GPIO number (-1 if not RTC GPIO pin) */
    int8_t adc;       /*!< ADC Channel number (-1 if not ADC pin) */
    int8_t touch;     /*!< Touch Channel number (-1 if not Touch pin) */
} esp32_gpioMux_t;

extern const esp32_gpioMux_t esp32_gpioMux[NUM_GPIOS];
extern const int8_t esp32_adc2gpio[20];

#define digitalPinIsValid(pin)          ((pin) < 40 && esp32_gpioMux[(pin)].reg)
#define digitalPinCanOutput(pin)        ((pin) < 34 && esp32_gpioMux[(pin)].reg)
#define digitalPinToRtcPin(pin)         (((pin) < 40)?esp32_gpioMux[(pin)].rtc:-1)
#define digitalPinToAnalogChannel(pin)  (((pin) < 40)?esp32_gpioMux[(pin)].adc:-1)
#define digitalPinToTouchChannel(pin)   (((pin) < 40)?esp32_gpioMux[(pin)].touch:-1)
#define digitalPinToDacChannel(pin)     (((pin) == 25)?0:((pin) == 26)?1:-1)

/* These are the standard Arduino functions. Note that often firmware does not
 * do any delays. They rely on the cycle delays of the CPU to advance time.
 * Being we did not model the CPU, we have to add them. Therefore we have two
 * sets of functions. The standar Arduino versions add a 1 cycle delay. The
 * _nodel versions can be used to force the result to be immediate. These
 * immediate ones are intended to be used internally.
 */
void pinMode_nodel(uint8_t pin, uint8_t mode);
void digitalWrite_nodel(uint8_t pin, uint8_t val);
int digitalRead_nodel(uint8_t pin);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);

void attachInterrupt(uint8_t pin, void (*)(void), int mode);
void attachInterruptArg(uint8_t pin, void (*)(void*), void * arg, int mode);
void detachInterrupt(uint8_t pin);

#include "pins_arduino.h"
/* We need these for the Wire library */
#define PIN_WIRE_SDA SDA
#define PIN_WIRE_SCL SCL

const char *pinname(int pin);

#include <sdkconfig.h>

#ifdef __cplusplus
}
#endif

#endif
