/*******************************************************************************
 * esp_wifi.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the esp_wifi functions for the ESP32 to the ESPMOD SystemC
 *   model. It was based off the functions from Espressif Systems.
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

#include <systemc.h>
#include "WiFi.h"
#include "info.h"
#include <IPAddress.h>
#include "esp_system.h"
#include "esp_err.h"
#include "WiFiType.h"
#include "esp_wifi_types.h"
#include "wifistat.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include <string.h>

wifistat_t wifistat;

/**
  * @brief  Init WiFi
  *         Alloc resource for WiFi driver, such as WiFi control structure, RX/TX buffer,
  *         WiFi NVS structure etc, this WiFi also start WiFi task
  *
  * @attention 1. This API must be called before all other WiFi API can be called
  * @attention 2. Always use WIFI_INIT_CONFIG_DEFAULT macro to init the config to default values, this can
  *               guarantee all the fields got correct value when more fields are added into wifi_init_config_t
  *               in future release. If you want to set your owner initial values, overwrite the default values
  *               which are set by WIFI_INIT_CONFIG_DEFAULT, please be notified that the field 'magic' of 
  *               wifi_init_config_t should always be WIFI_INIT_CONFIG_MAGIC!
  *
  * @param  config pointer to WiFi init configuration structure; can point to a temporary variable.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_NO_MEM: out of memory
  *    - others: refer to error code esp_err.h
  */
esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
   PRINTF_INFO("WIFI", "Initializing");
   wifistat.initialized = true;
   wifistat.started = false;
   wifistat.stat = WL_IDLE_STATUS;
   return ESP_OK;
}

/**
  * @brief  Deinit WiFi
  *         Free all resource allocated in esp_wifi_init and stop WiFi task
  *
  * @attention 1. This API should be called if you want to remove WiFi driver from the system
  *
  * @return 
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_deinit(void) {
   SC_REPORT_INFO("WIFI", "De-Init");
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   wifistat.initialized = false;
   wifistat.started = false;
   wifistat.connected = false;
   return ESP_OK;
}

/**
  * @brief     Set the WiFi operating mode
  *
  *            Set the WiFi operating mode as station, soft-AP or station+soft-AP,
  *            The default mode is soft-AP mode.
  *
  * @param     mode  WiFi operating mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_set_mode(wifi_mode_t mode) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   else switch(mode) {
      case WIFI_MODE_STA:
         SC_REPORT_INFO("WIFI", "Setting WiFi to Station Mode");
         wifistat.mode = mode;
         return ESP_OK;
      case WIFI_MODE_NULL:
         SC_REPORT_INFO("WIFI", "Switching WiFi Off");
         wifistat.mode = mode;
         return ESP_OK;
      case WIFI_MODE_AP:
         SC_REPORT_INFO("WIFI", "Setting WiFi to AP Mode");
         wifistat.mode = mode;
         return ESP_OK;
      case WIFI_MODE_APSTA:
         SC_REPORT_INFO("WIFI", "Setting WiFi to AP/STA Mode");
         wifistat.mode = mode;
         return ESP_OK;
      default:
         return ESP_ERR_INVALID_ARG;
   }
}

/**
  * @brief  Get current operating mode of WiFi
  *
  * @param[out]  mode  store current WiFi mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_mode(wifi_mode_t *mode) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   else if (mode == NULL) return ESP_ERR_INVALID_ARG;
   else *mode = wifistat.mode;
   return ESP_OK;
}

/**
  * @brief  Start WiFi according to current configuration
  *         If mode is WIFI_MODE_STA, it create station control block and start station
  *         If mode is WIFI_MODE_AP, it create soft-AP control block and start soft-AP
  *         If mode is WIFI_MODE_APSTA, it create soft-AP and station control block and start soft-AP and station
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_NO_MEM: out of memory
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
  *    - ESP_FAIL: other WiFi internal errors
  */
esp_err_t esp_wifi_start(void) {
   system_event_t ev;

   /* This is actually done by the init before the firmware loads, but we do
    * it here for lack of a better place.
    */
   if (!wifistat.initialized) esp_wifi_init(NULL);

   /* Now we start the interface. */
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   wifistat.started = true;
   wifistat.connected = false;

   if (wifistat.mode == WIFI_MODE_STA || wifistat.mode == WIFI_MODE_APSTA) {
      ev.event_id = SYSTEM_EVENT_STA_START;
      WiFiClass::_eventCallback(NULL, &ev);
   }
   if (wifistat.mode == WIFI_MODE_AP || wifistat.mode == WIFI_MODE_APSTA) {
      ev.event_id = SYSTEM_EVENT_AP_START;
      WiFiClass::_eventCallback(NULL, &ev);
   }

   return ESP_OK;
}

/**
  * @brief  Stop WiFi
  *         If mode is WIFI_MODE_STA, it stop station and free station control block
  *         If mode is WIFI_MODE_AP, it stop soft-AP and free soft-AP control block
  *         If mode is WIFI_MODE_APSTA, it stop station/soft-AP and free station/soft-AP control block
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_stop(void) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   else return ESP_OK;
}

/**
 * @brief  Restore WiFi stack persistent settings to default values
 *
 * This function will reset settings made using the following APIs:
 * - esp_wifi_get_auto_connect,
 * - esp_wifi_set_protocol,
 * - esp_wifi_set_config related
 * - esp_wifi_set_mode
 *
 * @return
 *    - ESP_OK: succeed
 *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
 */
esp_err_t esp_wifi_restore(void) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   /* Set default settings. */
   return ESP_OK;
}

/**
  * @brief     Connect the ESP32 WiFi station to the AP.
  *
  * @attention 1. This API only impact WIFI_MODE_STA or WIFI_MODE_APSTA mode
  * @attention 2. If the ESP32 is connected to an AP, call esp_wifi_disconnect to disconnect.
  * @attention 3. The scanning triggered by esp_wifi_start_scan() will not be effective until connection between ESP32 and the AP is established.
  *               If ESP32 is scanning and connecting at the same time, ESP32 will abort scanning and return a warning message and error
  *               number ESP_ERR_WIFI_STATE.
  *               If you want to do reconnection after ESP32 received disconnect event, remember to add the maximum retry time, otherwise the called	
  *               scan will not work. This is especially true when the AP doesn't exist, and you still try reconnection after ESP32 received disconnect
  *               event with the reason code WIFI_REASON_NO_AP_FOUND.
  *   
  * @return 
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, station or soft-AP control block wrong
  *    - ESP_ERR_WIFI_SSID: SSID of AP which station connects is invalid
  */
esp_err_t esp_wifi_connect(void) {
   system_event_t ev;

   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (!wifistat.started) return ESP_ERR_WIFI_NOT_STARTED;
   if (wifistat.conf.sta.ssid[0] == '\0'
         || strlen((const char *)wifistat.conf.sta.ssid)>31)
      return ESP_ERR_WIFI_SSID;
   if (strchr((const char *)wifistat.conf.sta.ssid, ' ') != NULL)
      PRINTF_WARN("WIFI",
         "Found a space in the SSID, what is legal but often not supported.");

   /* If we are running a DHCP, we need to set the IP addresses. We then
    * pick something as we do not actually have a server.
    */
   if (wifistat.dhcp_c) {
      wifistat.ip_info.ip.addr = IPAddress(192, 76, 0, 100);
      wifistat.ip_info.gw.addr = IPAddress(192, 76, 0, 1);
      wifistat.ip_info.netmask.addr = IPAddress(255, 255, 255, 0);
   }

   /* We call the callback */
   ev.event_id = SYSTEM_EVENT_STA_GOT_IP;
   ev.event_info.got_ip.ip_info.ip = wifistat.ip_info.ip;
   ev.event_info.got_ip.ip_info.gw = wifistat.ip_info.gw;
   ev.event_info.got_ip.ip_info.netmask = wifistat.ip_info.netmask;
   /* If the IP changed we note tag it. */
   if (wifistat.current_ip != wifistat.ip_info.ip.addr) 
      ev.event_info.got_ip.ip_changed = true;
   else ev.event_info.got_ip.ip_changed = false;
   WiFiClass::_eventCallback(NULL, &ev);

   /* And we do the connection. */
   wifistat.connected = true;
   wifistat.current_ip = wifistat.ip_info.ip.addr;

   return ESP_OK;
}

/**
  * @brief     Disconnect the ESP32 WiFi station from the AP.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi was not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_FAIL: other WiFi internal errors
  */
esp_err_t esp_wifi_disconnect(void) {
   system_event_t ev;

   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (!wifistat.started) return ESP_ERR_WIFI_NOT_STARTED;

   /* We call the callback */
   ev.event_id = SYSTEM_EVENT_STA_DISCONNECTED;
   /* For now it is the only one */
   ev.event_info.disconnected.reason = WIFI_REASON_UNSPECIFIED;

   WiFiClass::_eventCallback(NULL, &ev);

   /* And we do the connection. */
   wifistat.connected = false;

   return ESP_OK;
}

/**
  * @brief     Currently this API is just an stub API
  *

  * @return
  *    - ESP_OK: succeed
  *    - others: fail
  */
esp_err_t esp_wifi_clear_fast_connect(void);

/**
  * @brief     deauthenticate all stations or associated id equals to aid
  *
  * @param     aid  when aid is 0, deauthenticate all stations, otherwise deauthenticate station whose associated id is aid
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  */
esp_err_t esp_wifi_deauth_sta(uint16_t aid);

/**
  * @brief     Scan all available APs.
  *
  * @attention If this API is called, the found APs are stored in WiFi driver dynamic allocated memory and the
  *            will be freed in esp_wifi_scan_get_ap_records, so generally, call esp_wifi_scan_get_ap_records to cause
  *            the memory to be freed once the scan is done
  * @attention The values of maximum active scan time and passive scan time per channel are limited to 1500 milliseconds.
  *            Values above 1500ms may cause station to disconnect from AP and are not recommended.
  *
  * @param     config  configuration of scanning
  * @param     block if block is true, this API will block the caller until the scan is done, otherwise
  *                         it will return immediately
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  *    - ESP_ERR_WIFI_TIMEOUT: blocking scan is timeout
  *    - ESP_ERR_WIFI_STATE: wifi still connecting when invoke esp_wifi_scan_start
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *config, bool block);

/**
  * @brief     Stop the scan in process
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  */
esp_err_t esp_wifi_scan_stop(void);

/**
  * @brief     Get number of APs found in last scan
  *
  * @param[out] number  store number of APIs found in last scan
  *
  * @attention This API can only be called when the scan is completed, otherwise it may get wrong value.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_scan_get_ap_num(uint16_t *number);

/**
  * @brief     Get AP list found in last scan
  *
  * @param[inout]  number As input param, it stores max AP number ap_records can hold. 
  *                As output param, it receives the actual AP number this API returns.
  * @param         ap_records  wifi_ap_record_t array to hold the found APs
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_NO_MEM: out of memory
  */
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *number, wifi_ap_record_t *ap_records);


/**
  * @brief     Get information of AP which the ESP32 station is associated with
  *
  * @param     ap_info  the wifi_ap_record_t to hold AP information
  *            sta can get the connected ap's phy mode info through the struct member
  *            phy_11b，phy_11g，phy_11n，phy_lr in the wifi_ap_record_t struct.
  *            For example, phy_11b = 1 imply that ap support 802.11b mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_CONN: The station interface don't initialized
  *    - ESP_ERR_WIFI_NOT_CONNECT: The station is in disconnect status 
  */
esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap_info) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_CONN;
   if (!wifistat.started) return ESP_ERR_WIFI_CONN;
   if (!wifistat.connected) return ESP_ERR_WIFI_NOT_CONNECT;
   if (ap_info == NULL) return ESP_ERR_INVALID_ARG; /* NOT IN THE LIST */
   memcpy((void *)ap_info, (void *)&wifistat.ap_info, sizeof(wifi_ap_record_t));
   return ESP_OK;
}

/**
  * @brief     Set current WiFi power save type
  *
  * @attention Default power save type is WIFI_PS_MIN_MODEM.
  *
  * @param     type  power save type
  *
  * @return    ESP_OK: succeed
  */
esp_err_t esp_wifi_set_ps(wifi_ps_type_t type) {
   wifistat.pstype = type;
   return ESP_OK;
}

/**
  * @brief     Get current WiFi power save type
  *
  * @attention Default power save type is WIFI_PS_MIN_MODEM.
  *
  * @param[out]  type: store current power save type
  *
  * @return    ESP_OK: succeed
  */
esp_err_t esp_wifi_get_ps(wifi_ps_type_t *type) {
   if (type == NULL) return ESP_ERR_INVALID_ARG; /* NOT IN THE LIST */
   *type = wifistat.pstype;
   return ESP_OK;
}

/**
  * @brief     Set protocol type of specified interface
  *            The default protocol is (WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)
  *
  * @attention Currently we only support 802.11b or 802.11bg or 802.11bgn mode
  *
  * @param     ifx  interfaces
  * @param     protocol_bitmap  WiFi protocol bitmap
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_protocol(wifi_interface_t ifx, uint8_t protocol_bitmap);

/**
  * @brief     Get the current protocol bitmap of the specified interface
  *
  * @param     ifx  interface
  * @param[out] protocol_bitmap  store current WiFi protocol bitmap of interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_get_protocol(wifi_interface_t ifx, uint8_t *protocol_bitmap);

/**
  * @brief     Set the bandwidth of ESP32 specified interface
  *
  * @attention 1. API return false if try to configure an interface that is not enabled
  * @attention 2. WIFI_BW_HT40 is supported only when the interface support 11N
  *
  * @param     ifx  interface to be configured
  * @param     bw  bandwidth
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t bw);

/**
  * @brief     Get the bandwidth of ESP32 specified interface
  *
  * @attention 1. API return false if try to get a interface that is not enable
  *
  * @param     ifx interface to be configured
  * @param[out] bw  store bandwidth of interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t *bw);

/**
  * @brief     Set primary/secondary channel of ESP32
  *
  * @attention 1. This is a special API for sniffer
  * @attention 2. This API should be called after esp_wifi_start() or esp_wifi_set_promiscuous()
  *
  * @param     primary  for HT20, primary is the channel number, for HT40, primary is the primary channel
  * @param     second   for HT20, second is ignored, for HT40, second is the second channel
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (!wifistat.started) return ESP_ERR_WIFI_CONN;
   if (primary > 13) return ESP_ERR_INVALID_ARG;
   wifistat.conf.sta.channel = primary;
   return ESP_OK;
}

/**
  * @brief     Get the primary/secondary channel of ESP32
  *
  * @attention 1. API return false if try to get a interface that is not enable
  *
  * @param     primary   store current primary channel
  * @param[out]  second  store current second channel
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_channel(uint8_t *primary, wifi_second_chan_t *second) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   else if (primary == NULL || second == NULL) return ESP_ERR_INVALID_ARG;
   *primary = wifistat.conf.sta.channel;
   *second = WIFI_SECOND_CHAN_NONE;
   return ESP_OK;
}

/**
  * @brief     configure country info
  *
  * @attention 1. The default country is {.cc="CN", .schan=1, .nchan=13, policy=WIFI_COUNTRY_POLICY_AUTO}
  * @attention 2. When the country policy is WIFI_COUNTRY_POLICY_AUTO, the country info of the AP to which
  *               the station is connected is used. E.g. if the configured country info is {.cc="USA", .schan=1, .nchan=11}
  *               and the country info of the AP to which the station is connected is {.cc="JP", .schan=1, .nchan=14}
  *               then the country info that will be used is {.cc="JP", .schan=1, .nchan=14}. If the station disconnected
  *               from the AP the country info is set back back to the country info of the station automatically,
  *               {.cc="USA", .schan=1, .nchan=11} in the example.
  * @attention 3. When the country policy is WIFI_COUNTRY_POLICY_MANUAL, always use the configured country info.
  * @attention 4. When the country info is changed because of configuration or because the station connects to a different
  *               external AP, the country IE in probe response/beacon of the soft-AP is changed also.
  * @attention 5. The country configuration is not stored into flash
  * @attention 6. This API doesn't validate the per-country rules, it's up to the user to fill in all fields according to 
  *               local regulations.
  *
  * @param     country   the configured country info
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_country(const wifi_country_t *country);

/**
  * @brief     get the current country info
  *
  * @param     country  country info
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_country(wifi_country_t *country);


/**
  * @brief     Set MAC address of the ESP32 WiFi station or the soft-AP interface.
  *
  * @attention 1. This API can only be called when the interface is disabled
  * @attention 2. ESP32 soft-AP and station have different MAC addresses, do not set them to be the same.
  * @attention 3. The bit 0 of the first byte of ESP32 MAC address can not be 1. For example, the MAC address
  *      can set to be "1a:XX:XX:XX:XX:XX", but can not be "15:XX:XX:XX:XX:XX".
  *
  * @param     ifx  interface
  * @param     mac  the MAC address
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_WIFI_MAC: invalid mac address
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  *    - others: refer to error codes in esp_err.h
  */
esp_err_t esp_wifi_set_mac(wifi_interface_t ifx, const uint8_t mac[6]) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (ifx != WIFI_IF_STA && ifx != WIFI_IF_AP)
      return ESP_ERR_INVALID_ARG;
   PRINTF_WARN("WIFI", "esp_wifi_set_mac currently is unsupported.");
   return ESP_OK;
}

/**
  * @brief     Get mac of specified interface
  *
  * @param      ifx  interface
  * @param[out] mac  store mac of the interface ifx
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  */
esp_err_t esp_wifi_get_mac(wifi_interface_t ifx, uint8_t mac[6]) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (ifx != WIFI_IF_STA && ifx != WIFI_IF_AP)
      return ESP_ERR_INVALID_ARG;

   switch(ifx) {
      case WIFI_IF_STA:
         mac[0] = 0x55;
         mac[1] = 0x37;
         mac[2] = 0xce;
         mac[3] = 0x99;
         mac[4] = 0x75;
         mac[5] = 0x1E;
         return ESP_OK;
      case WIFI_IF_AP:
         mac[0] = 0x55;
         mac[1] = 0x37;
         mac[2] = 0xce;
         mac[3] = 0x99;
         mac[4] = 0x75;
         mac[5] = 0x39;
         return ESP_OK;
      default: return ESP_ERR_INVALID_ARG;
   }
}

/**
  * @brief The RX callback function in the promiscuous mode. 
  *        Each time a packet is received, the callback function will be called.
  *
  * @param buf  Data received. Type of data in buffer (wifi_promiscuous_pkt_t or wifi_pkt_rx_ctrl_t) indicated by 'type' parameter.
  * @param type  promiscuous packet type.
  *
  */
typedef void (* wifi_promiscuous_cb_t)(void *buf, wifi_promiscuous_pkt_type_t type);

/**
  * @brief Register the RX callback function in the promiscuous mode.
  *
  * Each time a packet is received, the registered callback function will be called.
  *
  * @param cb  callback
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);

/**
  * @brief     Enable the promiscuous mode.
  *
  * @param     en  false - disable, true - enable
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous(bool en);

/**
  * @brief     Get the promiscuous mode.
  *
  * @param[out] en  store the current status of promiscuous mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous(bool *en);

/**
  * @brief Enable the promiscuous mode packet type filter.
  *
  * @note The default filter is to filter all packets except WIFI_PKT_MISC
  *
  * @param filter the packet type filtered in promiscuous mode.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_filter(const wifi_promiscuous_filter_t *filter);

/**
  * @brief     Get the promiscuous filter.
  *
  * @param[out] filter  store the current status of promiscuous filter
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous_filter(wifi_promiscuous_filter_t *filter);

/**
  * @brief Enable subtype filter of the control packet in promiscuous mode.
  *
  * @note The default filter is to filter none control packet.
  *
  * @param filter the subtype of the control packet filtered in promiscuous mode.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_promiscuous_ctrl_filter(const wifi_promiscuous_filter_t *filter);

/**
  * @brief     Get the subtype filter of the control packet in promiscuous mode.
  *
  * @param[out] filter  store the current status of subtype filter of the control packet in promiscuous mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_promiscuous_ctrl_filter(wifi_promiscuous_filter_t *filter);

/**
  * @brief     Set the configuration of the ESP32 STA or AP
  *
  * @attention 1. This API can be called only when specified interface is enabled, otherwise, API fail
  * @attention 2. For station configuration, bssid_set needs to be 0; and it needs to be 1 only when users need to check the MAC address of the AP.
  * @attention 3. ESP32 is limited to only one channel, so when in the soft-AP+station mode, the soft-AP will adjust its channel automatically to be the same as
  *               the channel of the ESP32 station.
  *
  * @param     interface  interface
  * @param     conf  station or soft-AP configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  *    - ESP_ERR_WIFI_MODE: invalid mode
  *    - ESP_ERR_WIFI_PASSWORD: invalid password
  *    - ESP_ERR_WIFI_NVS: WiFi internal NVS error
  *    - others: refer to the erro code in esp_err.h
  */
esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (conf == NULL) return ESP_ERR_INVALID_ARG;

   if (interface == WIFI_IF_STA) {
      if (strlen((const char *)(conf->sta.password)) > 64) {
         return ESP_ERR_WIFI_PASSWORD;
      }
   }
   else if (interface == WIFI_IF_AP) {
      if (strlen((char *)conf->ap.password) > 0
            && strlen((char *)conf->ap.password) < 8
            || strlen((char *)conf->ap.password) > 64)
         return ESP_ERR_WIFI_PASSWORD;
      /* We have no DHCP server, so for lack of a better place, we also set the
       * IP here.
       */
      if (!wifistat.dhcp_s) {
         wifistat.ip_info.ip.addr = IPAddress(192, 76, 0, 1);
         wifistat.ip_info.gw.addr = IPAddress(192, 76, 0, 1);
         wifistat.ip_info.netmask.addr = IPAddress(255, 255, 255, 0);
      }
   }
   else return ESP_ERR_INVALID_ARG;

   memcpy((void *)&wifistat.conf, (void *)conf, sizeof(wifi_config_t));

   return ESP_OK;
}

/**
  * @brief     Get configuration of specified interface
  *
  * @param     interface  interface
  * @param[out]  conf  station or soft-AP configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_IF: invalid interface
  */
esp_err_t esp_wifi_get_config(wifi_interface_t interface, wifi_config_t *conf) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (conf == NULL) return ESP_ERR_INVALID_ARG;
   if (interface != WIFI_IF_STA && interface != WIFI_IF_AP)
      return ESP_ERR_INVALID_ARG;

   if (interface == WIFI_IF_STA) {
      if (wifistat.mode != WIFI_MODE_STA && wifistat.mode != WIFI_MODE_APSTA) {
         return ESP_ERR_WIFI_IF;
      }
   }
   else if (interface == WIFI_IF_AP) {
      if (wifistat.mode != WIFI_MODE_AP && wifistat.mode != WIFI_MODE_APSTA) {
         return ESP_ERR_WIFI_IF;
      }
   }
   else return ESP_ERR_INVALID_ARG;

   memcpy((void *)conf, (void *)&wifistat.conf, sizeof(wifi_config_t));
   return ESP_OK;
}

/**
  * @brief     Get STAs associated with soft-AP
  *
  * @attention SSC only API
  *
  * @param[out] sta  station list
  *             ap can get the connected sta's phy mode info through the struct member
  *             phy_11b，phy_11g，phy_11n，phy_lr in the wifi_sta_info_t struct.
  *             For example, phy_11b = 1 imply that sta support 802.11b mode
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  *    - ESP_ERR_WIFI_MODE: WiFi mode is wrong
  *    - ESP_ERR_WIFI_CONN: WiFi internal error, the station/soft-AP control block is invalid
  */
esp_err_t esp_wifi_ap_get_sta_list(wifi_sta_list_t *sta) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (sta == NULL) return ESP_ERR_INVALID_ARG;
   if (wifistat.mode == WIFI_MODE_STA) return ESP_ERR_WIFI_MODE;
   memcpy((void *)sta, (void *)&wifistat.sta_list, sizeof(wifi_sta_list_t));
   return ESP_OK;
}


/**
  * @brief     Set the WiFi API configuration storage type
  *
  * @attention 1. The default value is WIFI_STORAGE_FLASH
  *
  * @param     storage : storage type
  *
  * @return
  *   - ESP_OK: succeed
  *   - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *   - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_storage(wifi_storage_t storage);

/**
  * @brief     Set auto connect
  *            The default value is true
  *
  * @param     en : true - enable auto connect / false - disable auto connect
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_MODE: WiFi internal error, the station/soft-AP control block is invalid
  *    - others: refer to error code in esp_err.h
  */
esp_err_t esp_wifi_set_auto_connect(bool en) {
   return ESP_OK;
}

/**
  * @brief     Get the auto connect flag
  *
  * @param[out] en  store current auto connect configuration
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_auto_connect(bool *en) {
   return ESP_OK;
}

/**
  * @brief     Set 802.11 Vendor-Specific Information Element
  *
  * @param     enable If true, specified IE is enabled. If false, specified IE is removed.
  * @param     type Information Element type. Determines the frame type to associate with the IE.
  * @param     idx  Index to set or clear. Each IE type can be associated with up to two elements (indices 0 & 1).
  * @param     vnd_ie Pointer to vendor specific element data. First 6 bytes should be a header with fields matching vendor_ie_data_t.
  *            If enable is false, this argument is ignored and can be NULL. Data does not need to remain valid after the function returns.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init()
  *    - ESP_ERR_INVALID_ARG: Invalid argument, including if first byte of vnd_ie is not WIFI_VENDOR_IE_ELEMENT_ID (0xDD)
  *      or second byte is an invalid length.
  *    - ESP_ERR_NO_MEM: Out of memory
  */
esp_err_t esp_wifi_set_vendor_ie(bool enable, wifi_vendor_ie_type_t type, wifi_vendor_ie_id_t idx, const void *vnd_ie);

/**
  * @brief     Function signature for received Vendor-Specific Information Element callback.
  * @param     ctx Context argument, as passed to esp_wifi_set_vendor_ie_cb() when registering callback.
  * @param     type Information element type, based on frame type received.
  * @param     sa Source 802.11 address.
  * @param     vnd_ie Pointer to the vendor specific element data received.
  * @param     rssi Received signal strength indication.
  */
typedef void (*esp_vendor_ie_cb_t) (void *ctx, wifi_vendor_ie_type_t type, const uint8_t sa[6], const vendor_ie_data_t *vnd_ie, int rssi);

/**
  * @brief     Register Vendor-Specific Information Element monitoring callback.
  *
  * @param     cb   Callback function
  * @param     ctx  Context argument, passed to callback function.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_vendor_ie_cb(esp_vendor_ie_cb_t cb, void *ctx);

/**
  * @brief     Set maximum WiFi transmiting power
  *
  * @attention WiFi transmiting power is divided to six levels in phy init data.
  *            Level0 represents highest transmiting power and level5 represents lowest
  *            transmiting power. Packets of different rates are transmitted in
  *            different powers according to the configuration in phy init data.
  *            This API only sets maximum WiFi transmiting power. If this API is called,
  *            the transmiting power of every packet will be less than or equal to the
  *            value set by this API. If this API is not called, the value of maximum
  *            transmitting power set in phy_init_data.bin or menuconfig (depend on
  *            whether to use phy init data in partition or not) will be used. Default
  *            value is level0. Values passed in power are mapped to transmit power
  *            levels as follows:
  *            - [78, 127]: level0
  *            - [76, 77]: level1
  *            - [74, 75]: level2
  *            - [68, 73]: level3
  *            - [60, 67]: level4
  *            - [52, 59]: level5
  *            - [44, 51]: level5 - 2dBm
  *            - [34, 43]: level5 - 4.5dBm
  *            - [28, 33]: level5 - 6dBm
  *            - [20, 27]: level5 - 8dBm
  *            - [8, 19]: level5 - 11dBm
  *            - [-128, 7]: level5 - 14dBm
  *
  * @param     power  Maximum WiFi transmiting power.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  */
esp_err_t esp_wifi_set_max_tx_power(int8_t power) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (!wifistat.started) return ESP_ERR_WIFI_NOT_STARTED;
   wifistat.power = power;
   return ESP_OK;
}

/**
  * @brief     Get maximum WiFi transmiting power
  *
  * @attention This API gets maximum WiFi transmiting power. Values got
  *            from power are mapped to transmit power levels as follows:
  *            - 78: 19.5dBm
  *            - 76: 19dBm
  *            - 74: 18.5dBm
  *            - 68: 17dBm
  *            - 60: 15dBm
  *            - 52: 13dBm
  *            - 44: 11dBm
  *            - 34: 8.5dBm
  *            - 28: 7dBm
  *            - 20: 5dBm
  *            - 8:  2dBm
  *            - -4: -1dBm
  *
  * @param     power  Maximum WiFi transmiting power.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_max_tx_power(int8_t *power) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (!wifistat.started) return ESP_ERR_WIFI_NOT_STARTED;
   if (power == NULL) return ESP_ERR_INVALID_ARG;
   *power = wifistat.power;
   return ESP_OK;
}

/**
  * @brief     Set mask to enable or disable some WiFi events
  *
  * @attention 1. Mask can be created by logical OR of various WIFI_EVENT_MASK_ constants.
  *               Events which have corresponding bit set in the mask will not be delivered to the system event handler.
  * @attention 2. Default WiFi event mask is WIFI_EVENT_MASK_AP_PROBEREQRECVED.
  * @attention 3. There may be lots of stations sending probe request data around.
  *               Don't unmask this event unless you need to receive probe request data.
  *
  * @param     mask  WiFi event mask.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */
esp_err_t esp_wifi_set_event_mask(uint32_t mask) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   wifistat.mask = mask;
   return ESP_OK;
}

/**
  * @brief     Get mask of WiFi events
  *
  * @param     mask  WiFi event mask.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_get_event_mask(uint32_t *mask) {
   if (!wifistat.initialized) return ESP_ERR_WIFI_NOT_INIT;
   if (mask == NULL) return ESP_ERR_INVALID_ARG;
   *mask = wifistat.mask;
   return ESP_OK;
}

/**
  * @brief     Send raw ieee80211 data
  *
  * @attention Currently only support for sending beacon/probe request/probe response/action and non-QoS
  *            data frame
  * 
  * @param     ifx interface if the Wi-Fi mode is Station, the ifx should be WIFI_IF_STA. If the Wi-Fi
  *            mode is SoftAP, the ifx should be WIFI_IF_AP. If the Wi-Fi mode is Station+SoftAP, the 
  *            ifx should be WIFI_IF_STA or WIFI_IF_AP. If the ifx is wrong, the API returns ESP_ERR_WIFI_IF.
  * @param     buffer raw ieee80211 buffer
  * @param     len the length of raw buffer, the len must be <= 1500 Bytes and >= 24 Bytes
  * @param     en_sys_seq indicate whether use the internal sequence number. If en_sys_seq is false, the 
  *            sequence in raw buffer is unchanged, otherwise it will be overwritten by WiFi driver with 
  *            the system sequence number.
  *            Generally, if esp_wifi_80211_tx is called before the Wi-Fi connection has been set up, both
  *            en_sys_seq==true and en_sys_seq==false are fine. However, if the API is called after the Wi-Fi
  *            connection has been set up, en_sys_seq must be true, otherwise ESP_ERR_INVALID_ARG is returned.
  *
  * @return
  *    - ESP_OK: success
  *    - ESP_ERR_WIFI_IF: Invalid interface
  *    - ESP_ERR_INVALID_ARG: Invalid parameter
  *    - ESP_ERR_WIFI_NO_MEM: out of memory
  */

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

/**
  * @brief The RX callback function of Channel State Information(CSI)  data. 
  *
  *        Each time a CSI data is received, the callback function will be called.
  *
  * @param ctx context argument, passed to esp_wifi_set_csi_rx_cb() when registering callback function. 
  * @param data CSI data received. The memory that it points to will be deallocated after callback function returns. 
  *
  */
typedef void (* wifi_csi_cb_t)(void *ctx, wifi_csi_info_t *data);


/**
  * @brief Register the RX callback function of CSI data.
  *
  *        Each time a CSI data is received, the callback function will be called.
  *
  * @param cb  callback
  * @param ctx context argument, passed to callback function
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  */

esp_err_t esp_wifi_set_csi_rx_cb(wifi_csi_cb_t cb, void *ctx);

/**
  * @brief Set CSI data configuration
  *
  * @param config configuration
  * 
  * return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start or promiscuous mode is not enabled
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_csi_config(const wifi_csi_config_t *config);

/**
  * @brief Enable or disable CSI
  *
  * @param en true - enable, false - disable
  *
  * return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_WIFI_NOT_START: WiFi is not started by esp_wifi_start or promiscuous mode is not enabled
  *    - ESP_ERR_INVALID_ARG: invalid argument
  */
esp_err_t esp_wifi_set_csi(bool en);

/**
  * @brief     Set antenna GPIO configuration
  *
  * @param     config  Antenna GPIO configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: Invalid argument, e.g. parameter is NULL, invalid GPIO number etc
  */
esp_err_t esp_wifi_set_ant_gpio(const wifi_ant_gpio_config_t *config);

/**
  * @brief     Get current antenna GPIO configuration
  *
  * @param     config  Antenna GPIO configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument, e.g. parameter is NULL
  */
esp_err_t esp_wifi_get_ant_gpio(wifi_ant_gpio_config_t *config);


/**
  * @brief     Set antenna configuration
  *
  * @param     config  Antenna configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: Invalid argument, e.g. parameter is NULL, invalid antenna mode or invalid GPIO number
  */
esp_err_t esp_wifi_set_ant(const wifi_ant_config_t *config);

/**
  * @brief     Get current antenna configuration
  *
  * @param     config  Antenna configuration.
  *
  * @return
  *    - ESP_OK: succeed
  *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
  *    - ESP_ERR_INVALID_ARG: invalid argument, e.g. parameter is NULL
  */
esp_err_t esp_wifi_get_ant(wifi_ant_config_t *config);
