/*******************************************************************************
 * espintr.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 interrupt management.
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
#include "info.h"
#include "espintr.h"
#include "soc/soc.h"
#include "esp_intr_alloc.h"
#include "Arduino.h"

void espintr::initialize() {
   int i;
   for (i = 0; i < ESPM_INTR_TABLE; i = i + 1) {
      table_pro[i] = -1;
      table_app[i] = -1;
   }
   for (i = 0; i < XCHAL_NUM_INTERRUPTS; i = i + 1) {
      handler_pro[i].fn = NULL;
      handler_pro[i].arg = NULL;
      handler_app[i].fn = NULL;
      handler_app[i].arg = NULL;
   }
}

void espintr::catcher() {
   uint32_t pro_lvl, app_lvl;
   while(true) {
      wait();
      /* We collect all signals into one set of busses. */
      pro_lvl = 0;
      app_lvl = 0;

      if (ledc_intr_i.read() && table_pro[ETS_LEDC_INTR_SOURCE] != -1)
         pro_lvl = pro_lvl | (1<<table_pro[ETS_LEDC_INTR_SOURCE]);
      if (ledc_intr_i.read() && table_app[ETS_LEDC_INTR_SOURCE] != -1)
         app_lvl = app_lvl | (1<<table_app[ETS_LEDC_INTR_SOURCE]);

      /* Once we are done we set the new values for raw and intr. */
      raw_pro.write(pro_lvl);
      intr_pro.write(mask_pro.read() & pro_lvl);
      raw_app.write(app_lvl);
      intr_app.write(mask_pro.read() & app_lvl);
   }
}

int espintr::get_next(uint32_t edgemask, uint32_t now, uint32_t last) {
   /* Any bits that are edge triggered, we take the value for last. Any that
    * are not, we take the inverse of now forcing an edge trigger.
    */
   uint32_t lm = last & edgemask | ~now & ~edgemask;
   /* Priority NMI */
   if ((now & (1<<14))>0) return 14;
   /* Priority 5 */
   if ((now & ~last & (1<<16))>0) return 16; /* Timer style */
   if ((now & ~lm & (1<<26))>0) return 26;
   if ((now & ~lm & (1<<31))>0) return 31;
   /* Priority 4 */
   if ((now & ~lm & (1<<24))>0) return 24;
   if ((now & ~lm & (1<<25))>0) return 25;
   if ((now & ~lm & (1<<28))>0) return 28;
   if ((now & ~lm & (1<<30))>0) return 30;
   /* Priority 3 */
   if ((now & ~last & (1<<11))>0) return 11; /* Profiling */
   if ((now & ~last & (1<<15))>0) return 15; /* Timer */
   if ((now & ~lm & (1<<22))>0) return 22;
   if ((now & ~lm & (1<<23))>0) return 23;
   if ((now & ~lm & (1<<27))>0) return 27;
   if ((now & ~last & (1<<29))>0) return 29; /* Software */
   /* Priority 2 */
   if ((now & ~lm & (1<<19))>0) return 19;
   if ((now & ~lm & (1<<20))>0) return 20;
   if ((now & ~lm & (1<<21))>0) return 21;
   /* Priority 1 */
   if ((now & ~lm & (1<<0))>0) return 0;
   if ((now & ~lm & (1<<1))>0) return 1;
   if ((now & ~lm & (1<<2))>0) return 2;
   if ((now & ~lm & (1<<3))>0) return 3;
   if ((now & ~lm & (1<<4))>0) return 4;
   if ((now & ~lm & (1<<5))>0) return 5;
   if ((now & ~last & (1<<6))>0) return 6; /* Timer */
   if ((now & ~last & (1<<7))>0) return 7; /* Software */
   if ((now & ~lm & (1<<8))>0) return 8;
   if ((now & ~lm & (1<<9))>0) return 9;
   if ((now & ~lm & (1<<10))>0) return 10;
   if ((now & ~lm & (1<<12))>0) return 12;
   if ((now & ~lm & (1<<13))>0) return 13;
   if ((now & ~lm & (1<<17))>0) return 17;
   if ((now & ~lm & (1<<18))>0) return 18;
   return -1;
}

void espintr::driver_app() {
   uint32_t last = 0;
   int nextintr;
   while(true) {
      wait();
      nextintr = get_next(edge_interrupt_app.read(), intr_app.read(), last);
      last = intr_app.read();
      if (nextintr >= 0 && handler_app[nextintr].fn != NULL)
         (*handler_pro[nextintr].fn)(handler_pro[nextintr].arg);
   }
}

void espintr::driver_pro() {
   uint32_t last = 0;
   int nextintr;
   while(true) {
      wait();
      nextintr = get_next(edge_interrupt_pro.read(), intr_pro.read(), last);
      last = intr_pro.read();
      if (nextintr >= 0 && handler_pro[nextintr].fn != NULL)
         (*handler_pro[nextintr].fn)(handler_pro[nextintr].arg);
   }
}

void espintr::maskupdate() {
   if (setunset) mask_pro.write(mask_pro.read() | newmask);
   else mask_pro.write(mask_pro.read() & ~newmask);
   if (setunset) mask_app.write(mask_app.read() | newmask);
   else mask_app.write(mask_app.read() & ~newmask);
}

void espintr::trace(sc_trace_file *tf) {
   sc_trace(tf, raw_app, raw_app.name());
   sc_trace(tf, raw_pro, raw_pro.name());
   sc_trace(tf, mask_app, mask_app.name());
   sc_trace(tf, mask_pro, mask_pro.name());
   sc_trace(tf, intr_app, intr_app.name());
   sc_trace(tf, intr_pro, intr_pro.name());
}
