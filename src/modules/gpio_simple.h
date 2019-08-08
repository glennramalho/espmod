/*******************************************************************************
 * gpio_simple.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#ifndef _GPIO_SIMPLE_H
#define _GPIO_SIMPLE_H

#include <systemc.h>
#include "gpio_base.h"

class gpio : public gpio_base {
   public:
   sc_inout_resolved pin {"pin"};

   /* Multifunction specific tasks */
   virtual void set_function(gpio_function_t newfunction);
   virtual gpio_function_t get_function();
   /* set_val() is inherited from the base class. */
   virtual bool get_val();

   /* Tasks */
   void drive();  /* Internal task which updates values. */

   /* Sets initial drive condition. */
   gpio(sc_module_name name, int optargs, int initial):
         gpio_base(name, optargs, initial) {
      SC_THREAD(drive);
      sensitive << updatedriver;
   }
   SC_HAS_PROCESS(gpio);

   virtual gpio_type_t get_type() const {return GPIO_TYPE_SIMPLE; }
};

#endif
