/*******************************************************************************
 * pn532.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a crude model for the PN532 with firmware v1.6. It will work for
 *   basic work with a PN532 system.
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
#include "pn532.h"
#include "info.h"

const unsigned char PN532_ID = 0x24;

/*******************************************************************************
** I2C Thread ******************************************************************
*******************************************************************************/

void pn532::pushack() {
   to.write(0x00);
   to.write(0x00);
   to.write(0xFF);
   to.write(0x00);
   to.write(0xFF);
   to.write(0x00);
}

void pn532::pushpreamble(int len, bool hosttopn, int cmd, unsigned char *c) {
   to.write(0x00);
   to.write(0x00);
   to.write(0xFF);
   to.write(len);
   to.write(0x100-len);
   /* Now we clear the checksum as we begin with the TFI. */
   *c = 0;
   pushandcalc((hosttopn)?0xd4:0xd5, c);
   pushandcalc(cmd, c);
}

void pn532::setcardnotpresent() {
   mif.tags = 0;
   mif.sens_res = 0x0000;
   mif.sel_res = 0;
   mif.uidLength = 0;
   mif.uidValue = 0x0000;
}

void pn532::setcardpresent() {
   mif.tags = 1;
   mif.sens_res = 0x5522;
   mif.sel_res = 0;
   mif.uidLength = 4;
   mif.uidValue = 0x75D17E20;
}

void pn532::start_of_simulation() {
   /* We begin setting the card as non-present. */
   setcardnotpresent();
}

void pn532::i2c_th(void) {
   int i;
   unsigned int devid, data;
   bool invalid;
   enum {IDLE, DEVID, ACKWR, ACKRD, RETZ, ACKWRD, WRDATA, READ, ACKNACK}
      i2cstate;

   SC_REPORT_INFO("I2C", "Starting I2C Thread");

   /* Then we begin waiting for the I2C commands. */
   while(1) {
      wait(sda.default_event() | scl.default_event());

      /* If we get Z or X, we ignore it. */
      if (!scl.read().islogic()) {
         PRINTF_WARN("PN532", "SCL is not defined");
      }
      else if (!sda.read().islogic()) {
         PRINTF_WARN("PN532", "SDA is not defined");
      }
      /* At any time a start or stop bit can come in. */
      else if (scl.read().ishigh() && sda.value_changed_event().triggered()) {
         if (sda.read().islow()) {
            /* START BIT */
            i2cstate = DEVID;
            i = 7;
            devid = 0;
         }
            /* STOP BIT */
         else i2cstate = IDLE;
      }
      /* The rest has to be data changes. For simplicity all data in and flow
       * control is here. The data returned is done on another branch.
       */
      else if (scl.value_changed_event().triggered() && scl.read().ishigh())
            switch(i2cstate) {
         case IDLE: break;
         case DEVID:
            /* We collect all bits of the device ID. The last bit is the R/W
             * bit.
             */
            if (i > 0) devid = (devid << 1) + ((sda.read().ishigh())?1:0);
            /* If the ID matches, we can process it. */
            else if (devid == PN532_ID && sda.read().ishigh()) i2cstate = ACKRD;
            else if (devid == PN532_ID && sda.read().islow()) i2cstate = ACKWR;
            /* If the devid does not match, we return Z. */
            else i2cstate = RETZ;

            i = i - 1;
            if (i == 0 && devid == PN532_ID) {
               /* Once we reach zero we release the IRQ line. We also record if
                * the block is invalid (read during IRQ high) or valid (read
                * during IRQ low.
                */
               invalid = !gethasdata();
               ackirq_ev.notify();
            }
            break;
         /* If the address does not match, we return Z. */
         case RETZ:
            i2cstate = IDLE;
            break;
         /* Once a code has been recognized as a write, so we return an ACK.
          * so that the master knows it was accepted. He can start sending
          * the data. We just store it in a buffer and let the FSM handle it.
          */
         case ACKRD:
         case ACKWR:
            i = 7;
            /* If it is a Read ack, we start loading the data to return. */
            if (i2cstate == ACKRD) {
               /* If it is busy we return 00, meaning, not ready, and after
                * that we will get only garbage. If there is data,
                * we get a 01 and then we will get valid data.
                */
               if (invalid) data = 0x00;
               else data = 0x01;
               i2cstate = READ;
            }
            /* If this is a ACKWR, we then clear the buffer and collect the
             * data.
             */
            else {
               data = 0;
               i2cstate = WRDATA;
            }
            break;
         /* Each bit is taken. */
         case WRDATA:
            data = (data << 1) + ((sda.read().ishigh())?1:0);
            if (i == 0) i2cstate = ACKWRD;
            else i = i - 1;
            break;
         /* Once a byte is in, we store it in the buffer and prepare for more.*/
         case ACKWRD:
            from.write(data);
            data = 0;
            i = 7;
            i2cstate = WRDATA;
            break;
         /* Depending on the acknack we either return more data or not. */
         case ACKNACK:
            if (sda.read().ishigh()) i2cstate = IDLE;
            else {
               i2cstate = READ;
               i = 7;
               if (to.num_available() == 0) data = 0;
               else data = put();
            }
            break;
         /* On reads, we send the data after each bit change. Note: the driving
          * is done on the other edge.
          */
         case READ:
            if (i == 0) i2cstate = ACKNACK;
            else i = i - 1;
            break;
      }
      /* Anytime the clock drops, we return data. We only do the data return
       * to keep the FSM simpler.
       */
      else if (scl.value_changed_event().triggered() && scl.read().islow())
            switch (i2cstate) {
         /* If we get an illegal code, we return Z. We remain here until
          * we get.
          */
         case RETZ: sda.write(GN_LOGIC_Z); break;
         /* ACKWRD, ACKWR and ACKRD we return a 0. */
         case ACKRD:
         case ACKWR:
         case ACKWRD:
            sda.write(GN_LOGIC_0);
            break;
         /* For the WRITE state, all we do is release the SDA so that the
          * master can write. */
         case WRDATA:
         case ACKNACK:
            sda.write(GN_LOGIC_Z);
            break;
         /* Each bit is returned. */
         case READ:
            if (invalid) sda.write(GN_LOGIC_0);
            else if ((data & (1 << i))>0) sda.write(GN_LOGIC_Z);
            else sda.write(GN_LOGIC_0);
            break;
         default: ; /* For the others we do nothing. */
      }

      icstate.write(i2cstate);
   }
}

void pn532::pushresp() {
   unsigned char cksum = 0;
   if (mif.cmd == 0x4a) {
      /* And we wait some short processing time before placing the
       * packet to return in the buffer.
       */
      wait(60, SC_MS);
      sethasdata();

      /* Preamble */
      pushpreamble(0xA, false, 0x4A, &cksum);
      /* Packet */
      pushandcalc(mif.tags, &cksum);
      pushandcalc(0x00, &cksum); /* Tag no. */
      pushandcalc(mif.sens_res>>8, &cksum);/* sens res upper */
      pushandcalc(mif.sens_res&0xff, &cksum);/* sens res lower */
      pushandcalc(mif.sel_res, &cksum); /* sel res */
      pushandcalc(mif.uidLength, &cksum); /* NFCID Len */
      /* NFCID */
      pushandcalc(mif.uidValue>>24, &cksum);
      pushandcalc((mif.uidValue&0xff0000)>>16, &cksum);
      pushandcalc((mif.uidValue&0xff00)>>8, &cksum);
      pushandcalc(mif.uidValue&0xff, &cksum);
      to.write(0x100-cksum); /* DCS */
      to.write(0x00); /* ZERO */
   }
   else if (mif.cmd == 0x14) {
      wait(1, SC_MS);
      sethasdata();
      pushpreamble(0x5, false, 0x15, &cksum);
      to.write(0x100-cksum); /* DCS */
      to.write(0x00); /* ZERO */
   }
   else if (mif.cmd == 0x02) {
      wait(1, SC_MS);
      sethasdata();
      pushpreamble(0x06, false, 0x3, &cksum);
      /* Firmware Version, we just pick something cool from
       * one of the examples.
       */
      pushandcalc(0x32, &cksum); /* IC */
      pushandcalc(0x01, &cksum); /* Firmware Version */
      pushandcalc(0x06, &cksum); /* Revision */
      pushandcalc(0x07, &cksum); /* Support */
      to.write(0x100-cksum); /* DCS */
      to.write(0x00); /* ZERO */
   }

   /* The last command was executed, so we blank it out. */
   mif.cmd = 0x0;
}

void pn532::process_th() {
   unsigned char cksum = 0;
   unsigned char msg;
   enum {IDLE, UNLOCK1, UNLOCK2, UNLOCK3, UNLOCK4, UNLOCK5, PD0, PDN, DCS,
      LOCK} pn532state;
   clrhasdata();

   while(true) {
      if (from.num_available() == 0) {
         wait(ackirq_ev | from.data_written_event());
         /* When the ID is recognized, the IRQ goes high, regardless of what he
          * actually does.
          */
         if (ackirq_ev.triggered()) {
            clrhasdata();
            /* Depending on the command, we might have also a response.*/
            pushresp();
            continue;
         }
      }

      msg = grab(&cksum);
      switch(pn532state) {
         case IDLE:
            if (msg == 0x0) pn532state = UNLOCK1;
            else pn532state = IDLE;
            break;
         case UNLOCK1:
            if (msg == 0x0) pn532state = UNLOCK2;
            else pn532state = IDLE;
            break;
         case UNLOCK2:
            /* The first 0x0 can be as long as the customer wants, so we
             * discard any extra 0x0 received.
             */
            if (msg == 0x0) pn532state = UNLOCK2;
            else if (msg == 0xff) pn532state = UNLOCK3;
            else pn532state = IDLE;
            break;
         case UNLOCK3:
            /* We collect the length */
            mif.len = msg;
            pn532state = UNLOCK4;
            break;
         case UNLOCK4:
            /* Anytime we go to the UNLOCK5 (TFI state) we clear the cksum. */
            cksum = 0;
            /* Now we check the LCS, it should be the complement of the LEN. */
            if (0x100 - mif.len != msg) {
               PRINTF_WARN("PN532", "Warning: got illegal LCS.");
            }
            pn532state = UNLOCK5;
            break;
         case UNLOCK5: {
            unsigned char tfi = msg;
            if (tfi != 0xD4) {
               PRINTF_WARN("PN532", "Warning: got illegal TFI %02x", tfi);
               pn532state = IDLE;
            }
            mif.len = mif.len - 1;
            pn532state = PD0;
            break;
         }
         case PD0:
            if (pn532state == PD0) mif.cmd = msg;
            mif.len = mif.len - 1;
            if (mif.len == 0) pn532state = DCS;
            else pn532state = PDN;
            /* We dump any previous command in the fifo. */
            while(to.num_available() != 0) { cksum = cksum + to.read(); }
            break;
         case PDN:
            /* Some packages have a packet. If it has one we take it in to the
             * right places. Any remaining bytes or packets in unsupported
             * messages are pitched.
             */
            mif.len = mif.len - 1;
            if (mif.cmd == 0x14) {
               mif.mode = msg;
               if (mif.len>0) { mif.timeout=grab(&cksum); mif.len = mif.len-1;}
               if (mif.len>0) { mif.useirq=grab(&cksum); mif.len = mif.len-1;}
            }
            if (mif.len == 0) pn532state = DCS;
            break;
         case DCS: {
            /* We now check the DCS. Note that the DCS was added, so we should
             * see zero. If it is wrong, we need to backtrack to get the
             * expected value.
             */
            if (cksum != 0) {
               PRINTF_WARN("PN532", "Got the DCS to be %02x when expected %02x",
                     msg, 0x100 - (0xff & (cksum - msg)));
            }
            pn532state = LOCK;
            break;
         }
         case LOCK:
            /* We check the command to make sure it is a LOCK. */
            if (msg != 0x00) {
               PRINTF_WARN("PN532", "Relock did not match!");
            }
            /* And we return to idle. */
            pn532state = IDLE;
            /* Now we can execute the command. */
            if (mif.cmd == 0x4a) {
               PRINTF_INFO("TEST", "Accepted Inlist Passive Target");
               /* We put then the response in the buffer */
               sethasdata();
               pushack();
            }
            else if (mif.cmd == 0x14) {
               PRINTF_INFO("TEST", "Accepted SAM configuration command");
               /* We put then the response in the buffer */
               sethasdata();
               pushack();
            }
            else if (mif.cmd == 0x02) {
               PRINTF_INFO("TEST", "Accepted getVersion command");
               sethasdata();
               pushack();
            }
      }
      pnstate.write(pn532state);
   }
}

void pn532::trace(sc_trace_file *tf) {
   sc_trace(tf, pnstate, pnstate.name());
   sc_trace(tf, icstate, icstate.name());
   sc_trace(tf, intoken, intoken.name());
   sc_trace(tf, outtoken, outtoken.name());
   sc_trace(tf, ackirq_ev, ackirq_ev.name());
}
