/*******************************************************************************
 * setfield.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Defines some fields for doing bit manipulations in sc_signals.
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
 *******************************************************************************
 */

#ifndef _SETFIELD_H
#define _SETFIELD_H

/* The set macro is put in an ifdef C++ as it deals with SystemC signals. */
#ifdef __cplusplus
#define SETFIELD(sig, mask, val, shift) \
   sig.write(sig.read() & ~(mask) | (mask) & (((uint32_t)(val)<<(shift))))
#define RDFIELD(sig, mask, shift) ((sig.read() & (mask)) >> (shift))
#endif

#endif
