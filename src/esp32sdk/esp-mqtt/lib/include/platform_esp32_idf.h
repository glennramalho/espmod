/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 * Tuan PM <tuanpm at live dot com>
 *
 * This file was modified by Glenn Ramalho on 29 July 2019 to port it to the
 * ESPMOD SystemC model.
 */
#ifndef _ESP_PLATFORM_H__
#define _ESP_PLATFORM_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
//#include "freertos/queue.h"
#include "freertos/event_groups.h"

/*
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
*/

#include "sys/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

char *platform_create_id_string(void);
int platform_random(int max);
long long platform_tick_get_ms(void);
void ms_to_timeval(int timeout_ms, struct timeval *tv);

#define ESP_MEM_CHECK(TAG, a, action) 

#ifdef __cplusplus
}
#endif

#endif
