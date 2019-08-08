// Copyright 2019 Glenn Ramalho - RFIDo Design
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This code was based off the work from Espressif Systems covered by the
// License:
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include <systemc.h>
#include "esp32-hal-log.h"
#include "esp_log.h"
#include <stdarg.h>
#include <stdlib.h>

const char *pathToFileName(const char *path) {
   const char *basename = strrchr(path, '/');
   if (basename == NULL) return ".";
   else return &(basename[1]);
}

int log_printf(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGP", buffer);
   return size;
}
int log_v(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGV", buffer);
   return size;
}
int isr_log_v(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGISRV", buffer);
   return size;
}
int log_i(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOG", buffer);
   return size;
}
int isr_log_i(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGISR", buffer);
   return size;
}
int log_d(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGDEBUG", buffer);
   return size;
}
int isr_log_d(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGISRD", buffer);
   return size;
}
int log_w(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_WARNING("LOG", buffer);
   return size;
}
int isr_log_w(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_WARNING("LOGISR", buffer);
   return size;
}
int log_e(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_WARNING("LOGERR", buffer);
   return size;
}
int isr_log_e(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_WARNING("LOGISRERR", buffer);
   return size;
}
int log_n(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGNOTE", buffer);
   return size;
}
int isr_log_n(const char *fmt, ...) {
   int size;
   char buffer[128];
   va_list ap;
   va_start(ap, fmt);
   size = vsnprintf(buffer, 128, fmt, ap);
   va_end(ap);
   SC_REPORT_INFO("LOGISRNOTE", buffer);
   return size;
}
esp_log_level_t esp_level;
void esp_log_level_set(const char *tag, esp_log_level_t level) {
   esp_level = level;
}
uint32_t esp_log_timestamp() {
   double ts = sc_time_stamp().to_seconds();
   return (uint32_t)(ts*1000);
}
uint32_t esp_log_early_timestamp() {
   double ts = sc_time_stamp().to_seconds();
   return (uint32_t)(ts*1000);
}
void esp_log_write(esp_log_level_t level, const char *tag, const char * format,
   ...) {
   if (level <= esp_level) {
      int size;
      char buffer[128];
      va_list ap;
      size = strlen(tag);
      strncpy(buffer, tag, 16);
      if (size > 16) size = 16;
      buffer[size] = ':';
      va_start(ap, format);
      vsnprintf(buffer+size+1, 128 - size - 1, format, ap);
      va_end(ap);
      switch(level) {
         case ESP_LOG_ERROR: SC_REPORT_INFO("LOGERR", buffer); break;
         case ESP_LOG_WARN: SC_REPORT_INFO("LOGWARN", buffer); break;
         case ESP_LOG_INFO: SC_REPORT_INFO("LOGINFO", buffer); break;
         case ESP_LOG_DEBUG: SC_REPORT_INFO("LOGDEBUG", buffer); break;
         case ESP_LOG_VERBOSE: SC_REPORT_INFO("LOGVERB", buffer); break;
         default : SC_REPORT_INFO("LOGOTHER", buffer); break;
      }
   }
}
