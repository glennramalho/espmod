/*******************************************************************************
 * tcpip_adapter.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the tcpip_adapter functions for the ESP32 to the ESPMOD
 *   SystemC model. It was based off the functions from Espressif Systems.
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
#include "info.h"
#include <stdint.h>
#include "esp_err.h"
#include "tcpip_adapter.h"
#include "wifistat.h"

/**
 * @brief Start DHCP client
 *
 * @note DHCP Client is only supported for the Wi-Fi station and Ethernet interfaces.
 *
 * @note The default event handlers for the SYSTEM_EVENT_STA_CONNECTED and SYSTEM_EVENT_ETH_CONNECTED events call this function.
 *
 * @param[in]  tcpip_if Interface to start the DHCP client
 *
 * @return
 *         - ESP_OK
 *         - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         - ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED
 *         - ESP_ERR_TCPIP_ADAPTER_DHCPC_START_FAILED
 */
esp_err_t tcpip_adapter_dhcpc_start(tcpip_adapter_if_t tcpip_if) {
   if (tcpip_if != TCPIP_ADAPTER_IF_STA)
      return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (wifistat.dhcp_c) return ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED;
   wifistat.dhcp_c = true;
   return ESP_OK;
}

/**
 * @brief  Stop DHCP client
 *
 * @note DHCP Client is only supported for the Wi-Fi station and Ethernet interfaces.
 *
 * @note Calling tcpip_adapter_stop() or tcpip_adapter_down() will also stop the DHCP Client if it is running.
 *
 * @param[in] tcpip_if Interface to stop the DHCP client
 *
 * @return
 *      - ESP_OK
 *      - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *      - ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED
 *      - ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_dhcpc_stop(tcpip_adapter_if_t tcpip_if) {
   if (tcpip_if != TCPIP_ADAPTER_IF_STA)
      return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (!wifistat.dhcp_c) return ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED;
   wifistat.dhcp_c = false;
   return ESP_OK;
}

/**
 * @brief  Start DHCP server
 *
 * @note   Currently DHCP server is only supported on the Wi-Fi AP interface.
 *
 * @param[in]  tcpip_if Interface to start DHCP server. Must be TCPIP_ADAPTER_IF
_AP.
 *
 * @return
 *         - ESP_OK
 *         - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *         - ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED
 */
esp_err_t tcpip_adapter_dhcps_start(tcpip_adapter_if_t tcpip_if) {
   if (tcpip_if != TCPIP_ADAPTER_IF_AP)
      return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (wifistat.dhcp_s) return ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED;
   wifistat.dhcp_s = true;
   return ESP_OK;
}

/**
 * @brief  Stop DHCP server
 *
 * @note   Currently DHCP server is only supported on the Wi-Fi AP interface.
 *
 * @param[in]  tcpip_if Interface to stop DHCP server. Must be TCPIP_ADAPTER_IF_AP.
 *
 * @return
 *      - ESP_OK
 *      - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *      - ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED
 *      - ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY
 */
esp_err_t tcpip_adapter_dhcps_stop(tcpip_adapter_if_t tcpip_if) {
   if (tcpip_if != TCPIP_ADAPTER_IF_AP)
      return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (!wifistat.dhcp_s) return ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED;
   wifistat.dhcp_s = false;
   return ESP_OK;
}

/**
 * @brief  Get interface's IP address information
 *
 * If the interface is up, IP information is read directly from the TCP/IP stack.
 *
 * If the interface is down, IP information is read from a copy kept in the TCP/IP adapter
 * library itself.
 *
 * @param[in]   tcpip_if Interface to get IP information
 * @param[out]  ip_info If successful, IP information will be returned in this argument.
 *
 * @return
 *         - ESP_OK
 *         - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 */
esp_err_t tcpip_adapter_get_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
   if (tcpip_if != TCPIP_ADAPTER_IF_STA
         && tcpip_if != TCPIP_ADAPTER_IF_AP && wifistat.dhcp_s)
      return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (ip_info == NULL) return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   memcpy((void *)ip_info, (void *)&wifistat.ip_info,
         sizeof(tcpip_adapter_ip_info_t));
   return ESP_OK;
}

/**
 * @brief  Set interface's IP address information
 *
 * This function is mainly used to set a static IP on an interface.
 *
 * If the interface is up, the new IP information is set directly in the TCP/IP stack.
 *
 * The copy of IP information kept in the TCP/IP adapter library is also updated (this
 * copy is returned if the IP is queried while the interface is still down.)
 *
 * @note DHCP client/server must be stopped before setting new IP information.
 *
 * @note Calling this interface for the Wi-Fi STA or Ethernet interfaces may generate a
 * SYSTEM_EVENT_STA_GOT_IP or SYSTEM_EVENT_ETH_GOT_IP event.
 *
 * @param[in] tcpip_if Interface to set IP information
 * @param[in] ip_info IP information to set on the specified interface
 *
 * @return
 *      - ESP_OK
 *      - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS
 *      - ESP_ERR_TCPIP_ADAPTER_DHCP_NOT_STOPPED If DHCP server or client is still running
 */
esp_err_t tcpip_adapter_set_ip_info(tcpip_adapter_if_t tcpip_if, tcpip_adapter_ip_info_t *ip_info) {
   if (tcpip_if == TCPIP_ADAPTER_IF_STA && wifistat.dhcp_c ||
         tcpip_if == TCPIP_ADAPTER_IF_AP && wifistat.dhcp_s)
      return ESP_ERR_TCPIP_ADAPTER_DHCP_NOT_STOPPED;
   if (ip_info == NULL) return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   memcpy((void *)&wifistat.ip_info, (void *)ip_info,
         sizeof(tcpip_adapter_ip_info_t));
   return ESP_OK;
}

/**
 * @brief  Set the hostname of an interface
 *
 * @param[in]   tcpip_if Interface to set the hostname
 * @param[in]   hostname New hostname for the interface. Maximum length 32 bytes
.
 *
 * @return
 *         - ESP_OK - success
 *         - ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY - interface status error
 *         - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS - parameter error
 */
esp_err_t tcpip_adapter_set_hostname(tcpip_adapter_if_t tcpip_if,
      const char *hostname) {
   if (hostname == NULL) return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   strncpy(wifistat.hostname, hostname, TCPIP_HOSTNAME_MAX_SIZE);
   wifistat.hostname[TCPIP_HOSTNAME_MAX_SIZE] = '\0';
   return ESP_OK;
}

/**
 * @brief  Get interface hostname.
 *
 * @param[in]   tcpip_if Interface to get the hostname
 * @param[out]   hostname Returns a pointer to the hostname. May be NULL if no hostname is set. If set non-NULL, pointer remains valid (and string may change if the hostname changes).
 *
 * @return
 *         - ESP_OK - success
 *         - ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY - interface status error
 *         - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS - parameter error
 */
esp_err_t tcpip_adapter_get_hostname(tcpip_adapter_if_t tcpip_if,
      const char **hostname) {
   if (hostname == NULL) return ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS;
   if (wifistat.hostname[0] == '\0') *hostname = NULL;
   else *hostname = wifistat.hostname;
   return ESP_OK;
}

/**
 * @brief  SmartConfig
 **/
/*
esp_err_t esp_smartconfig_start(sc_callback_t scb, int i) {
   PRINTF_ERROR("WIFI", "smart config currently is unsupported.");
   return ESP_FAIL;
}
*/
