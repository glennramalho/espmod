/*******************************************************************************
 * info.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Provides simple shortcuts to the SC_REPORT_* macros to make it easy to use
 *   with printf style arguments.
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

#ifndef _INFO_H
#define _INFO_H

#include <stdio.h>

/* We implemented these as variadric defines so that when the function reports
 * the line number where the error occured it will point to the place where
 * the report function was used and not just point to some report functions
 * file.
 * There is one limitation with this approach. C99 and C++11 require at least
 * one argument to be passed, although GNU does not require it. To keep
 * compatability with other compilers, we included the format argument as one
 * of the variadric arguments.
 */
#define PRINTF_INFO(unit, ...) \
{ \
   char buffer[128], request[128];\
   snprintf(request, 128, __VA_ARGS__); \
   snprintf(buffer,128,"%s @ %s",request,sc_time_stamp().to_string().c_str()); \
   SC_REPORT_INFO(unit, buffer); \
}
#define PRINTF_FATAL(unit, ...) \
{ \
   char buffer[128];\
   snprintf(buffer, 128, __VA_ARGS__); \
   SC_REPORT_FATAL(unit, buffer); \
}
#define PRINTF_ERROR(unit, ...) \
{ \
   char buffer[128];\
   snprintf(buffer, 128, __VA_ARGS__); \
   SC_REPORT_ERROR(unit, buffer); \
}
#define PRINTF_WARN(unit, ...) \
{ \
   char buffer[128];\
   snprintf(buffer, 128, __VA_ARGS__); \
   SC_REPORT_WARNING(unit, buffer); \
}

#endif
