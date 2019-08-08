/*
 Esp.cpp - Main Arduino-ESP32 Model Functions
 Copyright (c) 2019 Glenn Ramalho - RFIDo. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 This file was based off the work covered by the license below:

    Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 */

#include "Arduino.h"
#include "Esp.h"
#include "esp_spi_flash.h"
#include <memory>
#include <soc/soc.h>
#include <systemc.h>

/**
 * User-defined Literals
 *  usage:
 *
 *   uint32_t = test = 10_MHz; // --> 10000000
 */

unsigned long long operator"" _kHz(unsigned long long x)
{
    return x * 1000;
}

unsigned long long operator"" _MHz(unsigned long long x)
{
    return x * 1000 * 1000;
}

unsigned long long operator"" _GHz(unsigned long long x)
{
    return x * 1000 * 1000 * 1000;
}

unsigned long long operator"" _kBit(unsigned long long x)
{
    return x * 1024;
}

unsigned long long operator"" _MBit(unsigned long long x)
{
    return x * 1024 * 1024;
}

unsigned long long operator"" _GBit(unsigned long long x)
{
    return x * 1024 * 1024 * 1024;
}

unsigned long long operator"" _kB(unsigned long long x)
{
    return x * 1024;
}

unsigned long long operator"" _MB(unsigned long long x)
{
    return x * 1024 * 1024;
}

unsigned long long operator"" _GB(unsigned long long x)
{
    return x * 1024 * 1024 * 1024;
}

EspClass ESP;

void EspClass::deepSleep(uint32_t time_us)
{
   /* We do not have support now for sleep and deep-sleep, so all we do is
    * print a notice. We can expand this later.
    */
   char buffer[80];
   snprintf(buffer, 80, "Request to enter Deep Sleep @ %s",
      sc_time_stamp().to_string().c_str());
   SC_REPORT_INFO("ESP", buffer);
   wait(time_us, SC_US);
   snprintf(buffer, 80, "Wake up from sleep @ %s",
      sc_time_stamp().to_string().c_str());
   SC_REPORT_INFO("ESP", buffer);
}

uint32_t EspClass::getCycleCount()
{
   /* We do not have a cycle count, so we get the time in nanoseconds,
    * convert it to clock cycles and return that.
    */
   double ct = sc_time_stamp().to_seconds();
   double cycles = ct * (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1.0e6);
   return (uint32_t)floor(cycles);
}

void EspClass::restart(void)
{
   /* For now we still do not support restarting. */
   char buffer[80];
   snprintf(buffer, 80, "Request to Restart @ %s",
      sc_time_stamp().to_string().c_str());
   SC_REPORT_INFO("ESP", buffer);
}

uint32_t EspClass::getHeapSize(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getFreeHeap(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getMinFreeHeap(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getMaxAllocHeap(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getPsramSize(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getFreePsram(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getMinFreePsram(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getMaxAllocPsram(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint32_t EspClass::getSketchSize () {
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 5 * 1024;
}

String EspClass::getSketchMD5()
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    char buffer[33];
    String md5;
    sprintf(buffer, "%08x%08x%08x%08x", rand(), rand(), rand(), rand());
    md5 = buffer;
    return md5;
}

uint32_t EspClass::getFreeSketchSpace () {
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return 32 * 1024;
}

uint8_t EspClass::getChipRevision(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return rand();
}

const char * EspClass::getSdkVersion(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return "SIMULATION";
}

uint32_t EspClass::getFlashChipSize(void)
{
   /* TODO: this should be taken from the flash model at init */
   return magicFlashChipSize(0x4);
}

uint32_t EspClass::getFlashChipSpeed(void)
{
   /* TODO: this should be taken from the flash model at init */
   return magicFlashChipSize(0x0);
}

FlashMode_t EspClass::getFlashChipMode(void)
{
   /* TODO: this should be taken from the flash model at init */
   return magicFlashChipMode(FM_UNKNOWN);
}

uint32_t EspClass::magicFlashChipSize(uint8_t byte)
{
    switch(byte & 0x0F) {
    case 0x0: // 8 MBit (1MB)
        return (1_MB);
    case 0x1: // 16 MBit (2MB)
        return (2_MB);
    case 0x2: // 32 MBit (4MB)
        return (4_MB);
    case 0x3: // 64 MBit (8MB)
        return (8_MB);
    case 0x4: // 128 MBit (16MB)
        return (16_MB);
    default: // fail?
        return 0;
    }
}

uint32_t EspClass::magicFlashChipSpeed(uint8_t byte)
{
    switch(byte & 0x0F) {
    case 0x0: // 40 MHz
        return (40_MHz);
    case 0x1: // 26 MHz
        return (26_MHz);
    case 0x2: // 20 MHz
        return (20_MHz);
    case 0xf: // 80 MHz
        return (80_MHz);
    default: // fail?
        return 0;
    }
}

FlashMode_t EspClass::magicFlashChipMode(uint8_t byte)
{
    FlashMode_t mode = (FlashMode_t) byte;
    if(mode > FM_SLOW_READ) {
        mode = FM_UNKNOWN;
    }
    return mode;
}

bool EspClass::flashEraseSector(uint32_t sector)
{
    return spi_flash_erase_sector(sector) == ESP_OK;
}

// Warning: These functions do not work with encrypted flash
bool EspClass::flashWrite(uint32_t offset, uint32_t *data, size_t size)
{
    return spi_flash_write(offset, (uint32_t*) data, size) == ESP_OK;
}

bool EspClass::flashRead(uint32_t offset, uint32_t *data, size_t size)
{
    return spi_flash_read(offset, (uint32_t*) data, size) == ESP_OK;
}


uint64_t EspClass::getEfuseMac(void)
{
    /* This does not make any sense to the simulation, so we return something
     * random to please the caller.
     */
    return (((uint64_t)rand())<<32) | (uint64_t)rand();
}
