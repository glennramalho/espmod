/*******************************************************************************
 * io_mux.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements the ESP32 IO MUX.
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
#include "io_mux.h"
#include "info.h"

/*********************
 * Function: set_init_mode()
 * inputs: new mode
 * outputs: none
 * globals: none
 *
 * Sets the initial mode for a IO MUX. Takes a single value.
 */
void io_mux::set_init_mode(unsigned int newmode) {
   ie = ((newmode & GPIOMODE_INPUT)>0 || (newmode & GPIOMODE_INOUT)>0);
   oe = ((newmode & GPIOMODE_OUTPUT)>0 || (newmode & GPIOMODE_INOUT)>0);
   forceoe = false;
   wpu = (newmode & GPIOMODE_WPU);
   wpd = (newmode & GPIOMODE_WPD);
   od = (newmode & GPIOMODE_OD);
}

/*********************
 * Function: set_mode()
 * inputs: new mode
 * outputs: none
 * globals: none
 *
 * Sets the mode for a IO MUX. Takes a single value.
 */
bool io_mux::set_mode(unsigned int newmode) {
   bool resp = true;
   if ((newmode & GPIOMODE_INPUT)>0 || (newmode & GPIOMODE_INOUT)>0)
      resp = resp & set_ie();
   else resp = resp & clr_ie();
   if ((newmode & GPIOMODE_OUTPUT)>0 || (newmode & GPIOMODE_INOUT)>0)
      resp = resp & set_oe();
   else resp = resp & clr_oe();
   if ((newmode & GPIOMODE_WPU)>0) resp = resp & set_wpu();
   else resp = resp & clr_wpu();
   if ((newmode & GPIOMODE_WPD)>0) resp = resp & set_wpd();
   else resp = resp & clr_wpd();
   if ((newmode & GPIOMODE_OD)>0) resp = resp & set_od();
   else resp = resp & clr_od();
   return resp;
}

/*********************
 * Function: get_mode()
 * inputs: none
 * outputs: none
 * returns: current mode
 * globals: none
 *
 * Returns the current mode as a single value.
 */
unsigned int io_mux::get_mode() {
   unsigned int mode;
   if (ie && oe) mode = GPIOMODE_INOUT;
   else if (ie) mode = GPIOMODE_INPUT;
   else if (oe) mode = GPIOMODE_OUTPUT;
   else mode = GPIOMODE_NONE;
   if (wpu) mode = mode | GPIOMODE_WPU;
   if (wpd) mode = mode | GPIOMODE_WPD;
   if (od) mode = mode | GPIOMODE_OD;
   return mode;
}

/*********************
 * Function: set_ie()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the IE flag
 */
bool io_mux::set_ie() {
   if (!oe && (modes & GPIOMODE_INPUT)>0 || oe && (modes & GPIOMODE_INOUT)>0) {
      if (ie) return true;
      ie = true;
      PRINTF_INFO("IOMUX", "Set IE to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot set IE to pin %s", name());
   return false;
}

/*********************
 * Function: set_oe()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the OE flag
 */
bool io_mux::set_oe() {
   if (!ie && (modes & GPIOMODE_OUTPUT)>0 || ie && (modes & GPIOMODE_INOUT)>0) {
      if (oe) return true;
      oe = true;
      PRINTF_INFO("IOMUX", "Set OE to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot set OE to pin %s", name());
   return false;
}

/*********************
 * Function: set_forceoe()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the force OE flag
 */
bool io_mux::set_forceoe() {
   forceoe = true;
   updatedriver.notify();
   return true;
}

/*********************
 * Function: set_wpu()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the weak pull-up of a pin.
 */
bool io_mux::set_wpu() {
   if ((modes & GPIOMODE_WPU)>0) {
      if (wpu) return true;
      wpu = true;
      PRINTF_INFO("IOMUX", "Set WPU to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot set WPU to pin %s", name());
   return false;
}

/*********************
 * Function: clr_ie()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Clears the input enable
 */
bool io_mux::clr_ie() {
   if (!oe && (modes & GPIOMODE_NONE)>0 || oe && (modes & GPIOMODE_OUTPUT)>0) {
      if (!ie) return true;
      ie = false;
      PRINTF_INFO("IOMUX", "Cleared IE to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot clear IE to pin %s", name());
   return false;
}

/*********************
 * Function: clr_oe()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Clears the Output enable
 */
bool io_mux::clr_oe() {
   if (!ie && (modes & GPIOMODE_NONE)>0 || ie && (modes & GPIOMODE_INPUT)>0) {
      if (!oe) return true;
      oe = false;
      PRINTF_INFO("IOMUX", "Cleared OE to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot clear OE to pin %s", name());
   return false;
}

/*********************
 * Function: clr_forceoe()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Clears the OE force.
 */
bool io_mux::clr_forceoe() {
   forceoe = false;
   updatedriver.notify();
   return true;
}

/*********************
 * Function: clr_wpu()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Disables the weak pull-up of a pin.
 */
bool io_mux::clr_wpu() {
   if ((modes & GPIOMODE_NOWPU)==0) {
      if (!wpu) return true;
      wpu = false;
      PRINTF_INFO("IOMUX", "Clearing WPU on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot clear WPU on pin %s", name());
   return false;
}

/*********************
 * Function: set_wpd()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the weak pull-down of a pin.
 */
bool io_mux::set_wpd() {
   if ((modes & GPIOMODE_WPD)>0) {
      if (wpd) return true;
      wpd = true;
      PRINTF_INFO("IOMUX", "Set WPD to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot set WPU to pin %s", name());
   return false;
}

/*********************
 * Function: clr_wpd()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Disables the weak pull-down of a pin.
 */
bool io_mux::clr_wpd() {
   if ((modes & GPIOMODE_NOWPD)==0) {
      if (!wpd) return true;
      wpd = false;
      PRINTF_INFO("IOMUX", "Clearing WPD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot clear WPD on pin %s", name());
   return false;
}

/*********************
 * Function: set_od()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Enables the open-drain mode on a pin.
 */
bool io_mux::set_od() {
   if ((modes & GPIOMODE_OD)>0) {
      if (od) return true;
      od = true;
      PRINTF_INFO("IOMUX", "Setting OD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot set OD to pin %s", name());
   return false;
}

/*********************
 * Function: clr_od()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Disables the open-drain mode on a pin.
 */
bool io_mux::clr_od() {
   if ((modes & GPIOMODE_NOOD)==0) {
      if (!od) return true;
      od = false;
      PRINTF_INFO("IOMUX", "Clearing OD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("IOMUX", "Cannot clear OD on pin %s", name());
   return false;
}

/*********************
 * Function: get_ie()
 * inputs: none
 * outputs: none
 * returns: wpu
 * globals: none
 *
 * Gets the IE flag
 */
bool io_mux::get_ie() {
   return ie;
}

/*********************
 * Function: get_oe()
 * inputs: none
 * outputs: none
 * returns: wpu
 * globals: none
 *
 * Gets the OE flag
 */
bool io_mux::get_oe() {
   return oe;
}

/*********************
 * Function: get_forceoe()
 * inputs: none
 * outputs: none
 * returns: wpu
 * globals: none
 *
 * Gets the Force OE flag
 */
bool io_mux::get_forceoe() {
   return forceoe;
}

/*********************
 * Function: get_wpu()
 * inputs: none
 * outputs: none
 * returns: wpu
 * globals: none
 *
 * Gets the weak pull-up property of a pin.
 */
bool io_mux::get_wpu() {
   return wpu;
}

/*********************
 * Function: get_wpd()
 * inputs: none
 * outputs: none
 * returns: wpd
 * globals: none
 *
 * Gets the weak pull-down property of a pin.
 */
bool io_mux::get_wpd() {
   return wpd;
}

/*********************
 * Function: get_od()
 * inputs: none
 * outputs: none
 * returns: od
 * globals: none
 *
 * Gets the open-drain property of a pin.
 */
bool io_mux::get_od() {
   return od;
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
bool io_mux::get_val() {
   gn_mixed sample;

   /* Analog IO_MUX function always returns false. */
   if (get_function() == GPIOMF_ANALOG) return false;
   /* If IE is off, we report low. */
   else if (!ie) return false;
   /* Now we go with the resolved value. */
   else {
      /* We sample the pin. */
      sample = pin.read();
      switch(sample.logic.to_char()) {
         /* Strong 0 or Strong 1 we simply return. */
         case '1': return true;
         case '0': return false;
         /* Z is ok if we had a weak pull-up or down. */
         case 'Z':
            if (wpu) return true;
            else if (wpd) return false;
         /* X is always bad.  And so is Z on regular pins.  Well, actually
          * Z on regular inputs os ok as the latchup transistor will pull
          * the pin low eventually, but it is still unelegant.
          */
         default:
            PRINTF_WARN("IOMUX",
               "Sampling value '%c' on %s", sample.to_char(), name());
            /* We return low simply because we have to return something. */
            return false;
      }
   }
}

/*********************
 * Function: set_function()
 * inputs: new function
 * outputs: none
 * returns: none
 * globals: none
 *
 * Sets the function, can be GPIO or ANALOG.
 */
void io_mux::set_function(gpio_function_t newfunction) {
   /* We ignore unchanged function requests. */
   if (newfunction == function) return;
   /* We also ignore illegal function requests. */
   if (newfunction == GPIOMF_ANALOG && !anaok) {
      PRINTF_WARN("IOMUX", "%s cannot be set to ANALOG", name())
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
   /* The IOMUX does not have a GPIO function. */
   if (newfunction == GPIOMF_GPIO) {
      PRINTF_WARN("IOMUX", "%s cannot go to GPIO mode", name())
      return;
   }

   /* Analog drives the pin to nothing. */
   else if (newfunction == GPIOMF_ANALOG) {
      /* This set_mode is an elegance thing, it might not be necessary. */
      set_mode(GPIOMODE_NONE);
      PRINTF_INFO("IOMUX", "%s set to ANALOG", name())
      function = GPIOMF_ANALOG;
      driveok = false;
   }
   /* And with the multi function we let the drive_func handle the settings. */
   else {
      PRINTF_INFO("IOMUX", "%s set to FUNC%d", name(), newfunction)
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
gpio_function_t io_mux::get_function() {
   return function;
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
void io_mux::drive() {
   for(;;) {
      /* Analog function we disable everything, even weak signals. */
      if (get_function() == GPIOMF_ANALOG) pin.write(GN_LOGIC_Z);
      /* If WPU and WPD are high, we have a problem. */
      else if (wpu && wpd) pin.write(GN_LOGIC_X);
      /* If we are not driving, we still have to drive the weak pullups. */
      else if (driveok==false) {
         if (wpu) pin.write(GN_LOGIC_W1);
         else if (wpd) pin.write(GN_LOGIC_W0);
         else pin.write(GN_LOGIC_Z);
      }
      /* If we have a weak to drive, we drive it. */
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

/*********************
 * Thread: drive_return()
 * inputs: none
 * outputs: none
 * returns: none
 * globals: none
 *
 * Drives the return path from the pin onto the alternate functions.
 */
void io_mux::drive_return() {
   gn_mixed pinsamp;
   bool retval;
   int func;

   /* We begin driving all returns to low. */
   for (func = 0; func < fout.size(); func = func + 1) fout[func]->write(false);

   /* Now we go into the loop waiting for a return or a change in function. */
   for(;;) {
      pinsamp = pin.read();

      /* If the input is disabled, we return 0 regardless of what is on the pin.
       */
      if (!ie) retval = false;
      /* If the sampling value is Z or X and we have a function set, we
       * then issue a warning. We skip time zero though to eliminate some
       * rampup conditions.
       */
      else if (sc_time_stamp() != sc_time(0, SC_NS) &&
            (pinsamp == GN_LOGIC_A || pinsamp == GN_LOGIC_X
               || pinsamp == GN_LOGIC_Z) && function != GPIOMF_ANALOG) {
         retval = false;
         PRINTF_WARN("IOMUX", "can't return '%c' onto FUNC%d",
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
void io_mux::drive_func() {
   for(;;) {
      /* We only use this thread if we have an alternate function selected. */
      if (function != GPIOMF_GPIO && function != GPIOMF_ANALOG &&
            function-1 < fin.size()) {
         /* If the OE is forced, we follow the forced signal. */
         if (forceoe) driveok = oe;
         /* If OE is not forced, we follow the fen. */
         else if (function-1 < fen.size() && fen[function-1]->read() == true)
            driveok = true;
         else driveok = false;
         
         pindrive = fin[function-1]->read();
         updatedriver.notify();
      }
      else {
         driveok = false;
         pindrive = false;
         updatedriver.notify();
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
      else if (function-1 >= fen.size())
         wait(updatefunc | fin[function-1]->value_changed_event());
      else wait(updatefunc | fin[function-1]->value_changed_event()
         | fen[function-1]->value_changed_event());
   }
}
