/*******************************************************************************
 * esp_attr.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file is a stub to help get the ESP32 firmware to compile. On the
 *   real system it defines several attributes to locate the code in specific
 *   areas of the flash. In the SystemC model these attributes do not apply
 *   as we are not modeling the flash firmware area. The macros then will
 *   expand to nothing.
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
 *    Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

#ifndef __ESP_ATTR_H__
#define __ESP_ATTR_H__

#define ROMFN_ATTR

#define IRAM_ATTR
#define DRAM_ATTR
/* The word alignment we need to keep as most CPUs need to do this. */
#define WORD_ALIGNED_ATTR __attribute__((aligned(4)))
#define DMA_ATTR WORD_ALIGNED_ATTR DRAM_ATTR
#define DRAM_STR(str) (str)
#define RTC_IRAM_ATTR
#define EXT_RAM_ATTR 
#define RTC_DATA_ATTR
#define RTC_RODATA_ATTR
#define RTC_SLOW_ATTR
#define RTC_FAST_ATTR
#define __NOINIT_ATTR
#define RTC_NOINIT_ATTR
#define NOINLINE_ATTR

#define _SECTION_ATTR_IMPL(SECTION, COUNTER)
#define _COUNTER_STRINGIFY(COUNTER) #COUNTER
#define IDF_DEPRECATED(REASON)

#endif /* __ESP_ATTR_H__ */
