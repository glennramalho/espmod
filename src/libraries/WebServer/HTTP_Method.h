/*
  HTTP Method.h - HTTP command codes
  Supports only one simultaneous client, knows how to handle GET and POST.

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

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
  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)

  Modified by Glenn Ramalho - RFIDo Design - 29 July 2019
    - minor modification, changed the binary coded values to hex to eliminate
      warnings and not use a GCC only feature.
*/

#ifndef _HTTP_Method_H_
#define _HTTP_Method_H_

typedef enum {
  HTTP_GET     = 0x01,
  HTTP_POST    = 0x02,
  HTTP_DELETE  = 0x04,
  HTTP_PUT     = 0x08,
  HTTP_PATCH   = 0x10,
  HTTP_HEAD    = 0x20,
  HTTP_OPTIONS = 0x40,
  HTTP_ANY     = 0x7f,
} HTTPMethod;

#endif /* _HTTP_Method_H_ */
