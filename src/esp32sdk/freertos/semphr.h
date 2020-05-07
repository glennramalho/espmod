/*******************************************************************************
 * portmacro.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is an adaptation of the semaphore macros for FreeRTOS to get them to
 *   work with SystemC's sc_semaphores. This is not a port of FreeRTOS. It just
 *   emulates its behavior on the ESP32 model.
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
 *  FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
 *  All rights reserved
 *
 *  VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.
 *
 *  This file is part of the FreeRTOS distribution.
 *
 *  FreeRTOS is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by the
 *  Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.
 *
 *  ***************************************************************************
 *  >>!   NOTE: The modification to the GPL is included to allow you to     !<<
 *  >>!   distribute a combined work that includes FreeRTOS without being   !<<
 *  >>!   obliged to provide the source code for proprietary components     !<<
 *  >>!   outside of the FreeRTOS kernel.                                   !<<
 *  ***************************************************************************
 *
 *  FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 *  link: http://www.freertos.org/a00114.html
 *
 *  ***************************************************************************
 *   *                                                                       *
 *   *    FreeRTOS provides completely free yet professionally developed,    *
 *   *    robust, strictly quality controlled, supported, and cross          *
 *   *    platform software that is more than just the market leader, it     *
 *   *    is the industry's de facto standard.                               *
 *   *                                                                       *
 *   *    Help yourself get started quickly while simultaneously helping     *
 *   *    to support the FreeRTOS project by purchasing a FreeRTOS           *
 *   *    tutorial book, reference manual, or both:                          *
 *   *    http://www.FreeRTOS.org/Documentation                              *
 *   *                                                                       *
 *  ***************************************************************************
 *
 *  http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
 *  the FAQ page "My application does not run, what could be wrong?".  Have you
 *  defined configASSERT()?
 *
 *  http://www.FreeRTOS.org/support - In return for receiving this top quality
 *  embedded software for free we request you assist our global community by
 *  participating in the support forum.
 *
 *  http://www.FreeRTOS.org/training - Investing in training allows your team to
 *  be as productive as possible as early as possible.  Now you can receive
 *  FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
 *  Ltd, and the world's leading authority on the world's leading RTOS.
 *
 *  http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
 *  including FreeRTOS+Trace - an indispensable productivity tool, a DOS
 *  compatible FAT file system, and our tiny thread aware UDP/IP stack.
 *
 *  http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
 *  Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.
 *
 *  http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
 *  Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
 *  licenses offer ticketed support, indemnification and commercial middleware.
 *
 *  http://www.SafeRTOS.com - High Integrity Systems also provide a safety
 *  engineered and independently SIL3 certified version for use in safety and
 *  mission critical applications that require provable dependability.
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

typedef void* xSemaphoreHandle;

/*************************
 * Task: xSemaphoreTake()
 *************************
 * To use this the gn_semaphore must have previously been created with a
 * cell of one position and available to be taken.
 */
void xSemaphoreTake( xSemaphoreHandle xSemaphore, int xBlockTime );

/*************************
 * Task: xSemaphoreTakeRecursive()
 *************************
 * For now it is the same thing as the regular take.
 */
#define xSemaphoreTakeRecursive( xMutex, xBlockTime ) \
   xSemaphoreTake( (xMutex), (xBlockTime))

/*************************
 * Task: xSemaphoreAltTake()
 *************************
 * This is the same as the regular semaphore take.
 */
#define xSemaphoreAltTake( xSemaphore, xBlockTime ) \
   xSemaphoreTake((xSemaphore), (xBlockTime))

/*************************
 * Task: xSemaphoreGive()
 *************************
 * Returns a value or posts a value to the semaphore.
 */
void xSemaphoreGive( xSemaphoreHandle xSemaphore );

/*************************
 * Task: xSemaphoreGiveRecursive()
 *************************
 * Returns a value or posts a value to the semaphore. For now, this is the
 * same as a regular give.
 */
#define xSemaphoreGiveRecursive( xMutex ) xSemaphoreGive ( xMutex )

/*************************
 * Task: xSemaphoreAltGive()
 *************************
 * Returns a value or posts a value to the semaphore. There is no need to make
 * a separate one.
 */
#define xSemaphoreAltGive( xSemaphore ) xSemaphoreGive ( xSemaphore )

/*************************
 * Task: xSemaphoreGiveFromISR()
 *************************
 * We do not have any special ISR handling, so regular semaphores work fine.
 * We do not have preemption though, so we have to ignore the argument.
 */
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )  \
   xSemaphoreGive ( xSemaphore )

/*************************
 * Task: xSemaphoreTakeFromISR()
 *************************
 * We do not have any special ISR handling, so regular semaphores work fine.
 * We do not have preemption though, so we have to ignore the argument.
 */
#define xSemaphoreTakeFromISR( xSemaphore, pxHigherPriorityTaskWoken )  \
   xSemaphoreTake ( xSemaphore )

/**
 * Deleteting semaphores is not supported.
 */
#define vSemaphoreDelete( xSemaphore )

/*************************
 * Task: xSemaphoreGetCount()
 *************************
 * Simply returns the count value.
 */
#define uxSemaphoreGetCount( xSemaphore ) xSemaphore.get_value()

#endif /* _SEMAPHORE_H */
