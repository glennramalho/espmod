/*******************************************************************************
 * clkpace.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements functions to step time. No clock was implemented to avoid the
 *   delay. Instead use these variables and functions for the same purpose.
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
 */

#ifndef _CLOCKPACER_H
#define _CLOCKPACER_H

#include <systemc.h>

class clockpacer_t {
   private:
   sc_time cpu_period;
   sc_time apb_period;
   sc_time ref_period;
   sc_time rtc8m_period;

   public:
   clockpacer_t(): cpu_period(3, SC_NS), apb_period(12.5, SC_NS),
      ref_period(1, SC_US), rtc8m_period(125, SC_NS) {};
   void wait_next_cpu_clk();
   void wait_next_apb_clk();
   void wait_next_ref_clk();
   void wait_next_rtc8m_clk();
   sc_time get_cpu_period() { return cpu_period; }
   sc_time get_apb_period() { return apb_period; }
   sc_time get_ref_period() { return ref_period; }
   sc_time get_rtc8m_period() { return rtc8m_period; }
   void set_cpu_period(sc_time &_n) { cpu_period = _n; }
   void set_apb_period(sc_time &_n) { apb_period = _n; }
   void set_ref_period(sc_time &_n) { ref_period = _n; }
   void set_rtc8m_period(sc_time &_n) { rtc8m_period = _n; }
};

extern clockpacer_t clockpacer;

#endif
