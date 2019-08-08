/*******************************************************************************
 * gpio_mix.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a GPIO with an alternate analog function.
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

#ifndef _GPIO_MIX_H
#define _GPIO_MIX_H

#include <systemc.h>
#include "gpio_base.h"
#include "gn_mixed.h"

class gpio_mix : public gpio_base {
   public:
   sc_inout<gn_mixed> pin {"pin"};

   /* Multifunction specific tasks */
   virtual void set_function(gpio_function_t newfunction);
   virtual gpio_function_t get_function();
   virtual void set_val(bool newval);
   virtual bool get_val();

   /* Tasks */
   void drive();  /* Internal task which updates values. */

   /* Sets initial drive condition. */
   gpio_mix(sc_module_name name, int optargs, int initial,
         bool hasana = false, gpio_function_t initialfunc = GPIOMF_GPIO):
         gpio_base(name, optargs, initial) {

      pinval = false;
      function = initialfunc;
      anaok = hasana;

      SC_THREAD(drive);
      sensitive << updatedriver;
   }
   SC_HAS_PROCESS(gpio_mix);

   virtual gpio_type_t get_type() const {return GPIO_TYPE_MIX; }

   protected:
   bool anaok;               /* Defines that analog function is available. */
   gpio_function_t function; /* Indicates internal selected function */
   bool pinval;              /* Indicates intended drive value for pin */
};

#endif
