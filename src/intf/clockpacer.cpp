/*******************************************************************************
 * clkpacer.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "clockpacer.h"

clockpacer_t clockpacer;

void clockpacer_t::wait_next_cpu_clk() {
   long int nanoseconds;
   nanoseconds = (long int)floor(sc_time_stamp().to_seconds() * 1e9);
   wait(cpu_period - sc_time(nanoseconds, SC_NS));
}

void clockpacer_t::wait_next_apb_clk() {
   long int nanoseconds;
   nanoseconds = (long int)floor(sc_time_stamp().to_seconds() * 1e9);
   wait(apb_period - sc_time(nanoseconds, SC_NS));
}

void clockpacer_t::wait_next_ref_clk() {
   long int nanoseconds;
   nanoseconds = (long int)floor(sc_time_stamp().to_seconds() * 1e9);
   wait(ref_period - sc_time(nanoseconds, SC_NS));
}

void clockpacer_t::wait_next_rtc8m_clk() {
   long int nanoseconds;
   nanoseconds = (long int)floor(sc_time_stamp().to_seconds() * 1e9);
   wait(rtc8m_period - sc_time(nanoseconds, SC_NS));
}
