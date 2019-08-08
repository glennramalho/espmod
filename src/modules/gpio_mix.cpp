/*******************************************************************************
 * gpio_mix.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a simple, single-function GPIO with analog.
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
#include "gpio_mix.h"
#include "info.h"

/*********************
 * Function: set_function()
 * inputs: new function
 * outputs: none
 * returns: none
 * globals: none
 *
 * Sets the function, can be GPIO or ANALOG.
 */
void gpio_mix::set_function(gpio_function_t newfunction) {
   /* We ignore unchanged function requests. */
   if (newfunction == function) return;
   /* We also ignore illegal function requests. */
   if (newfunction == GPIOMF_ANALOG && !anaok) {
      PRINTF_WARN("GPIOMFMIX", "%s cannot be set to ANALOG", name())
      return;
   }
   if (newfunction >= GPIOMF_FUNCTION) {
      PRINTF_WARN("GPIOMIX", "%s is single-function or analog", name())
      return;
   }

   /* When we change to the GPIO function, we set the driver high and set
    * the function accordingly.
    */
   if (newfunction == GPIOMF_GPIO) {
      PRINTF_INFO("GPIOMIX", "%s set to GPIO mode", name());
      function = newfunction;
      driveok = true;
   }
   /* Analog drives the pin to nothing. */
   else {
      /* This set_mode is an elegance thing, it might not be necessary. */
      set_mode(GPIOMODE_NONE);
      PRINTF_INFO("GPIOMIX", "%s set to ANALOG", name());
      function = GPIOMF_ANALOG;
      driveok = false;
   }

   updatedriver.notify();
}

/*********************
 * Function: get_function()
 * inputs: none
 * outputs: none
 * returns: current function
* globals: none
 *
 * Returns the current function.
 */
gpio_function_t gpio_mix::get_function() {
   return function;
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
bool gpio_mix::get_val() {
   gn_mixed sample;

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
         case 'A':
            PRINTF_WARN("Sampling an Analog signal %s", name());
            /* We return low simply because we have to return something. */
            return false;
         case 'Z':
            if (wpu) return true;
            else if (wpd) return false;
            /* And the remaining cases fall through to the next case. */
         /* X is always bad.  And so is Z on regular pins.  Well, actually
          * Z on regular inputs os ok as the latchup transistor will pull
          * the pin low eventually, but it is still unelegant.
          */
         default:
            PRINTF_WARN("GPIOMIX",
               "Sampling value '%c' on %s", sample.to_char(), name());
            /* We return low simply because we have to return something. */
            return false;
      }
   }
}

/*********************
 * Function: set_val()
 * inputs: new value
 * outputs: none
 * returns: none
 * globals: none
 *
 * Changes the value to set onto the GPIO, if GPIO function is set.
 */
void gpio_mix::set_val(bool newval) {
   pinval = newval;
   if (function == GPIOMF_GPIO) gpio_base::set_val(newval);
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
void gpio_mix::drive() {
   for(;;) {
      /* Analog function we disable everything, even weak signals. */
      if (get_function() == GPIOMF_ANALOG) pin.write(GN_LOGIC_Z);
      /* First we check if the output is enabled. If it is not, we drive the
       * weak driver. If we are in NONE mode or INPUT mode the pin is also
       * disabled.
       */
      else if (driveok==false || dir==GPIODIR_NONE || dir==GPIODIR_INPUT) {
         if (wpu && wpd) pin.write(GN_LOGIC_X);
         else if (wpu) pin.write(GN_LOGIC_W1);
         else if (wpd) pin.write(GN_LOGIC_W0);
         else pin.write(GN_LOGIC_Z);
      }
      /* WPU and WPD together is just bad. */
      else if (wpu && wpd) pin.write(GN_LOGIC_X);
      else if (pindrive == true && wpu) pin.write(GN_LOGIC_W1);
      else if (pindrive == false && wpd) pin.write(GN_LOGIC_W0);
      /* If we are driving high and we are in OD mode, we drive Z. OD mode
       * drives only low.
       */
      else if (pindrive == true && od) pin.write(GN_LOGIC_Z);
      /* And on the remaining cases, we drive the value onto the pin. */
      else if (pindrive == true) pin.write(GN_LOGIC_1);
      else pin.write(GN_LOGIC_0);

      wait();
   }
}
