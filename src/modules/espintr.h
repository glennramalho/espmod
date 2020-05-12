/*******************************************************************************
 * espintr.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 Interrupt Module
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

#ifndef _ESPINTR_H
#define _ESPINTR_H

#include <systemc.h>
#include "soc/soc.h"

typedef struct {
   void (*fn)(void *);
   void *arg;
} espm_intr_handler_t;

#define XCHAL_NUM_INTERRUPTS 32
#define ESPM_INTR_TABLE ETS_CACHE_IA_INTR_SOURCE+1
SC_MODULE(espintr) {
   public:
   /* Ports */
   sc_in<bool> ledc_intr_i{"ledc_intr_i"};

   sc_signal<uint32_t> raw_app{"raw_app", 0};
   sc_signal<uint32_t> raw_pro{"raw_pro", 0};
   sc_signal<uint32_t> mask_app{"mask_app", 0};
   sc_signal<uint32_t> mask_pro{"mask_pro", 0};
   sc_signal<uint32_t> intr_app{"intr_app", 0};
   sc_signal<uint32_t> intr_pro{"intr_pro", 0};
   sc_signal<uint32_t> edge_interrupt_pro{"edge_interrupt_pro", 0x40400040};
   sc_signal<uint32_t> edge_interrupt_app{"edge_interrupt_app", 0x40400040};

   /* Tasks */
   void catcher();
   void driver_pro();
   void driver_app();
   void maskupdate();

   /* Variables */
   int table_pro[ESPM_INTR_TABLE];
   int table_app[ESPM_INTR_TABLE];
   espm_intr_handler_t handler_pro[XCHAL_NUM_INTERRUPTS];
   espm_intr_handler_t handler_app[XCHAL_NUM_INTERRUPTS];
   sc_event recapture_ev;

   /* Mask update variables */
   private:
   uint32_t newmask;
   bool procpu;
   bool setunset;
   public:
   sc_event maskupdate_ev;

   /* Functions */
   void set_edge(unsigned int edgemask, int cpu) {
      if (cpu == 0)
         edge_interrupt_pro.write(edge_interrupt_pro.read() | edgemask);
      else
         edge_interrupt_app.write(edge_interrupt_app.read() | edgemask);
   }
   void clr_edge(unsigned int edgemask, int cpu) {
      if (cpu == 0)
         edge_interrupt_pro.write(edge_interrupt_pro.read() & ~edgemask);
      else
         edge_interrupt_app.write(edge_interrupt_app.read() & ~edgemask);
   }
   void setintrmask(unsigned int mask, int cpu) {
      newmask = mask;
      setunset = true;
      procpu = (cpu == 0);
      maskupdate_ev.notify();
   }
   void clrintrmask(unsigned int mask, int cpu) {
      newmask = mask;
      setunset = false;
      procpu = (cpu == 0);
      maskupdate_ev.notify();
   }
   unsigned int getintrmask(int cpu) {
      if (cpu == 0) return mask_pro.read();
      else return mask_app.read();
   }
   int alloc(int cpu, int _ets, int _no) {
      if (_ets < 0 || _ets >= ESPM_INTR_TABLE) return -1;
      if (cpu < 0 || cpu > 1) return -1;
      if (cpu == 0) table_pro[_ets] = _no;
      else table_app[_ets] = _no;
      return 0;
   }
   int dealloc(int cpu, int _ets) {
      /* When we get a dealloc request, we set it to -1. We also need to
       * retrigger the capture task.
       */
      if (_ets < 0 || _ets >= ESPM_INTR_TABLE) return -1;
      if (cpu < 0 || cpu > 1) return -1;
      if (cpu == 0) table_pro[_ets] = -1;
      else table_app[_ets] = -1;
      recapture_ev.notify();
      return 0;
   }
   int assignfn(int cpu, int _no, void (*fn)(void *), void *arg) {
      if (_no < 0 || _no > 31) return -1;
      if (cpu < 0 || cpu > 1) return -1;
      if (cpu == 0) {
         handler_pro[_no].fn = fn;
         handler_pro[_no].arg = arg;
      }
      else {
         handler_app[_no].fn = fn;
         handler_app[_no].arg = arg;
      }
      return 0;
   }
   int deassignfn(int cpu, int _no) {
      if (_no < 0 || _no > 31) return -1;
      if (cpu < 0 || cpu > 1) return -1;
      if (cpu == 0) {
         handler_pro[_no].fn = NULL;
         handler_pro[_no].arg = NULL;
      }
      else {
         handler_app[_no].fn = NULL;
         handler_app[_no].arg = NULL;
      }
   }
   bool has_handler(int cpu, int _no) {
      if (_no < 0 || _no > 31) return -1;
      if (cpu < 0 || cpu > 1) return -1;
      if (cpu == 0) return handler_pro[_no].fn != NULL;
      else return handler_app[_no].fn != NULL;
   }
   private:
   void initialize();
   int get_next(uint32_t edgemask, uint32_t now, uint32_t last);
   public:
   
   SC_CTOR(espintr) {
      /* We begin initializing all the allocation table to -1, we use this to
       * indicate unallocated.
       */
      initialize();
      /* Then we can set the interrupts. We watch every signal plus the mask. */
      SC_THREAD(catcher);
      sensitive << ledc_intr_i << mask_app << mask_pro << recapture_ev;

      /* The driver is sensitive only to the interrupts. */
      SC_THREAD(driver_app);
      sensitive << intr_app;
      SC_THREAD(driver_pro);
      sensitive << intr_pro;
      SC_METHOD(maskupdate);
      sensitive << maskupdate_ev;
   }

   void trace(sc_trace_file *tf);
};
extern espintr *espintrptr;

#endif
