/*******************************************************************************
 * cchanflash.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC module for an integer wide UART protocol. It should
 *   eventually be replaced with a real QSPI interface, but for now this works
 *   for the ESPMOD project.
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


#include <systemc.h>
#include <vector>
#include "cchanflash.h"

#define SECADDR(range, addr) (secstart[range]+(((addr)-rangestart[range])>>12))
#define PAGEADDRINSEC(range, addr) ((secstart[range]<<4)+((addr)>>8))
#define PAGEADDR(range, addr) \
   ((secstart[range]<<4)+(((addr)-(rangestart[range]))>>8))
#define ADDRINPAGE(range, addr) (((addr)-(rangestart[range]) & 0xfc)>>2)

int cchanflash::getrange(unsigned int addr) {
   unsigned int r;
   /* We need to scan the memory for the address o see if it is valid. */
   for(r = 0; r < rangestart.size(); r = r + 1) {
      /* The address ranges are in order, so if the current address is before
       * the current range, then there is no legal range.
       */
      if (addr < rangestart[r]) return -1;
      /* Now we check the ending address. If it is in this range, we return
       * the addres range.
       */
      if (addr <= rangeend[r]) return r;
   }
   /* And if we do not find it, we return -1. */
   return -1;
}

bool cchanflash::checkaddr(unsigned int addr) {
   /* If the range is valid, we return true. */
   return getrange(addr) >= 0;
}

bool cchanflash::preerase(unsigned int addr, unsigned int endaddr) {
   unsigned int i;
   unsigned int startsec, endsec;

   /* We need to find out what address range we are talking about. Only one
    * range can be erased at a time.
    */
   int r = getrange(addr);
   if (r < 0) {
      PRINTF_ERROR("SCFLASH", "Starting address %x is not in a valid range",
         addr);
      return false;
   }
   if (endaddr > rangeend[r]) {
      PRINTF_ERROR("SCFLASH", "Erasing must be done to one range at a time.");
      return false;
   }

   startsec = SECADDR(r, addr);
   endsec = SECADDR(r, endaddr);
   for(i = startsec; i < endsec; i = i + 1) secs[i] = ERS;

   return true;
}

bool cchanflash::preload(unsigned int addr, void* data, unsigned int size) {
   int r;
   unsigned int startsec, endsec, sec;

   /* First we make sure the starting and ending addresses are valid. */
   r = getrange(addr);
   if (r < 0) {
      PRINTF_ERROR("SCFLASH", "Access to illegal address %0x", addr);
      return false;
   }
   if (size + addr - 1> rangeend[r]) {
      PRINTF_ERROR("SCFLASH", "Access to illegal address %0x", addr + size -1);
      return false;
   }

   /* Now, we can deal with the data. We start by marking the set sectors.
    * For the preloading function we do not do all the checking if the secotr
    * is already programmed or erased. We just assume it is ok as this is for
    * preloading data for a test.
    */
   startsec = SECADDR(r, addr);
   endsec = SECADDR(r, addr + size - 1);
   for(sec = startsec; sec <= endsec; sec = sec + 1) {
      secs[sec] = PROG;
   }

   /* Now we program the data. Note that there could be something already
    * there. If there is we get rid of it.
    */
   unsigned int adinsec;
   unsigned int byteaddr;

   /* We do 64 words at a time. */
   unsigned int endaddr = addr - rangestart[r] + size - 1;
   unsigned int pagesize;
   unsigned int fromaddr = 0;
   unsigned int pageend;
   unsigned int writeend;

   /* We now scan one page at a time. */
   for(adinsec = addr - rangestart[r]; adinsec <= endaddr;
         adinsec = adinsec + 256) {

      /* Writes must be contained within a page. So first we need to find
       * out if this write will be within this page or not.
       */
      pageend = adinsec | 0x0ffU;

      /* Now we find out if this write will go all the way to the end of the
       * page or not.
       */
      if (endaddr >= pageend) writeend = pageend;
      else writeend = endaddr;
      
      /* We now set the size of the field. Anything beyond the string we chop
       * off. Therefore we will resize the field so that the page end position
       * fits.  Any new spaces we fill with blanks, 0xffffffff. */
      pagesize = (writeend % 256) + 1;
      pgm[PAGEADDRINSEC(r, adinsec)].resize(pagesize, 0xffffffff);

      /* And we copy in the data, one word at a time.  We cycle from the
       * current address in this page all the way till the end of this
       * page.
       */
      for (byteaddr = adinsec % 256; byteaddr < writeend % 256;
            byteaddr = byteaddr + 4) {
         pgm[PAGEADDRINSEC(r, adinsec)][byteaddr>>2]
            = ((unsigned int *)data)[fromaddr>>2];

         // Each time we transfered a word, we increment the from address.
         fromaddr = fromaddr + 4;
      }
   }

   return true;
}

void cchanflash::addrange(unsigned int _rangestart, unsigned int _rangeend) {
   int r;

   /* This function can only be called before the rangeinit(). */
   if (secs != NULL) {
      PRINTF_ERROR("SCFLASH", "addrange can only be called before rangeinit");
      return;
   }
   if (_rangeend <= _rangestart) {
      PRINTF_ERROR("SCFLASH", "range from %x to %x is invalid", _rangestart,
         _rangeend);
      return;
   }
   for (r = 0; r < (int)rangestart.size(); r = r + 1) {
      if (_rangestart >= rangestart[r] && _rangestart <= rangeend[r] ||
            _rangeend >= rangestart[r] && _rangeend <= rangeend[r] ||
            _rangestart < rangestart[r] && _rangeend > rangeend[r]) {
         PRINTF_ERROR("SCFLASH", "No range overlaps are allowed");
      }
   }

   /* Now that we know they are ok, we can insert them in order. */
   for(r = 0; r < (int)rangestart.size() && _rangestart > rangestart[r]; r=r+1);
   if (r == (int)rangestart.size()) {
      rangestart.push_back(_rangestart);
      rangeend.push_back(_rangeend);
   }
   else {
      rangestart.insert(rangestart.begin()+r, _rangestart);
      rangeend.insert(rangeend.begin()+r, _rangeend);
   }
}

void cchanflash::rangeinit() {
   int r, sec, seccnt, thissecsize;
   /* There must be at least one region declared to call this function. */
   if (rangestart.size() == 0 || rangeend.size() == 0) {
      PRINTF_ERROR("SCFLASH",
         "At least one range needs to be defined before calling rangeinit()");
      return;
   }
   else if (rangestart.size() != rangeend.size()) {
      PRINTF_ERROR("SCFLASH",
         "There should be the same number of range ends and range starts.");
      return;
   }

   /* We need to calculate the starting sector for each range and also the
    * total number of sectors.
    */
   seccnt = 0;
   thissecsize = 0;
   secstart.resize(rangestart.size());
   for(r = 0; r < (int)rangestart.size(); r = r + 1) {
      /* We set the current range to start just after the previous one
       * finished.
       */
      secstart[r] = seccnt;
      /* Now that we computed the sec starting position, we can calculate the
       * size of this range. We will need to do this to get the total size and
       * also the starting address of the next range.
       */
      thissecsize = (rangeend[r]+1-rangestart[r])/4096;
      seccnt = seccnt + thissecsize;
   }
   /* We can now create the space. There should be at least one sector. */
   if (seccnt == 0) {
      PRINTF_ERROR("SCFLASH", "There should at least one sector defined.");
      return;
   }

   /* We can now create the ranges and put them in place. */
   secs = new secstate_t[seccnt];
   if (secs == NULL) {
      PRINTF_FATAL("SCFLASH", "Could not allocate space for the sectors");
      return;
   }
   for (sec = 0; sec < seccnt; sec = sec + 1) secs[sec] = UNK;
   /* The data is set to all blank. */
   pgm = new std::vector<int>[seccnt*16];
   if (pgm == NULL) {
      PRINTF_FATAL("SCFLASH", "Could not allocate space for the memory model");
      return;
   }
}

void cchanflash::flash(void) {
   std::string msg("");
   unsigned int addr, size, pos;
   uint32_t data;
   char recv;
   int range;

   printf("Running Flash Thread\n");

   /* We check the secs pointer. If it is null, the memory has not been
    * initialized.
    */
   if (secs == NULL) {
      PRINTF_ERROR("SCFLASH",
      "The CCHAN flash needs valid ranges added and initialized before using.");
   }

   while(1) {
      msg = "";
      do {
         recv = i_uflash.from.read();
         msg += recv;
      } while(recv != '\n');

      printf("Flash got message %s", msg.c_str());

      /* ERASE */
      if (msg.find("e:") == 0) {
         /* We parse the message */
         sscanf(msg.c_str(), "e:%x", &addr);
         addr = addr * 4096; /* convert sector to address. */
         /* We forbid erasures to dangerous locations. */
         range = getrange(addr);
         if (range < 0) {
            PRINTF_ERROR("SCFLASH", "Access to illegal address %0x", addr);
            i_uflash.to.write('\1');
         }
         else {
            /* And we do a delay for the erasure. */
            wait(20, SC_MS);
            /* Assuming it worked, we label the sector as erased. */
            secs[SECADDR(range,addr)] = ERS;
            int pgstart = PAGEADDR(range,addr);
            int pgend = pgstart+15;
            int pg;
            for (pg = pgstart; pg <= pgend; pg = pg + 1) pgm[pg].resize(0);
            /* And we return success. */
            i_uflash.to.write('\0');
         }
      }
      /* PROGRAM */
      else if (msg.find("w:") == 0) {
         /* We parse the message */
         sscanf(msg.c_str(), "w:%x %u", &addr, &size);
         /* We forbid erasures to dangerous locations. */
         range = getrange(addr);
         if (range < 0) {
            PRINTF_ERROR("SCFLASH", "Access to illegal address %0x", addr);
            i_uflash.to.write('\1');
         }
         /* Writing to a never used sector can be bad. */
         else if (secs[SECADDR(range, addr)] == UNK)
         {
            PRINTF_ERROR("SCFLASH", "Access to unititialized address %0x",
               addr);
            i_uflash.to.write('\1');
         }
         /* We do not support size 0 or large messages. */
         else if (size < 1 || size > 64) {
            PRINTF_ERROR("SCFLASH", "Illegal Size %d", size);
            i_uflash.to.write('\1');
         }
         else if (((addr + size*4 - 1) & 0xffffff00) != (addr & 0xffffff00)) {
            PRINTF_ERROR("SCFLASH",
               "Write crossing 256byte page boundary: start = %x end = %x",
               addr, addr + size*4 - 1);
            i_uflash.to.write('\1');
         }
         /* Command is valid. */
         else {
            /* It should begin with "d:" */
            if (i_uflash.from.read() != 'd' || i_uflash.from.read() != ':') {
               while(i_uflash.from.read() != '\n') ;
               SC_REPORT_ERROR("SCFLASH", "Data message expected!");
               i_uflash.to.write('\1');
            }
            else {
               /* The request is valid, so we store it.  We simulate
                * the write time.
                */
               wait(20, SC_US);
               /* We define the page is programmed. */
               secs[SECADDR(range, addr)] = PROG;
               /* If the current position is too small to store the new page,
                * we extend it. For this we get the address within the page
                * and add the size. We always start a record from the address
                * zero.
                */
               int pageaddr = PAGEADDR(range, addr);
               int addrinpage = ADDRINPAGE(range, addr);
               if (addrinpage + size > pgm[pageaddr].size())
                  pgm[pageaddr].resize(addrinpage + size);

               /* Now we store the data sent. */
               for(pos = 0; pos < size; pos = pos + 1) {
                  data = (0x000000ffU & i_uflash.from.read());
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 8);
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 16);
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 24);
                  pgm[pageaddr][pos+addrinpage] = data;
               }
               while(i_uflash.from.read() != '\n') ;
               i_uflash.to.write('\0');
            }
         }
      }
      /* READ */
      else {
         /* We parse the message */
         sscanf(msg.c_str(), "r:%x %u", &addr, &size);
         /* We forbid accesses to dangerous locations. */
         range = getrange(addr);
         if (range < 0) {
            PRINTF_ERROR("SCFLASH", "Access to illegal address %0x", addr);
            i_uflash.to.write('\1');
         }
         /* We do not ban reads to unused locations as the firmware does
          * this to test the flash. We instead return different results.
          */
         else
         {
            int secaddr = SECADDR(range, addr);
            int pageaddr = PAGEADDR(range, addr);
            wait(200, SC_NS);
            printf("Got Flash Read %x %u @%s\n", addr, size,
               sc_time_stamp().to_string().c_str());

            /* Unknown: return random data. */
            if (secs[secaddr] == UNK) {
               for(pos = 0; pos < size; pos = pos + 1) {
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
               }
            }
            /* Erased: return FFFF. */
            else if (secs[secaddr] == ERS || pgm[pageaddr].size() == 0) {
               for(pos = 0; pos < size; pos = pos + 1) {
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
               }
            }
            /* Programmed: return the data programmed. */
            else {
               pos = (addr>>2) & 0x3f; /* offset addr */
               unsigned int end = size + pos - 1;
               unsigned int endsto = pgm[pageaddr].size()-1;

               /* If the starting pos is before the size (end of the
                * message) we put as much as we can.
                */
               for(; pos <= ((end>endsto)?endsto:end); pos = pos + 1) {
                  data = pgm[pageaddr][pos];
                     i_uflash.to.write((char)(data & 0xff));
                     i_uflash.to.write((char)((data>>8) & 0xff));
                     i_uflash.to.write((char)((data>>16) & 0xff));
                     i_uflash.to.write((char)((data>>24) & 0xff));
               }
               /* If we have not given all the data, we fill the rest with
                * ff
                */
               for(; pos <= end; pos = pos + 1) {
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
                  i_uflash.to.write((char)(0xff));
               }
            }
            i_uflash.to.write('\0'); /* Return success */
         }
      }
   }
}
