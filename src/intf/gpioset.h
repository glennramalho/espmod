/*******************************************************************************
 * gpioset.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Connects the GPIO SystemC library with the ESP Model GPIO functions.
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

#ifndef _GPIOSET_H
#define _GPIOSET_H

#include "io_mux.h"

void pinset(int pin, void *ngpioptr);
io_mux *getgpio(int pin);
#define GETFUNC(x) ((x)>>5)

typedef enum {UWARN, UGPIO, UALT} functypes_t;
extern functypes_t funcmatrix[40][6];

#endif
