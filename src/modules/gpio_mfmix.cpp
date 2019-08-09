/*******************************************************************************
 * gpio_mfmix.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a generic multi-function GPIO with
 * analog function.
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
#include "gpio_mfmix.h"
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
void gpio_mfmix::set_function(gpio_function_t newfunction) {
   /* We ignore unchanged function requests. */
   if (newfunction == function) return;
   /* We also ignore illegal function requests. */
   if (newfunction == GPIOMF_ANALOG && !anaok) {
      PRINTF_WARN("GPIOMFMIX", "%s cannot be set to ANALOG", name())
      return;
   }
   /* To select a function there must be at least the fin or the fout
    * available. The fen is optional. Ideal would be to require all three
    * but there are some peripherals that need only one of these pins,
    * so to make life easier, we require at least a fin or a fout.
    */
   if (newfunction >= GPIOMF_FUNCTION &&
         (newfunction-1 >= fin.size() && newfunction-1 >= fout.size())) {
      PRINTF_WARN("GPIOMF", "%s cannot be set to FUNC%d", name(),
         newfunction)
      return;
   }

   /* When we change to the GPIO function, we set the driver high and set
    * the function accordingly.
    */
   if (newfunction == GPIOMF_GPIO) {
      PRINTF_INFO("GPIOMFMIX", "%s set to GPIO mode", name())
      function = newfunction;
      driveok = true;
      /* set_val() handles the driver. */
      gpio_base::set_val(pinval);
   }
   /* Analog drives the pin to nothing. */
   else if (newfunction == GPIOMF_ANALOG) {
      /* This set_mode is an elegance thing, it might not be necessary. */
      set_mode(GPIOMODE_NONE);
      PRINTF_INFO("GPIOMFMIX", "%s set to ANALOG", name())
      function = GPIOMF_ANALOG;
      driveok = false;
   }
   /* And with the multi function we let the drive_func handle the settings. */
   else {
      PRINTF_INFO("GPIOMFMIX", "%s set to FUNC%d", name(), newfunction)
      function = newfunction;
   }

   /* And we set any notifications we need. */
   updatefunc.notify();
   updatereturn.notify();
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
gpio_function_t gpio_mfmix::get_function() {
   return function;
}

/*********************
 * Thread: drive_return()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Drives the return path from the pin onto the alternate functions.
 */
void gpio_mfmix::drive_return() {
   gn_mixed pinsamp;
   bool retval;
   int func;

   /* We begin driving all returns to low. */
   for (func = 0; func < fout.size(); func = func + 1) fout[func]->write(false);

   /* Now we go into the loop waiting for a return or a change in function. */
   for(;;) {
      /*
      printf("<<%s>>: U:%d pin:%d:%c @%s\n",
         name(), updatereturn.triggered(), pin.event(), pin.read().to_char(),
         sc_time_stamp().to_string().c_str());
      */
      pinsamp = pin.read();
      /* If the sampling value is Z or X and we have a function set, we
       * then issue a warning. We also do not warn at time 0s or we get some
       * dummy initialization warnings.
       */
      if (sc_time_stamp() != sc_time(0, SC_NS) &&
            (pinsamp == GN_LOGIC_A || pinsamp == GN_LOGIC_X
               || pinsamp == GN_LOGIC_Z) &&
            function != GPIOMF_ANALOG && function != GPIOMF_GPIO) {
         retval = false;
         PRINTF_WARN("GPIOMFMIX", "can't return '%c' onto FUNC%d",
            pinsamp.to_char(), function)
      }
      else if (pinsamp == GN_LOGIC_1) retval = true;
      else retval = false;

      for (func = 0; func < fout.size(); func = func + 1)
         if (function == GPIOMF_ANALOG) fout[func]->write(false);
         else if (function == GPIOMF_GPIO) fout[func]->write(false);
         else if (func != function-1) fout[func]->write(false);
         else fout[func]->write(retval);

      wait();
   }
}

/*********************
 * Thread: drive_func()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Drives the value from an alternate function onto the pins. This does not
 * actually drive the value, it just places it in the correct variables so that
 * the drive() thread handles it.
 */
void gpio_mfmix::drive_func() {
   for(;;) {
      /* We only use this thread if we have an alternate function selected. */
      if (function != GPIOMF_GPIO && function != GPIOMF_ANALOG &&
            function-1 < fin.size()) {
         /* We check if the fen is ok. If this function has no fen we default
          * it to enabled.
          */
         if (function-1 < fen.size() || fen[function-1]->read() == true)
            driveok = true;
         else driveok = false;

         /* Note that we do not set the pin val, just call set_val to handle
          * the pin drive.
          */
         gpio_base::set_val(fin[function-1]->read());
      }

      /* We now wait for a change. If the function is no selected or if we
       * have an illegal function selected, we have to wait for a change
       * in the function selection.
       */
      if (function == GPIOMF_ANALOG || function == GPIOMF_GPIO ||
            function-1 >= fin.size())
         wait(updatefunc);
      /* If we have a valid function selected, we wait for either the
       * function or the fen to change. We also have to wait for a
       * function change.
       */
      else if (fen.size() >= function-1)
         wait(updatefunc | fin[function-1]->value_changed_event());
      else wait(updatefunc | fin[function-1]->value_changed_event()
         | fen[function-1]->value_changed_event());
   }
}
