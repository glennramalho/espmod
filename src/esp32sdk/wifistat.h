/*******************************************************************************
 * wifistat.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a struct that is used by the ESPMOD SystemC model to keep track
 *   of the current connections.
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

#ifndef WIFISTAT_H
#define WIFISTAT_H

#include "tcpip_adapter.h"
#include "WiFiType.h"
#include "esp_wifi_types.h"
#include <IPAddress.h>

struct wifistat_t {
   bool initialized = false;
   bool started = false;
   bool connected = false;
   bool dhcp_c = false;
   bool dhcp_s = false;
   wl_status_t stat;
   wifi_mode_t mode;
   int8_t power;
   uint32_t mask = 0;
   char hostname[TCPIP_HOSTNAME_MAX_SIZE] = "";
   tcpip_adapter_ip_info_t ip_info;
   wifi_ap_record_t ap_info;
   wifi_sta_list_t sta_list;
   wifi_ps_type_t pstype;
   wifi_config_t conf;
   IPAddress current_ip;
};

extern wifistat_t wifistat;

#endif
