/*******************************************************************************
 * crccalc.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Generic class to compute the CRC32 for a value using the precalculated
 *   index table method.
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

#include "crccalc.h"

uint32_t crcTable[256];

void crctable_init() {
   const uint32_t polynomial = 0x04C11DB7;
   uint32_t divident;
   uint32_t current;
   int bit;

   for (divident = 0; divident < 256; divident++) {
      current = divident << 24;
      for (bit = 0; bit < 8; bit++) {
         if ((current & 0x80000000U) != 0) {
            current = (current << 1) ^ polynomial;
         }
         else current <<= 1;
      }

      crcTable[divident] = current;
   }
}

uint32_t do1crc(uint32_t crc, uint8_t b) {
   uint8_t pos = (uint8_t)((crc ^ (b << 24)) >> 24);
   crc = (uint32_t)((crc << 8) ^ crcTable[pos]);
   return crc;
}
