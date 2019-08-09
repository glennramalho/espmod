/*******************************************************************************
 * gpio_base.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a base class for a SystemC GPIO model.
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
#include "gpio_base.h"
#include "info.h"

/*********************
 * Function: printgpiomode()
 * input: pin direction, enable weak pull-up, enable weak pull-down, enable OD
 * output: none
 * return: mode string
 * globals: none
 *
 * Returns a string to use in debug messages with the given pin mode.
 */
const char *printgpiomode(gpio_dir_t dir, bool wpu, bool wpd, bool od) {
   switch(dir) {
      case GPIODIR_NONE:
         if (wpu && wpd) return "at Vcc/2";
         else if (wpu) return "weak pullup";
         else if (wpd) return "weak pulldown";
         return "floating pin";
      case GPIODIR_INPUT:
         if (wpu && wpd) return "at Vcc/2";
         else if (wpu) return "weak PU input";
         else if (wpd) return "weak PD input";
         return "input";
      case GPIODIR_OUTPUT:
         if (od) return "open drain output";
         else if (wpu && wpd) return "at Vcc/2";
         else if (wpu) return "weak PU output";
         else if (wpd) return "weak PD output";
         return "output";
      case GPIODIR_INOUT:
         if (od) return "open in/out";
         else if (wpu && wpd) return "at Vcc/2";
         else if (wpu) return "weak PU in/out";
         else if (wpd) return "weak PD in/out";
         return "regular in/out";
      default: return "Invalid";
   }
}

/*********************
 * Function: printgpiomode()
 * inputs: none
 * outputs: none
 * return: mode string
 * globals: none
 *
 * Returns a string to use in debug messages with the given pin mode.
 */
const char *printgpiomode(unsigned int mode) {
   gpio_dir_t dir;
   switch(GPIOMODE_DIR(mode)) {
      case GPIOMODE_NONE: dir = GPIODIR_NONE; break;
      case GPIOMODE_INPUT: dir = GPIODIR_INPUT; break;
      case GPIOMODE_OUTPUT: dir = GPIODIR_OUTPUT; break;
      case GPIOMODE_INOUT: dir = GPIODIR_INOUT; break;
      default: dir = GPIODIR_INVALID; break;
   }

   return printgpiomode(dir, (mode & GPIOMODE_WPU)>0,
      (mode & GPIOMODE_WPD)>0, (mode & GPIOMODE_OD)>0);
}

/*********************
 * Function: set_init_mode()
 * inputs: new mode
 * outputs: none
 * globals: none
 *
 * Sets the initial mode for a GPIO. Takes a single value.
 */
void gpio_base::set_init_mode(unsigned int newmode) {
   if ((modes & GPIOMODE_DIR(newmode))==0
         || (newmode & GPIOMODE_WPU)>0 && (modes & GPIOMODE_WPU)==0
         || (newmode & GPIOMODE_WPU)==0 && (modes & GPIOMODE_NOWPU)>0
         || (newmode & GPIOMODE_WPD)>0 && (modes & GPIOMODE_WPD)==0
         || (newmode & GPIOMODE_WPD)==0 && (modes & GPIOMODE_NOWPD)>0
         || (newmode & GPIOMODE_OD)>0 && (modes & GPIOMODE_OD)==0
         || (newmode & GPIOMODE_OD)==0 && (modes & GPIOMODE_NOOD)>0)
      {
      PRINTF_WARN("GPIO", "Can't set pin %s to mode %s", 
         name(), printgpiomode(newmode));
   }
   else {
      wpu = (newmode & GPIOMODE_WPU);
      wpd = (newmode & GPIOMODE_WPD);
      od = (newmode & GPIOMODE_OD);
      switch(GPIOMODE_DIR(newmode)) {
         case GPIOMODE_NONE: dir = GPIODIR_NONE; break;
         case GPIOMODE_INPUT: dir = GPIODIR_INPUT; break;
         case GPIOMODE_OUTPUT: dir = GPIODIR_OUTPUT; break;
         default: dir = GPIODIR_INOUT; break;
      }
   }
}

/*********************
 * Function: set_mode()
 * inputs: new mode
 * outputs: none
 * globals: none
 *
 * Sets the mode for a GPIO. Takes a single value.
 */
bool gpio_base::set_mode(unsigned int newmode) {
   bool resp;
   switch(GPIOMODE_DIR(newmode)) {
      case GPIOMODE_INPUT: resp = set_dir(GPIODIR_INPUT); break;
      case GPIOMODE_OUTPUT: resp = set_dir(GPIODIR_OUTPUT); break;
      case GPIOMODE_INOUT: resp = set_dir(GPIODIR_INOUT); break;
      default: resp = set_dir((gpio_dir_t)newmode); break;
   }
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
unsigned int gpio_base::get_mode() {
   unsigned int mode;
   switch(get_dir()) {
      case GPIODIR_INOUT: mode = GPIOMODE_INOUT; break;
      case GPIODIR_INPUT: mode = GPIOMODE_INPUT; break;
      case GPIODIR_OUTPUT: mode = GPIOMODE_OUTPUT; break;
      default: mode = GPIOMODE_NONE; break;
   }
   if (wpu) mode = mode | GPIOMODE_WPU;
   if (wpd) mode = mode | GPIOMODE_WPD;
   if (od) mode = mode | GPIOMODE_OD;
   return mode;
}

/*********************
 * Function: get_dir()
 * inputs: none
 * outputs: none
 * returns: current direction
 * globals: none
 *
 * Returns the current direction of the pin.
 */
gpio_dir_t gpio_base::get_dir() {
   return dir;
}

/*********************
 * Function: set_dir()
 * inputs: new direction
 * outputs: none
 * returns: none
 * globals: none
 *
 * Sets the new direction for a pin. The other parameters are left unchanged.
 */
bool gpio_base::set_dir(gpio_dir_t newdir) {
   const char *dirstr;
   unsigned int newmode;

   /* If the pin direction is not changing, we just return. We return true
    * though as it was successful.
    */
   if (dir == newdir) return true;

   /* We get a direction string so that we can print it. */
   switch(newdir) {
      case GPIODIR_NONE: dirstr = "NONE"; newmode = GPIOMODE_NONE; break;
      case GPIODIR_INPUT: dirstr = "INPUT"; newmode = GPIOMODE_INPUT; break;
      case GPIODIR_OUTPUT: dirstr = "OUTPUT"; newmode = GPIOMODE_OUTPUT; break;
      case GPIODIR_INOUT: dirstr = "BI-DIR"; newmode = GPIOMODE_INOUT; break;
      default: dirstr = "unknown dir"; newmode = GPIOMODE_NONE; break;
   }

   /* We first check that the direction is allowed. If not, we complain. */
   if ((modes & newmode)==0) {
      /* The pin does not have this option. We then need to complain. */
      PRINTF_WARN("GPIO", "Cannot set pin %s to %s", name(), dirstr);
      return false;
   }

   /* The value changed, so we issue an info and notify the driver. */
   dir = newdir;
   PRINTF_INFO("GPIO", "Setting pin %s to %s", name(), dirstr);
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
bool gpio_base::set_wpu() {
   if ((modes & GPIOMODE_WPU)>0) {
      if (wpu) return true;
      wpu = true;
      PRINTF_INFO("GPIO", "Set WPU to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot set WPU to pin %s", name());
   return false;
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
bool gpio_base::clr_wpu() {
   if ((modes & GPIOMODE_NOWPU)==0) {
      if (!wpu) return true;
      wpu = false;
      PRINTF_INFO("GPIO", "Clearing WPU on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot clear WPU on pin %s", name());
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
bool gpio_base::set_wpd() {
   if ((modes & GPIOMODE_WPD)>0) {
      if (wpd) return true;
      wpd = true;
      PRINTF_INFO("GPIO", "Set WPD to pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot set WPU to pin %s", name());
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
bool gpio_base::clr_wpd() {
   if ((modes & GPIOMODE_NOWPD)==0) {
      if (!wpd) return true;
      wpd = false;
      PRINTF_INFO("GPIO", "Clearing WPD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot clear WPD on pin %s", name());
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
bool gpio_base::set_od() {
   if ((modes & GPIOMODE_OD)>0) {
      if (od) return true;
      od = true;
      PRINTF_INFO("GPIO", "Setting OD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot set OD to pin %s", name());
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
bool gpio_base::clr_od() {
   if ((modes & GPIOMODE_NOOD)==0) {
      if (!od) return true;
      od = false;
      PRINTF_INFO("GPIO", "Clearing OD on pin %s", name());
      /* We might have to drive a value. */
      updatedriver.notify();

      return true;
   }

   /* Other combinations, not allowed or illegal, we report here. */
   PRINTF_WARN("GPIO", "Cannot clear OD on pin %s", name());
   return false;
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
bool gpio_base::get_wpu() {
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
bool gpio_base::get_wpd() {
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
bool gpio_base::get_od() {
   return od;
}

/*********************
 * Function: set_val()
 * inputs: new lvl
 * outputs: none
 * returns: none
 * globals: none
 *
 * Sets the level to drive onto the pin.
 */
void gpio_base::set_val(bool newval) {
   pindrive = newval;
   updatedriver.notify();
}
