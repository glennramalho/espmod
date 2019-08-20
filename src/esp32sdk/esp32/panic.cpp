/*******************************************************************************
 * panic.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file replaces the run time error functions from the ESP32 with
 *   equivalents that will kill the model and print a message on the screen.
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
#include <stdlib.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/*
  Note: The linker script will put everything in this file in IRAM/DRAM, so it also works with flash cache disabled.
*/

void  __attribute__((weak)) vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
    printf("***ERROR*** A stack overflow in task ");
    printf((char *)pcTaskName);
    printf(" has been detected.\r\n");
    abort();
}

/* These two weak stubs for esp_reset_reason_{get,set}_hint are used when
 * the application does not call esp_reset_reason() function, and
 * reset_reason.c is not linked into the output file.
 */
void __attribute__((weak)) esp_reset_reason_set_hint(esp_reset_reason_t hint)
{
}

esp_reset_reason_t __attribute__((weak)) esp_reset_reason_get_hint(void)
{
    return ESP_RST_UNKNOWN;
}

static inline void invoke_abort()
{
   SC_REPORT_FATAL("PANIC", "Abort Called");
}

void abort()
{
    printf("abort() was called\n");
    /* Calling code might have set other reset reason hint (such as Task WDT),
     * don't overwrite that.
     */
    if (esp_reset_reason_get_hint() == ESP_RST_UNKNOWN) {
        esp_reset_reason_set_hint(ESP_RST_PANIC);
    }
    invoke_abort();
}

/*
  This disables all the watchdogs for when we call the gdbstub.
*/
static void esp_panic_dig_reset()
{
    SC_REPORT_FATAL("PANIC", "Panic Dig Reset");
}

void esp_set_breakpoint_if_jtag(void *fn)
{
   PRINTF_INFO("PANIC", "Set breakpoint %p", fn);
}


esp_err_t esp_set_watchpoint(int no, void *adr, int size, int flags)
{
    if (no < 0 || no > 1) {
        return ESP_ERR_INVALID_ARG;
    }
    if (flags & (~0xC0000000)) {
        return ESP_ERR_INVALID_ARG;
    }
    PRINTF_INFO("PANIC", "Setting watchpoint %d addr=%p size=%d flags=%x",
       no, adr, size, flags);
    return ESP_OK;
}

void esp_clear_watchpoint(int no)
{
    PRINTF_INFO("PANIC", "Clearing watchpoint %d", no);
}

static void esp_error_check_failed_print(const char *msg, esp_err_t rc, const char *file, int line, const char *function, const char *expression)
{
    printf("%s failed: esp_err_t 0x%x", msg, rc);
}

void _esp_error_check_failed_without_abort(esp_err_t rc, const char *file, int line, const char *function, const char *expression)
{
    esp_error_check_failed_print("ESP_ERROR_CHECK_WITHOUT_ABORT", rc, file, line, function, expression);
}

void _esp_error_check_failed(esp_err_t rc, const char *file, int line, const char *function, const char *expression)
{
    esp_error_check_failed_print("ESP_ERROR_CHECK", rc, file, line, function, expression);
    invoke_abort();
}
