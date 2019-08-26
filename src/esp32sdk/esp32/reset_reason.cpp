// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Modified by Glenn Ramalho 26 Aug 2019
// - adapted to work with the ESP32 model

#include "esp_system.h"
#include "reset_reason.h"
#include "rom/rtc.h"
#include "esp_attr.h"

void esp_reset_reason_clear_hint();
esp_reset_reason_t esp_reset_reason_get_hint();

static esp_reset_reason_t s_reset_reason;
uint32_t __reset_reason_hint;

esp_reset_reason_t get_reset_reason(RESET_REASON rtc_reset_reason, esp_reset_reason_t reset_reason_hint)
{
    switch (rtc_reset_reason) {
        case POWERON_RESET:
            return ESP_RST_POWERON;

        /* For ESP32, ESP_RST_EXT is never returned */


        case SW_CPU_RESET:
        case SW_RESET:
        case EXT_CPU_RESET: /* unused */
            if (reset_reason_hint == ESP_RST_PANIC ||
                reset_reason_hint == ESP_RST_BROWNOUT ||
                reset_reason_hint == ESP_RST_TASK_WDT ||
                reset_reason_hint == ESP_RST_INT_WDT) {
                return reset_reason_hint;
            }
            return ESP_RST_SW;

        case DEEPSLEEP_RESET:
            return ESP_RST_DEEPSLEEP;

        case TG0WDT_SYS_RESET:
            return ESP_RST_TASK_WDT;

        case TG1WDT_SYS_RESET:
            return ESP_RST_INT_WDT;

        case OWDT_RESET:
        case RTCWDT_SYS_RESET:
        case RTCWDT_RTC_RESET:
        case RTCWDT_CPU_RESET:  /* unused */
        case TGWDT_CPU_RESET:   /* unused */
            return ESP_RST_WDT;

        case RTCWDT_BROWN_OUT_RESET:    /* unused */
            return ESP_RST_BROWNOUT;

        case SDIO_RESET:
            return ESP_RST_SDIO;

        case INTRUSION_RESET: /* unused */
        default:
            return ESP_RST_UNKNOWN;
    }
}

void  esp_reset_reason_init(void)
{
    esp_reset_reason_t hint = esp_reset_reason_get_hint();
    s_reset_reason = get_reset_reason(rtc_get_reset_reason(PRO_CPU_NUM), hint);
    if (hint != ESP_RST_UNKNOWN) {
        esp_reset_reason_clear_hint();
    }
}

esp_reset_reason_t esp_reset_reason(void)
{
    return s_reset_reason;
}

#define RST_REASON_BIT  0x80000000
#define RST_REASON_MASK 0x7FFF
#define RST_REASON_SHIFT 16

/* in IRAM, can be called from panic handler */
void esp_reset_reason_set_hint(esp_reset_reason_t hint)
{
    assert((hint & (~RST_REASON_MASK)) == 0);
    __reset_reason_hint = hint | (hint << RST_REASON_SHIFT) | RST_REASON_BIT;
}

/* in IRAM, can be called from panic handler */
esp_reset_reason_t esp_reset_reason_get_hint(void)
{
    uint32_t reset_reason_hint = __reset_reason_hint;
    uint32_t high = (reset_reason_hint >> RST_REASON_SHIFT) & RST_REASON_MASK;
    uint32_t low = reset_reason_hint & RST_REASON_MASK;
    if ((reset_reason_hint & RST_REASON_BIT) == 0 || high != low) {
        return ESP_RST_UNKNOWN;
    }
    return (esp_reset_reason_t) low;
}
void esp_reset_reason_clear_hint()
{
    __reset_reason_hint = 0;
}
