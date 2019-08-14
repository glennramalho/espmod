/*******************************************************************************
 * gpioset.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "gpioset.h"
#include "esp32-hal-gpio.h"
#include "info.h"

gpio *gpioptr[NUM_GPIOS];

void pinset(int pin, void *ngpioptr) {
   if (pin < NUM_GPIOS) gpioptr[pin] = (gpio *)ngpioptr;
   else PRINTF_WARN("GPIOSET", "Pin %d does not have a GPIO associated", pin);
}

gpio *getgpio(int pin) {
   if (pin >= NUM_GPIOS || gpioptr[pin] == NULL) return NULL;
   else return gpioptr[pin];
}

functypes_t funcmatrix[40][6] =
{
   {UGPIO, UALT,  UGPIO, UWARN, UWARN, UWARN}, /* GPIO0 */
   {UALT,  UALT,  UGPIO, UWARN, UWARN, UWARN}, /* GPIO1 */
   {UGPIO, UALT,  UGPIO, UALT , UALT , UWARN}, /* GPIO2 */
   {UALT,  UALT,  UGPIO, UWARN, UWARN, UWARN}, /* GPIO3 */
   {UGPIO, UALT,  UGPIO, UALT , UALT , UALT }, /* GPIO4 */
   {UGPIO, UALT,  UGPIO, UALT , UWARN, UALT }, /* GPIO5 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO6 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO7 */
   {UALT,  UALT , UGPIO, UALT , UALT , UWARN}, /* GPIO8 */
   {UALT,  UALT , UGPIO, UALT , UALT , UWARN}, /* GPIO9 */
   {UALT,  UALT , UGPIO, UALT , UALT , UWARN}, /* GPIO10 */
   {UALT,  UALT , UGPIO, UALT , UALT , UWARN}, /* GPIO11 */
   {UALT,  UALT , UGPIO, UALT , UALT , UALT }, /* GPIO12 */
   {UALT,  UALT , UGPIO, UALT , UALT , UALT }, /* GPIO13 */
   {UALT,  UALT , UGPIO, UALT , UALT , UALT }, /* GPIO14 */
   {UALT,  UALT , UGPIO, UALT , UALT , UALT }, /* GPIO15 */
   {UGPIO, UWARN, UGPIO, UALT , UALT , UALT }, /* GPIO16 */
   {UGPIO, UWARN, UGPIO, UALT , UALT , UALT }, /* GPIO17 */
   {UGPIO, UALT , UGPIO, UALT , UWARN, UALT }, /* GPIO18 */
   {UGPIO, UALT , UGPIO, UALT , UWARN, UALT }, /* GPIO19 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO20 */
   {UGPIO, UALT , UGPIO, UWARN, UWARN, UALT }, /* GPIO21 */
   {UGPIO, UALT , UGPIO, UALT , UWARN, UALT }, /* GPIO22 */
   {UGPIO, UALT , UGPIO, UALT , UWARN, UWARN}, /* GPIO23 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO24 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO25 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO26 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO27 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO28 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO29 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO30 */
   {UWARN, UWARN, UWARN, UWARN, UWARN, UWARN}, /* GPIO31 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO32 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UALT }, /* GPIO33 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO34 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO35 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO36 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO37 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}, /* GPIO38 */
   {UGPIO, UWARN, UGPIO, UWARN, UWARN, UWARN}  /* GPIO39 */
};
