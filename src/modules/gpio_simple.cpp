/*******************************************************************************
 * gpio_simple.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a simple, single-function GPIO.
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
#include "gpio_simple.h"
#include "info.h"

/*********************
 * Function: set_function()
 * inputs: new function
 * outputs: none
 * returns: none
 * globals: none
 *
 * This is a single-function GPIO. This function is here just for completeness.
 */
void gpio::set_function(gpio_function_t function) {
   if (function != GPIOMF_GPIO)
      PRINTF_WARN("GPIO", "%s is single-function", name())
}

/*********************
 * Function: get_function()
 * inputs: none
 * outputs: none
 * returns: current function
 * globals: none
 *
 * Returns the function for this GPIOs.
 */
gpio_function_t gpio::get_function() {
   return GPIOMF_GPIO;
}

/*********************
 * Function: get_val()
 * inputs: none
 * outputs: none
 * returns: current level
 * globals: none
 *
 * Samples the current level on the pin.
 */
bool gpio::get_val() {
   sc_logic sample;

   /* Analog GPIO function always returns false. */
   if (get_function() == GPIOMF_ANALOG) return false;
   /* This depends on the implementation, we are going with pin as output only
    * returns false as the feedback is disabled.
    */
   else if (dir == GPIODIR_OUTPUT || dir == GPIODIR_NONE) return false;
   /* Now we go with the resolved value. */
   else {
      /* We sample the pin. */
      sample = pin.read();
      switch(sample.to_char()) {
         /* Strong 0 or Strong 1 we simply return. */
         case '1': return true;
         case '0': return false;
         /* Z is ok if we had a weak pull-up or down. */
         case 'Z':
            if (wpu) return true;
            else if (wpd) return false;
            /* And the remaining cases fall through to the next case. */
         /* X is always bad.  And so is Z on regular pins.  Well, actually
          * Z on regular inputs os ok as the latchup transistor will pull
          * the pin low eventually, but it is still unelegant.
          */
         default:
            PRINTF_WARN("GPIO",
               "Sampling value '%c' on %s", sample.to_char(), name());
            /* We return low simply because we have to return something. */
            return false;
      }
   }
}

/*********************
 * Thread: drive()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Drives the current level onto the I/O pad.
 */
void gpio::drive() {
   for(;;) {
      /* First we check if the output is enabled. If it is not, we drive the
       * weak driver. If we are in NONE mode or INPUT mode the pin is also
       * disabled.
       */
      if (driveok==false || dir == GPIODIR_NONE || dir==GPIODIR_INPUT) {
         /* SystemC does not support weak0/1, so we drive Z and ignore the pin
          * value. We will flag both WPU and WPD together but any other case
          * must be done externally via a pull-up.
          */
         if (wpu && wpd) pin.write(SC_LOGIC_X);
         pin.write(SC_LOGIC_Z);
      }
      /* WPU and WPD together is just bad. */
      else if (wpu && wpd) pin.write(SC_LOGIC_X);
      /* SystemC does not support weak0/1, so we put Z */
      else if (pindrive == true && wpu) pin.write(SC_LOGIC_Z);
      else if (pindrive == false && wpd) pin.write(SC_LOGIC_Z);
      /* If we are driving high and we are in OD mode, we drive Z. OD mode
       * drives only low.
       */
      else if (pindrive == true && od) pin.write(SC_LOGIC_Z);
      /* And on the remaining cases, we drive the value onto the pin. */
      else if (pindrive == true) pin.write(SC_LOGIC_1);
      else pin.write(SC_LOGIC_0);

      wait();
   }
}
