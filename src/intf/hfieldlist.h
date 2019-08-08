/*
  hfieldlist.h - Simple list of "A"="B"
  Copyright (c) 2019 Glenn Ramalho - RFIDo Design. All rights reserved.

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

  This was based off the work covered by the following license:
     Copyright (c) 2019 Ivan Grokhotkov. All rights reserved.

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.
*/

#ifndef _HFIELDLIST
#define _HFIELDLIST

#include <string>
#include <vector>

class hfield {
    public:
       hfield(std::string _name, std::string _value);
       hfield();
       std::string name;
       std::string value;
};

class hfieldlist {
   public:
      hfieldlist();
      void deleteArgs();
      void regArg(const char *name, const char *value);
      bool setArg(const char *name, const char *value);
      bool setArg(const char *name, int value);
      bool hasArg(const char *name);
      std::string arg(const char *name);
      int argN(const char *name);
      int args();
      std::string argName(int i);
      std::string arg(int i);

   private:
      std::vector<hfield> _arg;
};

#endif
