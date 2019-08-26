/*******************************************************************************
 * ctrlregs.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Defines a number of variables the SystemC model needs to communicate with
 *   the library functions.
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

#ifndef _CTRLREGS_H
#define _CTRLREGS_H

#include "rom/rtc.h"

struct ctrlregs_t {
   RESET_REASON cpu0_reset_reason;
   RESET_REASON cpu1_reset_reason;
   bool return_to_start;
};

extern ctrlregs_t *ctrlregsptr;

#endif
