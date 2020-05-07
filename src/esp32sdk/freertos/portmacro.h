/*******************************************************************************
 * portmacro.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file sets a few defines used by the FreeRTOS so we can get it to
 *   compile. This is not a port of FreeRTOS. It is just a set of defines to
 *   get the existing files to compile without having to port the whole
 *   FreeRTOS.
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

#ifndef _PORTMACRO_H
#define _PORTMACRO_H

#include <stdint.h>
typedef uint32_t StackType_t;
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef int portBASE_TYPE;
typedef unsigned int UBaseType_t;
#define portTICK_RATE_MS 1
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#define portNUM_CONFIGURABLE_REGIONS 1
/* This is not quite it but will work for now. */
#define portYIELD_FROM_ISR() yield()

/* These were changed to call SystemC semaphores. The semaphore should
 * be declared locally. Some need to make sure they are not called from ISRs,
 * the others are safe. Being we currently do not know, we have to just call
 * the function and ignore this.
 */
#define portENTER_CRITICAL(mux) (mux)->trywait()
#define portEXIT_CRITICAL(mux) (mux)->post()
#define portENTER_CRITICAL_SAFE(mux) (mux)->trywait()
#define portEXIT_CRITICAL_SAFE(mux) (mux)->post()
#define portENTER_CRITICAL_ISR(mux) (mux)->trywait()
#define portEXIT_CRITICAL_ISR(mux) (mux)->post()

/* We do not really have a system to report which is the CPU in the simulation.
 * Therefore for now we just return always cpu zero. Maybe in the future we
 * could set some parameter for spawned processes to carry foreward. So we
 * always return the app cpu.
 */
static inline uint32_t xPortGetCoreID() { return 1; }

#endif
