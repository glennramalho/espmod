#include "simnetdb.h"
#include "lwip/sockets.h"
#include <stdio.h>

char ipa[4];
char *ipalist[] = {ipa, NULL};
/*******************************************************************************
 * simnetdb.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the gethostbyname() function to emulate some of the
 *   netdb.h functionality.
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
 */

char *aliaslist[] = {NULL};

struct hostent resp;
struct hostent *gethostbyname (const char *__name) {
   int v;
   unsigned int v1, v2, v3, v4;
   /* This is not quite how it works, but it is good enough for now. */
   v = sscanf(__name, "%u.%u.%u.%u", &v1, &v2, &v3, &v4);
   if (v == 4 && v1 <= 255 && v2 <= 255 && v3 <= 255 && v4 <= 255) {
      ipa[0] = v1;
      ipa[1] = v2;
      ipa[2] = v3;
      ipa[3] = v4;
   }
   else {
      ipa[0] = 0;
      ipa[1] = 0;
      ipa[2] = 0;
      ipa[3] = 0;
   }

   resp.h_name = __name;
   resp.h_aliases = aliaslist;
   resp.h_addrtype = AF_INET;
   resp.h_length = 4;
   resp.h_addr_list = ipalist;
   return &resp;
}
