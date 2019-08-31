/*******************************************************************************
 * esp_spi_flash.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the esp_spi flash functions for the ESP32 to the ESPMOD
 *   SystemC model. It was based off the functions from Espressif Systems.
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
 *    Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <systemc.h>
#include "Arduino.h"
#include "TestSerial.h"
#include <stdint.h>
typedef uint32_t uint32;
typedef uint16_t uint16;
#include "esp_spi_flash.h"
/* This should be changed to a more generic method so that this file does not
 * depend on the board module, but for now, this will work. TODO
 */
#include "doitesp32devkitv1.h"

TestSerial Flashport;
sc_mutex flashmutex;

/* This function must be called once. Being it is already called by the startup
 * there is no need to check if the user called it too.
 */
void spi_flash_init() {
}

/* This is a support function to convert the model flash errors to the ESP
 * error codes. For now it is quite simple.
 */
esp_err_t retflerr(uint8_t code) {
   if (code == 0) return ESP_OK;
   else return ESP_FAIL;
}

/* This should be changed to talk to the TB but for now it just gives
 * back the size.
 */
size_t spi_flash_get_chip_size() {
   return 1024 * 1024 * 4;
}

uint32 spi_flash_get_id(void) {
   return rand(); /* Not sure what it should be so we will guess something. */
}

/* Erase functions */
esp_err_t spi_flash_erase_sector(size_t sec) {
   esp_err_t resp;
   flashmutex.lock();
   Flashport.printf("e:%0x\r\n", sec);
   while(Flashport.available()==0) delay(1);
   resp = retflerr(Flashport.read());
   flashmutex.unlock();
   return resp;
}

esp_err_t spi_flash_erase_range(size_t start_address, size_t size) {
   size_t cnt;
   esp_err_t resp;
   esp_err_t accumerr;
   if ((start_address % 4096)>0) {
      SC_REPORT_ERROR("SCFLASH", "Starting address must be 4kB alligned.");
   }
   if ((size % 4096)>0) {
      SC_REPORT_ERROR("SCFLASH", "Erase Size must be 4kB alligned.");
   }
   accumerr = ESP_OK;
   for(cnt = 0; cnt < (size >> 12); cnt = cnt + 1) {
      resp = spi_flash_erase_sector((start_address >> 12) + cnt);
      if (accumerr == ESP_OK) accumerr = resp;
   }
   return accumerr;
}

/* Write Functions */
esp_err_t spi_flash_write(size_t des_addr, const void *src_addr, size_t size) {
   size_t pos;
   uint32_t val;
   size_t sizew = ceil(size/4);
   uint32_t *src;
   esp_err_t resp;
   src = (uint32_t *)src_addr;
   if (size > 256) 
      SC_REPORT_ERROR("SCFLASH",
         "The flash does not support more than 256 bytes at a time");
   flashmutex.lock();
   Flashport.printf("w:%0x %u\r\n", des_addr, sizew);
   Flashport.printf("d:");
   for(pos = 0; pos < sizew; pos = pos + 1) {
      val = src[pos];
      Flashport.write(val & 0xff);
      Flashport.write((val>>8) & 0xff);
      Flashport.write((val>>16) & 0xff);
      Flashport.write((val>>24) & 0xff);
   }
   Flashport.printf("\r\n");
   while(Flashport.available()==0) delay(1);
   resp = retflerr(Flashport.read());
   flashmutex.unlock();
   return resp;
}

esp_err_t spi_flash_write_encrypted(size_t des_addr, size_t *src_addr,
     size_t size) {
   /* We do not support yet encryption. */
   return ESP_ERR_INVALID_STATE;
}

/* Read Functions */
esp_err_t spi_flash_read(size_t src_addr, void *des_addr, size_t size) {
   size_t pos;
   uint32_t val;
   size_t sizew = ceil(size/4);
   uint32_t *dest;
   esp_err_t resp;
   dest = (uint32_t *)des_addr;
   /* We only support 256 bytes at a time, so we break up larger requests into
    * smaller ones.
    */
   if (size > 256) {
      esp_err_t resp;
      /* We keep doing 256 bytes at a time or until an error shows up. */
      do {
         resp = spi_flash_read(src_addr, des_addr, 256);
         size = size - 256;
         des_addr = (char *)des_addr + 256;
         src_addr = src_addr + 256;
      } while(size > 256 && resp == ESP_OK);
      /* If an error came up, we quit and report. */
      if (resp != ESP_OK) return resp;
      /* If there is still something left, we do it now. */
      if (size > 0) return spi_flash_read(src_addr, des_addr, size);
      else return ESP_OK;
   }
   /* If the size is 0, we just return success. */
   if (size == 0) return ESP_OK;

   /* Any other sizes, we process. */
   flashmutex.lock();
   Flashport.printf("r:%0x %u\r\n", src_addr, sizew);
   for(pos = 0; pos < sizew; pos = pos + 1) {
      while(Flashport.available() < 4) delay(1);
      val = Flashport.read();
      val = val | (((size_t)Flashport.read()) << 8);
      val = val | (((size_t)Flashport.read()) << 16);
      val = val | (((size_t)Flashport.read()) << 24);
      dest[pos] = val;
   }
   while(Flashport.available() < 1) delay(1);
   resp = retflerr(Flashport.read());
   flashmutex.unlock();
   return resp;
}
esp_err_t spi_flash_read_encrypted(size_t src_addr, void *des_addr,
      size_t size) {
   /* As stated in the main file, when encryption is disabled, we do the
    * standard read. */
   return spi_flash_read(src_addr, des_addr, size);
}

/* These functions are not supported so they do nothing. */
esp_err_t spi_flash_mmap(size_t src_addr, size_t size,
      spi_flash_mmap_memory_t memory, const void** out_ptr,
      spi_flash_mmap_handle_t* out_handle) {
   return ESP_OK;
}
esp_err_t spi_flash_mmap_pages(const int *pages, size_t page_count,
      spi_flash_mmap_memory_t memory, const void** out_ptr,
      spi_flash_mmap_handle_t* out_handle) {
   return ESP_ERR_NO_MEM;
}
void spi_flash_munmap(spi_flash_mmap_handle_t handle) {}
void spi_flash_mmap_dump() {}
size_t spi_flash_mmap_get_free_pages(spi_flash_mmap_memory_t memory) {
   return 0;
}
size_t spi_flash_cache2phys(const void *cached) {return 0; }

const void *spi_flash_phys2cache(size_t phys_offs,
      spi_flash_mmap_memory_t memory) { return NULL; }
bool spi_flash_cache_enabled() { return false; }
void spi_flash_guard_set(const spi_flash_guard_funcs_t* funcs) {}
const spi_flash_guard_funcs_t *spi_flash_guard_get() {return NULL; }
void spi_flash_reset_counters() {}
void spi_flash_dump_counters() {}
