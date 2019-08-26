/*******************************************************************************
 * romrtc.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file replaces the functions in in the esp32/rom/rtc.h file for
 *   equivalents that work with the ESP32 SystemC model.
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
 *
 * This file was based off the work covered by the license below:
 *    Copyright 2018 Espressif Systems (Shanghai) PTE LTD
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <systemc.h>
#include "info.h"
#include "rom/rtc.h"
#include "ctrlregs.h"

RESET_REASON rtc_get_reset_reason(int cpu_no) {
   switch(cpu_no) {
      case 0: return ctrlregsptr->cpu0_reset_reason;
      case 1: return ctrlregsptr->cpu1_reset_reason;
      default:
         PRINTF_WARN("RESETREASON",
            "Attempted to get reset reason of non--existant cpu %d", cpu_no);
         return NO_MEAN;
   }
}

WAKEUP_REASON rtc_get_wakeup_cause(void) {
   /* Not yet supported by the model. */
   return NO_SLEEP;
}

uint32_t calc_rtc_memory_crc(uint32_t start_addr, uint32_t crc_len) {
   /* Currently there is no RTC memory */
   return 0;
}

void set_rtc_memory_crc() {
   PRINTF_WARN("RTC", "Currently there is no RTC memory in the model");
}

void software_reset() {
   ctrlregsptr->cpu0_reset_reason = SW_RESET;
   ctrlregsptr->cpu1_reset_reason = SW_RESET;
   ctrlregsptr->return_to_start = true;
}

void software_reset_cpu(int cpu_no) {
   PRINTF_WARN("RTC", "software_reset_cpu() is not recommended and in the model it does the same as software_reset()");
   if (cpu_no == 0) ctrlregsptr->cpu0_reset_reason = SW_RESET;
   else if (cpu_no == 1) ctrlregsptr->cpu1_reset_reason = SW_RESET;
   else {
      PRINTF_WARN("RESETREASON",
         "Attempted to apply software reset on non--existant cpu %d", cpu_no);
      return;
   }

   ctrlregsptr->return_to_start = true;
}
