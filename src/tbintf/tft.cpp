/*******************************************************************************
 * tft.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a test bench interface to check the results of the TFT eSPÌ
 *   library.
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
#include "tft.h"

sc_event sigdone_ev;
unsigned int getlv(int bit, sc_logic v, bool zok = true);

enum {NONE,
   ID0, ID1, ID2, ID3,
   DS0, DS1, DS2, DS3, DS4,
   PM0, PM1,
   AM0, AM1,
   PX0, PX1,
   DF0, DF1,
   SM0, SM1,
   SD0, SD1
} rdstate = NONE;

/*******************************************************************************
** TFT Thread ******************************************************************
*******************************************************************************/

unsigned int getlv(int bit, sc_logic v, bool zok) {
   if (v == SC_LOGIC_X) {
      SC_REPORT_ERROR("TB", "Tries to bring X into 2level logic");
      return 0;
   }
   else if(v == SC_LOGIC_Z && !zok) {
      SC_REPORT_ERROR("TB", "Tries to bring Z into 2level logic");
      v = SC_LOGIC_0;
      return 1 << bit;
   }
   else if (v == SC_LOGIC_1 || v == SC_LOGIC_Z) {
      return 1 << bit;
   }
   else return 0;
}

void tftmod::drive(unsigned int val) {
   d0.write(((val & 0x01)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d1.write(((val & 0x02)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d2.write(((val & 0x04)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d3.write(((val & 0x08)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d4.write(((val & 0x10)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d5.write(((val & 0x20)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d6.write(((val & 0x40)>0)?GN_LOGIC_Z:GN_LOGIC_0);
   d7.write(((val & 0x80)>0)?GN_LOGIC_Z:GN_LOGIC_0);
}

void tftmod::read(void) {
   while(1) {
      wait();

      /* High-Z and logic 1 we treat the same way as there are only
       * pulldowns. Therefore, if we see a Z it means that the model did
       * not put the pull-up.
       */
      if (rd.read() == SC_LOGIC_X) {
         SC_REPORT_ERROR("TB", "DC went X");
      }
      /* We drive on the falling edge and remove just after the falling edge. */
      else if (rd.read() != SC_LOGIC_0) {
         wait(10, SC_NS);
         d0.write(GN_LOGIC_Z);
         d1.write(GN_LOGIC_Z);
         d2.write(GN_LOGIC_Z);
         d3.write(GN_LOGIC_Z);
         d4.write(GN_LOGIC_Z);
         d5.write(GN_LOGIC_Z);
         d6.write(GN_LOGIC_Z);
         d7.write(GN_LOGIC_Z);
      }
      else {
         switch(rdstate) {
            case ID0: drive(0); rdstate = ID1; break;
            case ID1: drive(0x55); rdstate = ID2; break;
            case ID2: drive(0xaa); rdstate = ID3; break;
            case ID3: drive(0x55); rdstate = NONE; break;
            case DS0: drive(0); rdstate = DS1; break;
            case DS1: drive(0x0); rdstate = DS2; break;
            case DS2: drive(0x61); rdstate = DS3; break;
            case DS3: drive(0x0); rdstate = DS4; break;
            case DS4: drive(0x0); rdstate = NONE; break;
            case PM0: drive(0); rdstate = PM1; break;
            case PM1: drive(0x8); rdstate = NONE; break;
            case AM0: drive(0); rdstate = AM1; break;
            case AM1: drive(0x8); rdstate = NONE; break;
            case PX0: drive(0); rdstate = PX1; break;
            case PX1: drive(0x8); rdstate = NONE; break;
            case DF0: drive(0); rdstate = DF1; break;
            case DF1: drive(0x8); rdstate = NONE; break;
            case SM0: drive(0); rdstate = SM1; break;
            case SM1: drive(0x8); rdstate = NONE; break;
            case SD0: drive(0); rdstate = SD1; break;
            case SD1: drive(0x0); rdstate = NONE; break;
            default: drive(0); break;
         }
      }
   }
}

void tftmod::write(void) {
   unsigned int msg;
   uint32_t val, val1;
   int cmd;
   sc_uint<8> t;
   enum {IDLE, WRSKIP, WRE0, WR0, WR1, WR2, WR3, WRN, SLEEP} state = IDLE;
   SC_REPORT_INFO("I2C", "Starting TFT Thread");
   sc_time sleepchangetime;

   /* we begin in sleep. */
   sleepchangetime = sc_time(0, SC_NS);


   while(1) {
      wait();

      /* High-Z and logic 1 we treat the same way as there are only
       * pulldowns. Therefore, if we see a Z it means that the model did
       * not put the pull-up.
       */
      if (wr.read() == SC_LOGIC_0) continue;
      /* X, now is bad. */
      if (wr.read() == SC_LOGIC_X) {
         SC_REPORT_ERROR("TB", "DC went X");
         continue;
      }

      /* We sample the bits 7 to 0 */
      msg = getlv(0, d0.read().logic);
      msg |= getlv(1, d1.read().logic);
      msg |= getlv(2, d2.read().logic);
      msg |= getlv(3, d3.read().logic);
      msg |= getlv(4, d4.read().logic);
      msg |= getlv(5, d5.read().logic);
      msg |= getlv(6, d6.read().logic);
      msg |= getlv(7, d7.read().logic);

      /* If we see DC low and RD high, this is a new command. We then look at
       * the msg field (d7:d0) to see what command was given.
       */
      if (dc.read() == SC_LOGIC_0 && rd.read() == SC_LOGIC_1) {
         /* If we get a command just after entering or exiting sleep, we need to
          * raise a flag.
          */
         if (sleepchangetime - sc_time_stamp() < sc_time(5, SC_MS)) {
            SC_REPORT_WARNING("TFT",
               "Command sent less than 5ms after sleep start or stop.");
         }

         cmd = msg;
         val = 0;
         /* We check the command to see if it is valid. */
         switch(cmd) {
            case 0x0:
               printf("%s: %s: NOP\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               break;
            case 0x1:
               printf("%s: %s: Software Reset\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               break;
            case 0xa:
               printf("%s: %s: Power Mode Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = PM0;
               break;
            case 0xb:
               printf("%s: %s: Address Mode Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = AM0;
               break;
            case 0xc:
               printf("%s: %s: Pixel Format Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = PX0;
               break;
            case 0xd:
               printf("%s: %s: Display Mode Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = DF0;
               break;
            case 0xe:
               printf("%s: %s: Signal Mode Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = SM0;
               break;
            case 0xf:
               printf("%s: %s: Self Diagnostics Read\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = SD0;
               break;
            case 0x10:
               /* If we are already in sleep nothing happens. */
               if (sleep.read()) {
                  printf("%s: %s: Sleep request but already in Sleep Mode\n",
                     name(), sc_time_stamp().to_string().c_str());
                  break;
               }
               /* If we are not, we raise the sleep state. This in itself does
                * not mean anything. Just means the screen will be off.
                */
               printf("%s: %s: Entering Sleep Mode\n",
                  name(), sc_time_stamp().to_string().c_str());
               if (sleepchangetime - sc_time_stamp() < sc_time(120, SC_MS))
                  SC_REPORT_WARNING("TFT", "Exiting Sleep mode too quick.");
               rdstate = NONE;
               sleep.write(1);
               sleepchangetime = sc_time_stamp();
               break;
            case 0x11:
               /* If we are not in Sleep mode, then we do nothing. */
               if (sleep.read() == false) {
                  printf("%s: %s: Sleep request but already in Sleep Mode\n",
                     name(), sc_time_stamp().to_string().c_str());
                  break;
               }
               /* If we got the exit sleep command, we take it. If it came in
                * too quick, we want but keep on processing as nothing happened
                * as we cannot tell the real impact of this.
                */
               printf("%s: %s: Exit Sleep Mode\n",
                  name(), sc_time_stamp().to_string().c_str());
               if (sleepchangetime - sc_time_stamp() < sc_time(120, SC_MS))
                  SC_REPORT_WARNING("TFT", "Exiting Sleep mode too quick.");
               state = IDLE;
               rdstate = NONE;
               sleep.write(0);
               sleepchangetime = sc_time_stamp();
               break;
            case 0x12:
               printf("%s: %s: Entering Partial ON\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x13:
               printf("%s: %s: Entering Normal Mode\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x20:
               printf("%s: %s: Display Inversion Off\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x21:
               printf("%s: %s: Display Inversion On\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x28:
               printf("%s: %s: Display Off\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x29:
               printf("%s: %s: Display On\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = IDLE;
               rdstate = NONE;
               break;
            case 0x2c:
               printf("%s: %s: Receiving Data\n",
                  name(), sc_time_stamp().to_string().c_str());
               signature = 0;
               explength = (endcol-startcol+1) * (endrow-startrow+1) * 2;
               llen = 0;
               state = WRN;
               rdstate = NONE;
               break;
            case 0xc8:
               printf("%s: %s: Setting Gama\n",
                  name(), sc_time_stamp().to_string().c_str());
               state = WRSKIP;
               rdstate = NONE;
               break;
            /* Generic Write functions */
            /* 5 parameter funcs */
            case 0xc0:
               state = WRE0;
               rdstate = NONE;
               break;
            /* 4 parameter funcs */
            case 0x2a:
            case 0x2b:
            case 0x30:
               state = WR0;
               rdstate = NONE;
               break;
            /* 3 parameter funcs */
            case 0xd0:
            case 0xd1:
               state = WR1;
               rdstate = NONE;
               break;
            /* 2 parameter funcs */
            case 0xd2:
               state = WR2;
               rdstate = NONE;
               break;
            /* 1 parameter funcs */
            case 0xc5:
            case 0x36:
            case 0x3a:
               state = WR3;
               rdstate = NONE;
               break;
         }
      }
      else switch(state) {
         case WRN:
            /* If the image is taller than a font, it has to be an image.
             * Not a good way to do this, but it works for now.
             */
            if (endcol - startcol > 30)
               signature.write(do1crc((uint32_t)signature.read(), msg));
            else if (msg == fglow || msg == fghigh)
               signature.write(do1crc((uint32_t)signature.read(), 0xff));
            else signature.write(do1crc((uint32_t)signature.read(), 0x00));
            explength = explength.read() - 1;
            llen = llen.read() + 1;
            if (explength.read() == 1) {
               sigdone_ev.notify();
               state = IDLE;
            }
            break;
         case WRSKIP: break;
         case WRE0: val1 = msg; state = WR0; break;
         case WR0: val = msg << 24; state = WR1; break;
         case WR1: val = val | (msg << 16); state = WR2; break;
         case WR2: val = val | (msg << 8); state = WR3; break;
         case WR3:
            val = val | msg;
            state = IDLE;
            if (cmd == 0x2a) {
               startcol = val>>16;
               endcol = (val&0xffff);
               printf("%s: %s: Setting column to = %04x:%04x\n",
                  name(), sc_time_stamp().to_string().c_str(),
                  val>>16, val & 0xffff);
            }
            else if (cmd == 0x2b) {
               startrow = val>>16;
               endrow = (val&0xffff);
               printf("%s: %s: Setting row to = %04x:%04x\n",
                  name(), sc_time_stamp().to_string().c_str(),
                  val>>16, val & 0xffff);
            }
            else if (cmd == 0x30) 
               printf("%s: %s: Set Partial Row Start = %08x End = %08x\n",
                  name(), sc_time_stamp().to_string().c_str(),
                  (val >> 16) & 0x1ff, val & 0x1ff);
            else if (cmd == 0x36) 
               printf("%s: %s: Memory Access Control %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else if (cmd == 0x3a) 
               printf("%s: %s: Pixel Format = %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else if (cmd == 0xc5) 
               printf("%s: %s: Frame Rate and Inversion %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else if (cmd == 0xc0) 
               printf("%s: %s: Pannel Driving %02x%08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val1, val);
            else if (cmd == 0xd0) 
               printf("%s: %s: Set Power %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else if (cmd == 0xd1) 
               printf("%s: %s: VCOM %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else if (cmd == 0xd2) 
               printf("%s: %s: Normal Power %08x\n",
                  name(), sc_time_stamp().to_string().c_str(), val);
            else
               printf("G: %s: %x: %08x\n", sc_time_stamp().to_string().c_str(),
                  cmd, val);
            break;
         default: state = IDLE;
            printf("D: %s: %02x\n", sc_time_stamp().to_string().c_str(),
               msg);
      }

      istate = state;
      icmd = cmd;
   }
}

void tftmod::waitcode(tft_obj_t *t) {
   /* We wait for the notification to arrive. Being signature is a SC_SIGNAL,
    * we need to wait a delta cycle so we can sample it.
    */
   wait(sigdone_ev);
   wait(SC_ZERO_TIME);
   t->x1 = startcol;
   t->y1 = startrow;
   t->x2 = endcol;
   t->y2 = endrow;
   t->signature = signature;
}

void tftmod::setfgcolor(unsigned int v) {
   fglow = v & 0xff;
   fghigh = (v>>8) & 0xff;
}

void tftmod::trace(sc_trace_file *tf) {
   sc_trace(tf, istate, istate.name());
   sc_trace(tf, icmd, icmd.name());
   sc_trace(tf, startcol, startcol.name());
   sc_trace(tf, endcol, endcol.name());
   sc_trace(tf, startrow, startrow.name());
   sc_trace(tf, endrow, endrow.name());
   sc_trace(tf, signature, signature.name());
   sc_trace(tf, explength, explength.name());
   sc_trace(tf, llen, llen.name());
   sc_trace(tf, sleep, sleep.name());
}
