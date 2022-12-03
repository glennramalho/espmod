/*******************************************************************************
 * pn532_base.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
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
#include "pn532_base.h"
#include "info.h"

void pn532_base::pushack() {
   to.write(0x00);
   to.write(0x00);
   to.write(0xFF);
   to.write(0x00);
   to.write(0xFF);
   to.write(0x00);
}

void pn532_base::pushsyntaxerr() {
   to.write(0x00);
   to.write(0x00);
   to.write(0xFF);
   to.write(0x01);
   to.write(0xFF);
   to.write(0x7F);
   to.write(0x81);
   to.write(0x00);
}

void pn532_base::pushpreamble(int len, bool hosttopn, int cmd,
      unsigned char *c) {
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

void pn532_base::setcardnotpresent() {
   mif.tags = 0;
   mif.sens_res = 0x0000;
   mif.sel_res = 0;
   mif.uidLength = 0;
   mif.uidValue = 0x0000;
}

void pn532_base::setcardpresent(uint32_t uid) {
   mif.tags = 1;
   mif.sens_res = 0x5522;
   mif.sel_res = 0;
   mif.uidLength = 4;
   mif.uidValue = uid;
   std::map<int, data_block_t>::iterator i;
   for(i = mif.mem.begin(); i != mif.mem.end(); i++)
      i->second.authenticated = false;
   mif.lastincmd = 0x0;
   mif.bn = 0x0;
}

void pn532_base::mifset(int pos, const char *value) {
   int i;
   bool fillzero;
   fillzero = false;
   for (i = 0; i < 15; i = i + 1) {
      if (!fillzero) {
         mif.mem[pos].data[i] = value[i];
         if (value[i] == '\0') fillzero = true;
      } else mif.mem[pos].data[i] = 0;
   }
}

void pn532_base::mifsetn(int pos, const uint8_t *value, int len) {
   int i;
   bool fillzero;
   fillzero = false;
   for (i = 0; i < 15; i = i + 1) {
      if (!fillzero) {
         mif.mem[pos].data[i] = value[i];
         if (i == len-1) fillzero = true;
      } else mif.mem[pos].data[i] = 0;
   }
}

void pn532_base::start_of_simulation() {
   /* We begin setting the card as non-present. */
   setcardnotpresent();
   mif.mxrtypassiveactivation = 0xff;
   /* And we initialize the device to off and no IRQ. */
   opstate.write(OPOFF);
}

pn532_base::resp_t pn532_base::pushresp() {
   unsigned char cksum = 0;

   /* Now we can start processing the commands. */
   if (mif.cmd == 0x4a && !mif.cmdbad) {
      /* If we got a card, we return the data on it. */
      if (mif.tags > 0) {
         /* Calculate size. */
         int len = 1 + 1 + mif.tags * (1+2+1+1+mif.uidLength) + 1;
         int i;
         /* Preamble */
         pushpreamble(len, false, 0x4B, &cksum);
         /* Packet */
         pushandcalc(mif.tags, &cksum);/* NbTg */
         for(i = 1; i <= mif.tags; i = i + 1) {
            pushandcalc(i, &cksum); /* Tag no. */
            pushandcalc(mif.sens_res>>8, &cksum);/* sens res upper */
            pushandcalc(mif.sens_res&0xff, &cksum);/* sens res lower */
            pushandcalc(mif.sel_res, &cksum); /* sel res */
            pushandcalc(mif.uidLength, &cksum); /* NFCID Len */
         }
         /* NFCID */
         pushandcalc(mif.uidValue>>24, &cksum);
         pushandcalc((mif.uidValue&0xff0000)>>16, &cksum);
         pushandcalc((mif.uidValue&0xff00)>>8, &cksum);
         pushandcalc(mif.uidValue&0xff, &cksum);
         to.write(0x100-cksum); /* DCS */
         to.write(0x00); /* ZERO */

      /* If there is no card, and it is configured to infinite retries, we need
       * to try again.
       */
      } else if (mif.retries == 0xff) {
         return RESP_RETRY;
      /* If we had a number of retries set, we then decrement the count and
       * retry.
       */
      } else if (mif.retries > 0) {
         mif.retries = mif.retries - 1;
         return RESP_RETRY;
      /* If we are out of retries, we can then return an empty frame. */
      } else {
         /* If no target came in, we return an empty list. */
         /* Preamble */
         pushpreamble(0x3, false, 0x4B, &cksum);
         /* Packet */
         pushandcalc(0, &cksum); /* NbTg */
         to.write(0x100-cksum); /* DCS */
         to.write(0x00); /* ZERO */
      }
   }
   /* SAMConfiguration command */
   else if (mif.cmd == 0x14 && !mif.cmdbad) {
      pushpreamble(0x2, false, 0x15, &cksum);
      to.write(0x100-cksum); /* DCS */
      to.write(0x00); /* ZERO */
   }
   /* Get Firmware Version. */
   else if (mif.cmd == 0x02 && !mif.cmdbad) {
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
   /* InDataExchange Command */
   else if (mif.cmd == 0x40) {
      /* We don't process the actual MiFare commands, we just return that it
       * was authenticated ok.
       */
      /* We return the data. If the command is bad, we return a failure and
       * some random junk.
       */
      if (mif.cmdbad) {
         PRINTF_INFO("MIFARE", "Command Rejected");
         pushpreamble(0x06, false, 0x41, &cksum);
         pushandcalc(0x55, &cksum); /* BAD */
         pushandcalc(mif.lastincmd, &cksum);
         pushandcalc(0x02, &cksum);
         pushandcalc(0x03, &cksum);
      } else if (mif.lastincmd == 0x60) {
         PRINTF_INFO("MIFARE", "Block 0x%02x Authenticated", mif.bn);
         pushpreamble(0x06, false, 0x41, &cksum);
         pushandcalc(0x00, &cksum); /* OK */
         pushandcalc(mif.lastincmd, &cksum);
         pushandcalc(0x02, &cksum);
         pushandcalc(0x03, &cksum);
      } else if (mif.lastincmd == 0xA0) {
         PRINTF_INFO("MIFARE", "Write to block 0x%02x", mif.bn);
         pushpreamble(0x06, false, 0x41, &cksum);
         pushandcalc(0x00, &cksum); /* OK */
         pushandcalc(mif.lastincmd, &cksum);
         pushandcalc(0x02, &cksum);
         pushandcalc(0x03, &cksum);
      } else if (mif.lastincmd == 0x30) {
         PRINTF_INFO("MIFARE", "Read from block 0x%02x", mif.bn);
         int p;
         pushpreamble(21, false, 0x41, &cksum);
         pushandcalc(0x00, &cksum); /* OK */
         p = 0;
         while (p < 16) {
            pushandcalc(mif.mem[mif.bn].data[p], &cksum);
            p = p + 1;
         }
         pushandcalc(0x90, &cksum);
         pushandcalc(0x00, &cksum);
      } else {
         PRINTF_INFO("MIFARE", "Other Command");
         pushpreamble(0x06, false, 0x41, &cksum);
         pushandcalc(0x00, &cksum); /* OK */
         pushandcalc(mif.lastincmd, &cksum);
         pushandcalc(0x02, &cksum);
         pushandcalc(0x03, &cksum);
      }
      to.write(0x100-cksum); /* DCS */
      to.write(0x00); /* ZERO */
   }
   else {
      /* For unknown commands we just return a syntax error. */
      pushsyntaxerr();
   }

   /* We also clear the command so we do not keep on sending it back. */
   mif.cmd = 0;
   return RESP_OK;
}

void pn532_base::resp_th() {
   while(true) {
      /* We first wait for a command to come in. */
      wait(newcommand_ev | ack_ev);

      /* Anytime we get a new command, we need to dump what we were doing before
       * and start it. So, we move the state back to OPACK to issue the new ACK.
       */
      if (newcommand_ev.triggered()) {
         ack_ev.notify(200, SC_US);
         opstate.write(OPACK);

      /* For other times, we just process whatever command came in. */
      } else switch (opstate.read()) {
         /* The manual was not clear what happens if the host does not read
          * the ACK. There are three possible options:
          * - it gets merged with the next command
          * - it gets overwitten
          * - the command does not start until the ACK was read
          * I am then assuming the third is the one.
          */
         case OPACK:
            /* We dump any previous possible data in the to FIFO, like a
             * previous unread response, and we push the ACK. Then we wait
             * for it to be read out.
             */
            flush_to();
            pushack();
            opstate.write(OPACKRDOUT);
            break;

         /* After the ACK has been read out, we do a delay. Some commands have
          * a predelay too.
          */
         case OPACKRDOUT:
            if (mif.predelay != 0) {
               ack_ev.notify(sc_time(mif.predelay, SC_MS));
               opstate.write(OPPREDELAY);
            } else {
               ack_ev.notify(sc_time(mif.delay, SC_MS));
               opstate.write(OPBUSY);
            }
            break;
         case OPPREDELAY:
            ack_ev.notify(sc_time(mif.delay, SC_MS));
            opstate.write(OPBUSY);
            break;
         /* Then we send the response. */
         case OPBUSY:
            flush_to();
            /* We try to push a response. If there is a retry request, we
             * reissue the delay and do it again.
             */
            if (RESP_RETRY == pushresp())
               ack_ev.notify(sc_time(mif.delay, SC_MS));

            /* If the response was ok, we go to the READOUT state. */
            else opstate.write(OPREADOUT);
            break;
         case OPREADOUT:
            opstate.write(OPIDLE);
         default: ;
      }
   }
}

void pn532_base::process_th() {
   unsigned char cksum = 0;
   unsigned char msg;
   enum {IDLE, UNLOCK1, UNLOCK2, UNLOCK3, UNLOCK4, UNLOCK5, PD0, PDN, DCS,
      LOCK} pn532state;

   while(true) {
      msg = grab(&cksum);
      switch(pn532state) {
         case IDLE:
            if (msg == 0x0) pn532state = UNLOCK1;
            else {
               PRINTF_WARN("PN532", "Warning: got an illegal preamble.");
               pn532state = IDLE;
            }
            break;
         case UNLOCK1:
            if (msg == 0x0) pn532state = UNLOCK2;
            else {
               PRINTF_WARN("PN532","Warning: got an illegal preamble, byte 2.");
               pn532state = IDLE;
            }
            break;
         case UNLOCK2:
            /* The first 0x0 can be as long as the customer wants, so we
             * discard any extra 0x0 received.
             */
            if (msg == 0x0) pn532state = UNLOCK2;
            else if (msg == 0xff) pn532state = UNLOCK3;
            else {
               PRINTF_WARN("PN532","Warning: got an illegal preamble, byte 3.");
               pn532state = IDLE;
            }
            break;
         case UNLOCK3:
            /* And we collect the length of the next command. */
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
            mif.cmdbad = false;

            /* SAM command */
            if (mif.cmd == 0x14) {
               mif.mode = msg;
               if (mif.len<2) {
                  mif.cmdbad = true;
               } else {
                  mif.timeout=grab(&cksum); mif.len = mif.len-1;
                  mif.useirq=grab(&cksum); mif.len = mif.len-1;
               }
            }
            /* InDataExchange */
            else if (mif.cmd == 0x40) {
               /* For the InDataExchange command we are talking to the card.
                * This varies a lot according to the card, and it can get
                * quite complex. We are trying to do something simple here,
                * so all we do is check for a few commands.
                */
               /* We first get the header. */
               if (mif.len < 2) mif.cmdbad = true;
               else {
                  /* msg has the Target No. */
                  mif.lastincmd = grab(&cksum); mif.len = mif.len - 1; /* cmd */
                  mif.bn = grab(&cksum); mif.len = mif.len - 1; /* Block No. */
                  /* Authenticate */
                  if (mif.lastincmd == 0x60 && mif.len >= 10) {
                     mif.mem[mif.bn].authenticated = true;
                  /* Write */
                  } else if (mif.lastincmd == 0xA0 && mif.len >= 16 &&
                        mif.mem[mif.bn].authenticated) {
                     int p;
                     p = 0;
                     while (p < 16) {
                        mif.mem[mif.bn].data[p] = grab(&cksum);
                        mif.len = mif.len - 1;
                        p = p + 1;
                     }
                  /* Read */
                  } else if (mif.lastincmd == 0x30 &&
                        mif.mem[mif.bn].authenticated) {
                     ;
                  /* Other commands we simply return a good as we do not know
                   * what to do.
                   */
                  } else {
                     mif.cmdbad = false;
                  }
               }
            }
            else if (mif.cmd == 0x32) {
               int cfgitem = msg;
               int cfg[3];
               switch (cfgitem) {
                  case 0x1: 
                     cfg[0] = grab(&cksum); mif.len = mif.len - 1;
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x1]=%02x", cfg[0]);
                     break;
                  case 0x2:
                     cfg[0] = grab(&cksum); mif.len = mif.len - 1;
                     cfg[1] = grab(&cksum); mif.len = mif.len - 1;
                     cfg[2] = grab(&cksum); mif.len = mif.len - 1;
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x2]=RFU: %02x", cfg[0]);
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x2]=fATR_RES_Timeout: %02x",
                        cfg[1]);
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x2]=fRetryTimeout: %02x",
                        cfg[2]);
                     break;
                  case 0x4:
                     cfg[0] = grab(&cksum); mif.len = mif.len - 1;
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x4]=MxRetryCOM: %02x", 
                           cfg[0]);
                     break;
                  case 0x5:
                     cfg[0] = grab(&cksum); mif.len = mif.len - 1;
                     cfg[1] = grab(&cksum); mif.len = mif.len - 1;
                     cfg[2] = grab(&cksum); mif.len = mif.len - 1;
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x5]=MxRtyATR: %02x",cfg[0]);
                     PRINTF_INFO("PN532",
                        "Accepted RFConfiguration [0x5]=MxRtyPSL: %02x",
                        cfg[1]);
                     PRINTF_INFO("PN532",
                  "Accepted RFConfiguration [0x5]=MxRtyPassiveActivation: %02x",
                        cfg[2]);
                     mif.mxrtypassiveactivation = cfg[2];
                     break;
               }
               if (mif.len<1) {
                  mif.cmdbad = true;
               } else {
                  mif.brty = grab(&cksum); mif.len = mif.len - 1;
               }

               /* brty=00 only supports 1 or 2 targets. */
               if (mif.maxtg > 2 || mif.maxtg <= 0) mif.cmdbad = true;
               if (mif.brty != 0x00) mif.cmdbad = true; /* we only support 00*/
            }

            else if (mif.cmd == 0x4a) {
               mif.maxtg = msg;
               if (mif.len<1) {
                  mif.cmdbad = true;
               } else {
                  mif.brty = grab(&cksum); mif.len = mif.len - 1;
               }

               /* brty=00 only supports 1 or 2 targets. */
               if (mif.maxtg > 2 || mif.maxtg <= 0) mif.cmdbad = true;
               if (mif.brty != 0x00) mif.cmdbad = true; /* we only support 00*/
            }

            /* We flush out the rest of the payload. */
            while (mif.len > 0) {
               pn532state = DCS;
               int i = grab(&cksum);
               PRINTF_INFO("I", "%02x", i);
               mif.len = mif.len - 1;
            }

            /* And we go to the DCS. */
            pn532state = DCS;
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
               PRINTF_INFO("PN532", "Accepted Inlist Passive Target");
               /* We put then the response in the buffer */
               mif.predelay = 5;
               mif.delay = 200;
               /* We set the retries to the maximum set and notify the process
                * to begin.
                */
               mif.retries = mif.mxrtypassiveactivation;
               newcommand_ev.notify();
            }
            else if (mif.cmd == 0x14) {
               PRINTF_INFO("PN532", "Accepted SAM configuration command");
               /* We put then the response in the buffer */
               mif.predelay = 5;
               mif.delay = 20;
               newcommand_ev.notify();
            }
            else if (mif.cmd == 0x02) {
               PRINTF_INFO("PN532", "Accepted getVersion command");
               mif.predelay = 2;
               mif.delay = 20;
               newcommand_ev.notify();
            }
            else if (mif.cmd == 0x40) {
               PRINTF_INFO("PN532", "Accepted InDataExchange command");
               mif.predelay = 0;
               mif.delay = 1;
               newcommand_ev.notify();
            }
            else if (mif.cmd == 0x32) {
               mif.predelay = 0;
               mif.delay = 1;
               newcommand_ev.notify();
            }
            else {
               /* Illegal commands also are processed as commands, so an
                * ACK is returned, just the data packet has a syntax error.
                */
               mif.predelay = 0;
               mif.delay = 1;
               PRINTF_INFO("PN532", "Accepted Unknown command");
               newcommand_ev.notify();
            }
      }
      pnstate.write(pn532state);
   }
}

/* For the IRQ manage, we initialize the thread taking the pin high. Then, when
 * the TO fifo changes, we will take it either high or low.
 */
void pn532_base::irqmanage_th() {
   bool wasempty = true;
   irq.write(GN_LOGIC_1);
   while(1) {
      wait(to.data_written_event() | to.data_read_event());

      if (to.num_available() == 0 && !wasempty) {
         irq.write(GN_LOGIC_1);
         /* If the read_th was waiting to read data, we then wake it up to
          * go to the next state.
          */
         if (opstate.read() == OPACKRDOUT || opstate.read() == OPREADOUT)
            ack_ev.notify();
      }
      else irq.write(GN_LOGIC_0);

      wasempty = to.num_available() == 0;
   }
}

void pn532_base::trace(sc_trace_file *tf) {
   sc_trace(tf, opstate, opstate.name());
   sc_trace(tf, ack_ev, ack_ev.name());
   sc_trace(tf, newcommand_ev, newcommand_ev.name());
   sc_trace(tf, pnstate, pnstate.name());
   sc_trace(tf, intoken, intoken.name());
   sc_trace(tf, outtoken, outtoken.name());
   sc_trace(tf, irq, irq.name());
}
