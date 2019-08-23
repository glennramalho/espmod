/*******************************************************************************
 * espm_abort.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file defines the prototype for the ESPM abort. This function should
 *   be called instead of abort() to end the simulations normally.
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
 */

#ifndef _ESPM_ABORT_H
#define _ESPM_ABORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* This function replaces the abort() function of the system. It is to be used
 * as it will stop the SystemC simulation nicely instead of calling the system's
 * abort function and perhaps leave some waveform file unfinished.
 */
void espm_abort();

#ifdef __cplusplus
}
#endif

#endif /* __ESP_SYSTEM_H__ */
