// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Modified 29 July 2019 by Glenn Ramalho


#include <stdlib.h>
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "simnetdb.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "platform.h"
#include "esp_transport_tcp.h"
#include "esp_transport_utils.h"

static const char *TAG = "TRANS_TCP";

typedef struct {
    int sock;
} esp_transport_tcp_t;

static int resolve_dns(const char *host, struct sockaddr_in *ip) {
   int a, b, c, d;

   /* Currently this is not supported, so for now it better be a IP coded as
    * string or a random value is returned.
    */
   if (host == NULL || ip == NULL) return ESP_FAIL;
   if (4 != sscanf(host, "%d.%d.%d.%d", &a, &b, &c, &d)) {
      ip->sin_addr.s_addr = a;
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (b << 8);
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (c << 16);
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (d << 24);
   }
   else {
      ip->sin_addr.s_addr = 127;
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (210 << 8);
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (0 << 16);
      ip->sin_addr.s_addr = ip->sin_addr.s_addr | (55 << 24);
   }
   ip->sin_family = AF_INET;
   return ESP_OK;
}

static int tcp_connect(esp_transport_handle_t t, const char *host, int port, int timeout_ms)
{
   struct sockaddr_in remote_ip;
   struct timeval tv;
   esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);

   bzero(&remote_ip, sizeof(struct sockaddr_in));

   //if stream_host is not ip address, resolve it AF_INET,servername,&serveraddr.sin_addr
   if (inet_pton(AF_INET, host, &remote_ip.sin_addr) != 1) {
      if (resolve_dns(host, &remote_ip) < 0) {
         return -1;
      }
   }

   tcp->sock = espm_socket(PF_INET, SOCK_STREAM, 0);

   if (tcp->sock < 0) {
      ESP_LOGE(TAG, "Error create socket");
      return -1;
   }

   remote_ip.sin_family = AF_INET;
   remote_ip.sin_port = htons(port);

   esp_transport_utils_ms_to_timeval(timeout_ms, &tv);

   espm_setsockopt(tcp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

   ESP_LOGD(TAG, "[sock=%d],connecting to server IP:%08x,Port:%d...",
      tcp->sock, remote_ip.sin_addr.s_addr, port);
   if (espm_connect(tcp->sock, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr)) != 0) {
        espm_close(tcp->sock);
        tcp->sock = -1;
        return -1;
    }
    return tcp->sock;
}

static int tcp_write(esp_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    int poll;
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    if ((poll = esp_transport_poll_write(t, timeout_ms)) <= 0) {
        return poll;
    }
    return espm_write(tcp->sock, buffer, len);
}

static int tcp_read(esp_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    int poll = -1;
    if ((poll = esp_transport_poll_read(t, timeout_ms)) <= 0) {
        return poll;
    }
    int read_len = espm_read(tcp->sock, buffer, len);
    if (read_len == 0) {
        return -1;
    }
    return read_len;
}

static int tcp_poll_read(esp_transport_handle_t t, int timeout_ms)
{
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(tcp->sock, &readset);
    struct timeval timeout;
    esp_transport_utils_ms_to_timeval(timeout_ms, &timeout);
    return espm_select(tcp->sock + 1, &readset, NULL, NULL, &timeout);
}

static int tcp_poll_write(esp_transport_handle_t t, int timeout_ms)
{
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(tcp->sock, &writeset);
    struct timeval timeout;
    esp_transport_utils_ms_to_timeval(timeout_ms, &timeout);
    return espm_select(tcp->sock + 1, NULL, &writeset, NULL, &timeout);
}

static int tcp_close(esp_transport_handle_t t)
{
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    int ret = -1;
    if (tcp->sock >= 0) {
        ret = espm_close(tcp->sock);
        tcp->sock = -1;
    }
    return ret;
}

static esp_err_t tcp_destroy(esp_transport_handle_t t)
{
    esp_transport_tcp_t *tcp = esp_transport_get_context_data(t);
    esp_transport_close(t);
    free(tcp);
    return 0;
}

esp_transport_handle_t esp_transport_tcp_init()
{
    esp_transport_handle_t t = esp_transport_init();
    esp_transport_tcp_t *tcp = calloc(1, sizeof(esp_transport_tcp_t));
    ESP_TRANSPORT_MEM_CHECK(TAG, tcp, return NULL);
    tcp->sock = -1;
    esp_transport_set_func(t, tcp_connect, tcp_read, tcp_write, tcp_close, tcp_poll_read, tcp_poll_write, tcp_destroy);
    esp_transport_set_context_data(t, tcp);

    return t;
}
