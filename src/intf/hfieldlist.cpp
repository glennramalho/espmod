/*
  hfieldlist.cpp - Simple list of "A"="B"
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

#include "hfieldlist.h"

hfield::hfield(std::string _name, std::string _value):
   name(_name), value(_value) { }
hfield::hfield(): name(""), value("") { }

/* We need to create this empty constructor or std::vector's constructor does
 * not get called.
 */
hfieldlist::hfieldlist(): _arg() { }

void hfieldlist::deleteArgs() { if (_arg.size() != 0) _arg.resize(0); }
void hfieldlist::regArg(const char *name, const char *value) {
   _arg.resize(_arg.size()+1);
   _arg[_arg.size()-1].name = name;
   _arg[_arg.size()-1].value = value;
}
bool hfieldlist::setArg(const char *name, const char *value) {
   unsigned int hnd;
   for(hnd = 0; hnd < _arg.size(); hnd = hnd + 1) {
      if (_arg[hnd].name == name) {
         _arg[hnd].value = value;
         return true;
      }
   }
   return false;
}
bool hfieldlist::setArg(const char *name, int value) {
   char v[10];
   snprintf(v, 10, "%d", value);
   return setArg(name, v);
}
bool hfieldlist::hasArg(const char *name) {
   unsigned int hnd;
   for(hnd = 0; hnd < _arg.size(); hnd = hnd + 1) {
      if (_arg[hnd].name == name) return true;
   }
   return false;
}
std::string hfieldlist::arg(const char *name) {
   unsigned int hnd;
   for(hnd = 0; hnd < _arg.size(); hnd = hnd + 1) {
      if (_arg[hnd].name == name) return _arg[hnd].value;
   }
   return std::string("");
}
int hfieldlist::argN(const char *name) {
   return atoi(arg(name).c_str());
}
int hfieldlist::args() { return _arg.size(); }
std::string hfieldlist::argName(int i) { return _arg[i].name; }
std::string hfieldlist::arg(int i) { return _arg[i].value; }
