/*******************************************************************************
 * FreeRTOS.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
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

/* This is a stub file to get the model to compile without having to port the
 * whole FreeRTOS to the model.
 */
#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H
#define PRIVILEGED_FUNCTION
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#endif
