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

bool cchanflash::checkaddr(unsigned int addr) {
   if (addr < userflashstart || addr > userflashend) return false;
   return true;
}

bool cchanflash::preerase(unsigned int addr, unsigned int endaddr) {
   char buffer[80];
   unsigned int i;

   if (!checkaddr(addr)) {
      sprintf(buffer, "Access to illegal address %0x", addr);
      SC_REPORT_ERROR("SCFLASH", buffer);
      return false;
   }
   if (!checkaddr(endaddr)) {
      sprintf(buffer, "Access to illegal address %0x", addr);
      SC_REPORT_ERROR("SCFLASH", buffer);
      return false;
   }

   for(i = (addr-userflashstart) >> 12;
         i < ((endaddr-userflashstart)>>12); i = i + 1) {
      secs[i] = ERS;
   }

   return true;
}

bool cchanflash::preload(unsigned int addr, void* data, unsigned int size) {
   char buffer[64];
   unsigned int startsec, endsec, sec;

   /* First we make sure the starting and ending addresses are valid. */
   if (!checkaddr(addr)) {
      sprintf(buffer, "Access to illegal address %0x", addr);
      SC_REPORT_ERROR("SCFLASH", buffer);
      return false;
   }
   if (!checkaddr(addr+size-1)) {
      sprintf(buffer, "Access to illegal address %0x", addr);
      SC_REPORT_ERROR("SCFLASH", buffer);
      return false;
   }

   /* Now, we can deal with the data. We start by marking the set sectors.
    * For the preloading function we do not do all the checking if the secotr
    * is already programmed or erased. We just assume it is ok as this is for
    * preloading data for a test.
    */
   startsec = (addr-userflashstart) >> 12;
   endsec = (addr+size-1-userflashstart) >> 12;
   for(sec = startsec; sec <= endsec; sec = sec + 1) {
      secs[sec] = PROG;
   }

   /* Now we program the data. Note that there could be something already
    * there. If there is we get rid of it.
    */
   unsigned int currentaddr;
   unsigned int byteaddr;

   /* We do 64 words at a time. */
   unsigned int endaddr = addr - userflashstart + size - 1;
   unsigned int pagesize;
   unsigned int fromaddr = 0;
   unsigned int pageend;
   unsigned int writeend;

   /* We now scan one page at a time. */
   for(currentaddr = addr - userflashstart; currentaddr <= endaddr;
         currentaddr = currentaddr + 256) {

      /* Writes must be contained within a page. So first we need to find
       * out if this write will be within this page or not.
       */
      pageend = currentaddr | 0x0ffU;

      /* Now we find out if this write will go all the way to the end of the
       * page or not.
       */
      if (endaddr >= pageend) writeend = pageend;
      else writeend = endaddr;
      
      /* We now set the size of the field. Anything beyond the string we chop
       * off. Therefore we will resize the field so that the page end position
       * fits.  Any new spaces we fill with blanks, 0xffffffff. */
      pagesize = (writeend % 256) + 1;
      pgm[currentaddr >> 8].resize(pagesize, 0xffffffff);

      /* And we copy in the data, one word at a time.  We cycle from the
       * current address in this page all the way till the end of this
       * page.
       */
      for (byteaddr = currentaddr % 256; byteaddr < writeend % 256;
            byteaddr = byteaddr + 4) {
         pgm[currentaddr >> 8][byteaddr>>2]
            = ((unsigned int *)data)[fromaddr>>2];

         // Each time we transfered a word, we increment the from address.
         fromaddr = fromaddr + 4;
      }
   }

   return true;
}

std::vector<int> *pgm; /* Datastr. */

void cchanflash::flash(void) {
   std::string msg("");
   unsigned int addr, size, pos;
   uint32_t data;
   char recv;
   char buffer[80];

   printf("Running Flash Thread\n");

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
         if (!checkaddr(addr)) {
            sprintf(buffer, "Access to illegal address %0x", addr);
            SC_REPORT_ERROR("SCFLASH", buffer);
            i_uflash.to.write('\1');
         }
         else {
            /* And we do a delay for the erasure. */
            wait(20, SC_MS);
            /* Assuming it worked, we label the sector as erased. */
            secs[(addr - userflashstart)>>12] = ERS;
            int pg;
            for (pg = 0; pg < 16; pg = pg + 1) {
               pgm[((addr - userflashstart)>>8)+pg].resize(0);
            }
            /* And we return success. */
            i_uflash.to.write('\0');
         }
      }
      /* PROGRAM */
      else if (msg.find("w:") == 0) {
         /* We parse the message */
         sscanf(msg.c_str(), "w:%x %u", &addr, &size);
         /* We forbid erasures to dangerous locations. */
         if (!checkaddr(addr)) {
            sprintf(buffer, "Access to illegal address %0x", addr);
            SC_REPORT_ERROR("SCFLASH", buffer);
            i_uflash.to.write('\1');
         }
         /* Writing to a never used sector can be bad. */
         else if (secs[(addr - userflashstart)>>12] == UNK)
         {
            char buffer[80];
            sprintf(buffer, "Access to illegal address %0x", addr);
            SC_REPORT_ERROR("SCFLASH", buffer);
            i_uflash.to.write('\1');
         }
         /* We do not support size 0 or large messages. */
         else if (size < 1 || size > 64) {
            char buffer[80];
            sprintf(buffer, "Illegal Size %d", size);
            SC_REPORT_ERROR("SCFLASH", buffer);
            i_uflash.to.write('\1');
         }
         else if (((addr + size*4 - 1) & 0xffffff00) != (addr & 0xffffff00)) {
            char buffer[80];
            sprintf(buffer,
               "Write crossing 256byte page boundary: start = %x end = %x",
               addr, addr + size*4 - 1);
            SC_REPORT_ERROR("SCFLASH", buffer);
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
               secs[(addr - userflashstart)>>12] = PROG;
               /* If the current position is too small to store the new page,
                * we extend it. For this we get the address within the page
                * and add the size. We always start a record from the address
                * zero.
                */
               int addrinpage = ((addr - userflashstart) & 0xfc) >> 2;
               if (addrinpage + size > pgm[(addr - userflashstart)>>8].size())
                  pgm[(addr - userflashstart)>>8].resize(addrinpage + size);

               /* Now we store the data sent. */
               for(pos = 0; pos < size; pos = pos + 1) {
                  data = (0x000000ffU & i_uflash.from.read());
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 8);
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 16);
                  data = data | ((0x000000ffU & i_uflash.from.read()) << 24);
                  pgm[(addr - userflashstart)>>8][pos+addrinpage] = data;
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
         if (!checkaddr(addr)) {
            sprintf(buffer, "Access to illegal address %0x", addr);
            SC_REPORT_ERROR("SCFLASH", buffer);
            i_uflash.to.write('\1');
         }
         /* We do not ban reads to unused locations as the firmware does
          * this to test the flash. We instead return different results.
          */
         else
         {
            wait(200, SC_NS);
            printf("Got Flash Read %x %u @%s\n", addr, size,
               sc_time_stamp().to_string().c_str());

            /* Unknown: return random data. */
            if (secs[(addr - userflashstart)>>12] == UNK) {
               for(pos = 0; pos < size; pos = pos + 1) {
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
                  i_uflash.to.write((char)rand());
               }
            }
            /* Erased: return FFFF. */
            else if (secs[(addr - userflashstart)>>12] == ERS ||
                  pgm[(addr - userflashstart)>>8].size() == 0) {
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
               unsigned int endsto
                  = pgm[(addr - userflashstart)>>8].size()-1;

               /* If the starting pos is before the size (end of the
                * message) we put as much as we can.
                */
               for(; pos <= ((end>endsto)?endsto:end); pos = pos + 1) {
                  data = pgm[(addr - userflashstart)>>8][pos];
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
