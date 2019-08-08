/*******************************************************************************
 * task.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file reimplements the freertos tasks to get them to compile under the
 *   ESPMOD SystemC model. The tasks were rewrittent to spawn SC dynamic
 *   threads.
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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc.h>
#include "FreeRTOS.h"
#include "task.h"

void vTaskDelay( const TickType_t xTicksToDelay ) {
   /* We set the portTICK_RATE_MS to 1, so the value should be the
    * number in lilliseconds. Perhaps we should change this later.
    */
   wait(xTicksToDelay, SC_MS);
}

BaseType_t xTaskCreatePinnedToCore(	TaskFunction_t pvTaskCode,
   const char * const pcName, const uint32_t usStackDepth,
   void * const pvParameters, UBaseType_t uxPriority,
   TaskHandle_t * const pvCreatedTask, const BaseType_t xCoreID)
{
   sc_spawn(sc_bind(pvTaskCode, pvParameters));
   return pdTRUE;
}

void vTaskDelete( TaskHandle_t xTaskToDelete ) {
}


