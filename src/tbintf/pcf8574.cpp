/*******************************************************************************
 * pcf8574.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
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
#include "pcf8574.h"
#include "info.h"

unsigned char pcf8574::sampleport() {
   unsigned char samp = 0;
   int cnt;
   for(cnt = 0; cnt < sig.size(); cnt = cnt + 1) {
      if (!sig[cnt]->read().islogic()) {
         PRINTF_WARN("PCF8574", "Sampled a signal at level %c",
            sig[cnt]->read().to_char());
      }
      else if (sig[cnt]->read().ishigh()) samp = samp | (1 << cnt);
   }

   return samp;
}

void pcf8574::intr_th() {
   intr.write(GN_LOGIC_Z);
   while(1) {
      switch (sig.size()) {
         case 0: wait(clearintr_ev); break;
         case 1:
            wait(clearintr_ev | sig[0]->default_event());
            break;
         case 2:
            wait(clearintr_ev |
               sig[0]->default_event() | sig[1]->default_event());
            break;
         case 3:
            wait(clearintr_ev |
               sig[0]->default_event() | sig[1]->default_event() |
               sig[2]->default_event());
            break;
         case 4:
            wait(clearintr_ev |
                  sig[0]->default_event() | sig[1]->default_event() |
                  sig[2]->default_event() | sig[3]->default_event());
            break;
         case 5:
            wait(clearintr_ev |
                  sig[0]->default_event() | sig[1]->default_event() |
                  sig[2]->default_event() | sig[3]->default_event() |
                  sig[4]->default_event());
            break;
         case 6:
            wait(clearintr_ev |
                  sig[0]->default_event() | sig[1]->default_event() |
                  sig[2]->default_event() | sig[3]->default_event() |
                  sig[4]->default_event() | sig[5]->default_event());
            break;
         case 7:
            wait(clearintr_ev |
                  sig[0]->default_event() | sig[1]->default_event() |
                  sig[2]->default_event() | sig[3]->default_event() |
                  sig[4]->default_event() | sig[5]->default_event() |
                  sig[6]->default_event());
            break;
         default:
            wait(clearintr_ev |
                  sig[0]->default_event() | sig[1]->default_event() |
                  sig[2]->default_event() | sig[3]->default_event() |
                  sig[4]->default_event() | sig[5]->default_event() |
                  sig[6]->default_event() | sig[7]->default_event());
            break;
      }

      /* If we got a lowerintr we lower the line. Anything else raises it.
       * Note that we do not raise the interrupt while we were writing.
       */
      if (clearintr_ev.triggered()) intr.write(GN_LOGIC_Z);
      else if (i2cstate != WRDATA && i2cstate != ACKWRD && i2cstate != ACKWR)
         intr.write(GN_LOGIC_0);
   }
}




void pcf8574::i2c_th(void) {
   int i;
   unsigned int devid, data;

   /* We begin initializing all ports to weak 1. */
   for(i = 0; i < sig.size(); i = i + 1) {
      sig[i]->write(GN_LOGIC_W1);
   }

   /* The interrupt begins as Z. */
   intr.write(GN_LOGIC_Z);

   /* Then we begin waiting for the I2C commands. */
   while(1) {
      /* We wait for a change on either the SDA or the SCL. */
      wait();

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
            else if (devid == device_id && sda.read().ishigh()) i2cstate=ACKRD;
            else if (devid == device_id && sda.read().islow()) i2cstate=ACKWR;
            /* If the devid does not match, we return Z. */
            else i2cstate = RETZ;

            i = i - 1;
            break;
         /* If the address does not match, we return Z. */
         case RETZ:
            i2cstate = IDLE;
            break;
         /* When we recognize the ACKRD, we sample the pins. This will be
          * returned via the I2C.
          */
         case ACKRD:
            i = 7;
            data = sampleport();

            /* We also clear the interrupt. */
            clearintr_ev.notify();
         /* When we hit the ACKWR, all we do is clear the shiftregister to
          * begin to receive data.
          */
         case ACKWR:
            data = 0;
            i2cstate = WRDATA;
            /* We also clear the interrupt. */
            clearintr_ev.notify();
            break;
         /* Each bit is taken. */
         case WRDATA:
            data = (data << 1) + ((sda.read().ishigh())?1:0);
            if (i == 0) i2cstate = ACKWRD;
            else i = i - 1;
            break;
         /* When we have finished a write, we send back an ACK to the master.
          * We also will drive all pins strong.
          */
         case ACKWRD: {
            /* We first take in the new drive value received. */
            drive = data;

            /* In the ACKWRD state, we drive all pins strong, once we exit, we
             * lower the logic 1s to weak so that they can be read.
             */
            if (i2cstate == ACKWRD) {
               int cnt;
               for (cnt = 0; cnt < sig.size(); cnt = cnt + 1) {
                  sig[cnt]->write(((drive & (1<<cnt))>0)?GN_LOGIC_1:GN_LOGIC_0);
               }
            }
            /* Then we clear the shiftregister to begin to collect data from
             * the master.
             */
            data = 0;
            i = 7;
            i2cstate = WRDATA;
            /* We also clear the interrupt. */
            clearintr_ev.notify();
            break;
         }
         /* Depending on the acknack we either return more data or not. */
         case ACKNACK:
            if (sda.read().ishigh()) i2cstate = IDLE;
            else {
               /* If we got the ACK, we then clear the registers, sample the
                * I/Os and return to the read state.
                */
               i2cstate = READ;
               i = 7;
               data = sampleport();
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
            /* When we enter the WRDATA, we need to weaken the driving logic 1s.
             * We then scan through them and redrive any of them from 1 to W1.
             * We only need to do this if it is not all 0s.
             */
            if (i == 7 && drive != 0x0) {
               int cnt;
               for (cnt = 0; cnt < sig.size(); cnt = cnt + 1) {
                  if ((drive & (1<<cnt))>0) sig[cnt]->write(GN_LOGIC_W1);
               }
            }
            /* The SDA we float. */
            sda.write(GN_LOGIC_Z);
            break;
         case ACKNACK:
            /* The SDA we float. */
            sda.write(GN_LOGIC_Z);
            break;
         /* Each bit is returned. */
         case READ:
            if ((data & (1 << i))>0) sda.write(GN_LOGIC_Z);
            else sda.write(GN_LOGIC_0);
            break;
         default: ; /* For the others we do nothing. */
      }
   }
}
