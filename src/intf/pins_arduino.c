/*******************************************************************************
 * pins_arduino.c -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a function that returns the pin name.
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
 */

#include "pins_arduino.h"
#include <stdio.h>

const char *pinname(int pin) {
   static char buffer[11];
   if (pin == T0) sprintf(buffer, "A10/T0/D%d", pin);
   else if (pin == T1) sprintf(buffer, "A11/T1/D%d", pin);
   else if (pin == T2) sprintf(buffer, "A12/T2/D%d", pin);
   else if (pin == T3) sprintf(buffer, "A13/T3/D%d", pin);
   else if (pin == T4) sprintf(buffer, "A14/T4/D%d", pin);
   else if (pin == T5) sprintf(buffer, "A15/T5/D%d", pin);
   else if (pin == T6) sprintf(buffer, "A16/T6/D%d", pin);
   else if (pin == T7) sprintf(buffer, "A17/T7/D%d", pin);
   else if (pin == T8) sprintf(buffer, "A5/T8/D%d", pin);
   else if (pin == T9) sprintf(buffer, "A4/T9/D%d", pin);
   else if (pin == SS) sprintf(buffer, "SS/D%d", pin);
   else if (pin == MOSI) sprintf(buffer, "MOSI/D%d", pin);
   else if (pin == MISO) sprintf(buffer, "MISO/D%d", pin);
   else if (pin == SCK) sprintf(buffer, "SCK/D%d", pin);
   else if (pin == RX) sprintf(buffer, "RX/D%d", pin);
   else if (pin == TX) sprintf(buffer, "TX/D%d", pin);
   else if (pin == A0) sprintf(buffer, "A0/D%d", pin);
   else if (pin == A3) sprintf(buffer, "A3/D%d", pin);
   else if (pin == A6) sprintf(buffer, "A6/D%d", pin);
   else if (pin == A7) sprintf(buffer, "A7/D%d", pin);
   else if (pin == A18) sprintf(buffer, "A18/D%d", pin);
   else if (pin == A19) sprintf(buffer, "A19/D%d", pin);
   else if (pin >= 0 || pin < NUM_DIGITAL_PINS) sprintf(buffer, "D%d", pin);
   else if (pin < 0 || pin > 99) sprintf(buffer, "UNK/D--");
   else sprintf(buffer, "UNK/D%d", pin);

   return buffer;
}
