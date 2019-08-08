// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

#include "platform.h"

#include "esp_system.h"
#include "esp_log.h"
#include <stdlib.h>
#include <sys/time.h>

static const char *TAG = "PLATFORM";

#define MAX_ID_STRING (32)

/*
char *platform_create_id_string()
{
    uint8_t mac[6];
    char *id_string = calloc(1, MAX_ID_STRING);
    ESP_MEM_CHECK(TAG, id_string, return NULL);
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(id_string, "ESP32_%02x%02X%02X", mac[3], mac[4], mac[5]);
    return id_string;
}

int platform_random(int max)
{
    return esp_random()%max;
}

long long platform_tick_get_ms()
{
    struct timeval te;
    esp_gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
*/

void esp_transport_utils_ms_to_timeval(int timeout_ms, struct timeval *tv)
{
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms - (tv->tv_sec * 1000)) * 1000;
}
