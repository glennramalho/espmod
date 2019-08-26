/*******************************************************************************
 * romrtc.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file replaces the esp_private/system_internal.h to initialize the
 *   ESP32 SystemC model reset interface.
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

#ifndef _RESET_REASON_H
#define _RESET_REASON_H

#ifdef __cplusplus
extern "C" {
#endif

void esp_reset_reason_set_hint(esp_reset_reason_t hint);
void esp_reset_reason_clear_hint();
esp_reset_reason_t esp_reset_reason_get_hint();
void esp_reset_reason_init(void);

#ifdef __cplusplus
}
#endif

#endif
