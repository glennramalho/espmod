/*******************************************************************************
 * gpio_base.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#ifndef _GPIO_BASE_H
#define _GPIO_BASE_H

#include <systemc.h>

/* The allowed modes are below. When defining the allowed modes, OR the codes
 * as needed.
 */
#define GPIOMODE_NONE      0x0001U
#define GPIOMODE_INPUT     0x0002U
#define GPIOMODE_OUTPUT    0x0004U
#define GPIOMODE_INOUT     0x0008U
#define GPIOMODE_WPU       0x0010U
#define GPIOMODE_WPD       0x0020U
#define GPIOMODE_OD        0x0040U
#define GPIOMODE_NOWPU     0x0100U
#define GPIOMODE_NOWPD     0x0200U
#define GPIOMODE_NOOD      0x0400U
#define GPIOMODE_INVALID   0x0000U
#define GPIOMODE_DIR(x)    ((x) & 0x000FU)

/* Function returns a string for a given mode code. Just one code can be given
 * or it returns invalid.
 */
const char *printgpiomode(unsigned int dir, bool wpu, bool wpd, bool od);
const char *printgpiomode(unsigned int mode);

#define GPIOMF_ANALOG -1
#define GPIOMF_GPIO 0
#define GPIOMF_FUNCTION 1
typedef int gpio_function_t;
typedef enum {
      GPIODIR_NONE = GPIOMODE_NONE,
      GPIODIR_INPUT = GPIOMODE_INPUT,
      GPIODIR_OUTPUT = GPIOMODE_OUTPUT,
      GPIODIR_INOUT = GPIOMODE_INOUT,
      GPIODIR_INVALID = GPIOMODE_INVALID} gpio_dir_t;
typedef enum {GPIO_TYPE_SIMPLE, GPIO_TYPE_MF, GPIO_TYPE_MIX,
   GPIO_TYPE_MFMIX} gpio_type_t;

SC_MODULE(gpio_base) {
   /* Sets and samples mode */
   bool set_mode(unsigned int newmode);
   unsigned int get_mode();
   gpio_dir_t get_dir();
   bool set_dir(gpio_dir_t newdir);
   bool set_wpu();
   bool set_wpd();
   bool set_od();
   bool clr_wpu();
   bool clr_wpd();
   bool clr_od();
   bool get_wpu();
   bool get_wpd();
   bool get_od();

   /* The single function GPIO will return errors if the user attempts to
    * change the function. */
   virtual void set_function(gpio_function_t newfunction) = 0;
   virtual gpio_function_t get_function() = 0;

   /* Sets and samples val */
   virtual void set_val(bool newval);
   virtual bool get_val() = 0;

   /* Tasks */
   sc_event updatedriver;    /* Used to trigger the drive() task */

   /* Sets initial drive condition. */
   gpio_base(sc_module_name name, int optargs, int initial) {
      pindrive = false;
      driveok = true;
      modes = optargs;
      set_init_mode(initial);
   }

   virtual gpio_type_t get_type() const = 0;

   protected:
   bool pindrive;        /* Current value being driven */
   bool driveok;         /* Indicates that a value should be driven. */
   bool wpu, wpd, od;
   gpio_dir_t dir;
   unsigned int modes;   /* Allowed modes */
   void set_init_mode(unsigned int newmode);
};

#endif
