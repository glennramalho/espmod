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

#ifndef _ESP32_HAL_LOG_H
#define _ESP32_HAL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

const char *pathToFileName(const char *path);
int log_printf(const char *fmt, ...);
int log_v(const char *fmt, ...);
int isr_log_v(const char *fmt, ...);
int log_i(const char *fmt, ...);
int isr_log_i(const char *fmt, ...);
int log_d(const char *fmt, ...);
int isr_log_d(const char *fmt, ...);
int log_w(const char *fmt, ...);
int isr_log_w(const char *fmt, ...);
int log_e(const char *fmt, ...);
int isr_log_e(const char *fmt, ...);
int log_n(const char *fmt, ...);
int isr_log_n(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
