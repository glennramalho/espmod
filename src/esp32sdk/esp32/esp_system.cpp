/*******************************************************************************
 * esp_system.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements several generic system functions to get them to work in the
 *   ESPMOD SystemC Model.
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

#include "esp_system.h"
#include "esp_wifi.h"
#include <stdlib.h>
#include <errno.h>
#include "Arduino.h"
#include "esp32/reset_reason.h"
#include "esp32/rom/rtc.h"

uint32_t get_free_heap_size() {
   /* This function makes no sense in the simulation, so we pick something. */
   return 32768;
}

uint32_t esp_random() {
   return rand();
}

esp_err_t esp_read_mac(uint8_t *mac, esp_mac_type_t type) {
   switch(type) {
      case ESP_MAC_WIFI_STA: return esp_wifi_get_mac(WIFI_IF_STA, mac);
      case ESP_MAC_WIFI_SOFTAP: return esp_wifi_get_mac(WIFI_IF_AP, mac);
      case ESP_MAC_BT: return ESP_ERR_INVALID_ARG;
      case ESP_MAC_ETH: return ESP_ERR_INVALID_ARG;
      default: return ESP_ERR_INVALID_ARG;
   }
}
esp_err_t esp_efuse_read_mac(uint8_t *mac) {
   return esp_wifi_get_mac(WIFI_IF_STA, mac);
}

const char *esp_get_idf_version() {
   return "SIMULATEDESP";
}

int esp_gettimeofday(struct timeval *tv, struct timezone *tz) {
   errno = 0;
   if (tv == NULL) {
      errno = EFAULT;
      return -1;
   }
   time_t tm = millis();
   tv->tv_sec = tm / 1000;
   tv->tv_usec = (tm % 1000) * 1000;
   return 0;
}

void esp_restart() {
   software_reset();
}

void system_restart() {
   software_reset();
}
