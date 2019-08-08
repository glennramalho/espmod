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

#ifndef _GPIO_MFMIX_H
#define _GPIO_MFMIX_H

#include <systemc.h>
#include "gpio_mix.h"
#include "gn_mixed.h"

class gpio_mfmix : public gpio_mix {
   public:
   /* We inherit the main i/o pin and add the function pins. */
   sc_port< sc_signal_in_if<bool>,0 > fin {"fin"};
   sc_port< sc_signal_in_if<bool>,0 > fen {"fen"};
   sc_port< sc_signal_out_if<bool>,0 > fout {"fout"};

   /* Multifunction specific tasks */
   virtual void set_function(gpio_function_t newfunction);
   virtual gpio_function_t get_function();

   /* Threads */
   sc_event updatefunc; /* Triggers a function change. */
   sc_event updatereturn; /* Triggers a feedback drive event. */
   void drive_func(); /* Updates the function drive. */
   void drive_return(); /* Updates feedback pins to functions */

   /* Sets initial drive condition. */
   gpio_mfmix(sc_module_name name, int optargs, int initial,
         bool hasana = false, gpio_function_t initialfunc = GPIOMF_GPIO):
         gpio_mix(name, optargs, initial, hasana, initialfunc) {

      SC_THREAD(drive_return);
      sensitive << pin << updatereturn;

      /* This one has no sensitivity list as it varies according to the
       * selected function.
       */
      SC_THREAD(drive_func);
   }
   SC_HAS_PROCESS(gpio_mfmix);

   virtual gpio_type_t get_type() const {return GPIO_TYPE_MFMIX; }
};

#endif
