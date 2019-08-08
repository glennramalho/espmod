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

#include <stdlib.h>

static bool inline psramInit() { return true; }
static bool inline psramFound() { return true; }

static void inline *ps_malloc(size_t size) {return malloc(size); }
static void inline *ps_calloc(size_t n, size_t size) {
   return calloc(n, size);
}
void *ps_realloc(void *ptr, size_t size) {
   return realloc(ptr, size);
}
