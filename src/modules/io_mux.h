/*******************************************************************************
 * gpio_mfmix.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a generic multi-function GPIO with variable
 * number of GPIO functions and analog function.
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

#ifndef _IO_MUX_H
#define _IO_MUX_H

#include <systemc.h>
#include "gpio_base.h"
#include "gn_mixed.h"

SC_MODULE(io_mux) {
   sc_inout<gn_mixed> pin {"pin"};

   sc_port< sc_signal_in_if<bool>,0 > fin {"fin"};
   sc_port< sc_signal_in_if<bool>,0 > fen {"fen"};
   sc_port< sc_signal_out_if<bool>,0 > fout {"fout"};

   /* Multifunction specific tasks */
   void set_function(gpio_function_t newfunction);
   gpio_function_t get_function();

   /* Pin driver functions */
   bool set_mode(unsigned int newmode);
   unsigned int get_mode();
   bool set_oe();
   bool set_ie();
   bool set_forceoe();
   bool set_wpu();
   bool set_wpd();
   bool set_od();
   bool clr_ie();
   bool clr_oe();
   bool clr_forceoe();
   bool clr_wpu();
   bool clr_wpd();
   bool clr_od();
   bool get_ie();
   bool get_oe();
   bool get_forceoe();
   bool get_wpu();
   bool get_wpd();
   bool get_od();

   /* Samples val */
   bool get_val();

   /* Tasks */
   sc_event updatedriver;    /* Used to trigger the drive() task */
   sc_event updatefunc; /* Triggers a function change. */
   sc_event updatereturn; /* Triggers a feedback drive event. */
   void drive(); /* Updates the output. */
   void drive_func(); /* Updates the function drive. */
   void drive_return(); /* Updates feedback pins to functions */

   /* Sets initial drive condition. */
   io_mux(sc_module_name name, int optargs, int initial,
         bool hasana = false, gpio_function_t initialfunc = GPIOMF_GPIO) {
      pindrive = false;
      function = initialfunc;
      anaok = hasana;
      driveok = true;
      modes = optargs;
      set_init_mode(initial);

      SC_THREAD(drive);
      sensitive << updatedriver;

      SC_THREAD(drive_return);
      sensitive << pin << updatereturn;

      /* This one has no sensitivity list as it varies according to the
       * selected function.
       */
      SC_THREAD(drive_func);
   }
   SC_HAS_PROCESS(io_mux);

   protected:
   bool pindrive;        /* Current value being driven */
   bool driveok;         /* Indicates that a value should be driven. */
   bool anaok;
   int function;
   bool ie, oe, wpu, wpd, od, forceoe; /* flags */
   unsigned int modes;   /* Allowed modes */
   void set_init_mode(unsigned int newmode);
};

#endif
