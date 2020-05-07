/*******************************************************************************
 * semphr.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a stubfile that later should be replaced with hooks into the
 *   SystemC Semaphores or Mutex.  This is not a port of FreeRTOS.
 *******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This file was based off the work covered by the license below:
 *    FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
 *    All rights reserved
 *
 *   FreeRTOS is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License (version 2) as published by the
 *   Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.
 *
 *   FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 *   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 *   link: http://www.freertos.org/a00114.html
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#define vSemaphoreDelete( xSemaphore )
#define xSemaphoreCreateMutex()                     ((void*)(1))
#define xSemaphoreGive( xSemaphore )
#define xSemaphoreTake( xSemaphore, xBlockTime )    pdTRUE

typedef void* SemaphoreHandle_t;

#if defined(__cplusplus)
}
#endif
