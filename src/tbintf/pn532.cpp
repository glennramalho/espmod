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

void pn532::i2c_th(void) {
   int i;
   unsigned int devid, data;
   enum {IDLE, DEVID, ACKWR, ACKRD, RETZ, ACKWRD, WRDATA, READ, ACKNACK}
      i2cstate;
   enum {BUFFER_UNKNOWN, BUFFER_NOTUSED, BUFFER_VALID, BUFFER_INVALID}
     buffer_state;

   buffer_state = BUFFER_UNKNOWN;

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
            buffer_state = BUFFER_UNKNOWN;
         }
            /* STOP BIT */
         else {
            /* If we see a stop bit and we were giving the customer a buffer,
             * we flush it out. If we were not giving it a buffer, or it was
             * an invalid buffer, we do nothing.
             */
            if (buffer_state == BUFFER_VALID) flush_to();
            i2cstate = IDLE;
         }
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
            /* If the ID matches, we can process it. First we handle reads. */
            else if (devid == PN532_ID && sda.read().ishigh()) {
               /* We will go to ACKRD. */
               i2cstate = ACKRD;
               /* We check the status of the buffer. If there is data to
                * be read out, it is valid. If not, it is invalid.
                */
               if (to.num_available() == 0) buffer_state = BUFFER_INVALID;
               else buffer_state = BUFFER_VALID;
            }
            else if (devid == PN532_ID && sda.read().islow()) {
               i2cstate = ACKWR;
               buffer_state = BUFFER_NOTUSED;
            }
            /* If the devid does not match, we return Z. */
            else i2cstate = RETZ;

            i = i - 1;
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
            /* If we are reading the buffer, and it is valid, we return 01 and
             * then the rest will be data. If it is invalid, we return 00 and
             * then we return garbage. For writes, we ignore this.
             */
            if (buffer_state == BUFFER_INVALID) {
               data = 0x00;
               outtoken.write(data); /* We also update the debug outtoken. */
               i2cstate = READ;
            }
            else if (buffer_state == BUFFER_VALID) {
               data = 0x01;
               outtoken.write(data);
               i2cstate = READ;
            }
            /* And for writes, the buffer is not used, we do the write. */
            else {
               data = 0;
               i2cstate = WRDATA;

               /* Before we start, we dump anything that could be left over
                * from the previous command.
                */
               while(from.num_available() != 0) (void)from.read();
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
            /* If we got a NACK, we flush out the buffer and quit. */
            if (sda.read().ishigh()) {
               i2cstate = IDLE;
               if (buffer_state == BUFFER_VALID) flush_to();
            }
            /* If we got an ACK but the buffer was invalid, we return junk. */
            else if (buffer_state == BUFFER_INVALID) {
               i2cstate = READ;
               i = 7;
               data = 0x7f;
               outtoken.write(data);
            }
            /* If we got an ACK and the buffer is valid, we return the data. */
            else {
               i2cstate = READ;
               i = 7;
               /* If the buffer is empty, we return 0. */
               if (to.num_available() == 0) data = 0;
               else data = to.read();
               outtoken.write(data);
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
            if (buffer_state == BUFFER_INVALID) sda.write(GN_LOGIC_0);
            else if ((data & (1 << i))>0) sda.write(GN_LOGIC_Z);
            else sda.write(GN_LOGIC_0);
            break;
         default: ; /* For the others we do nothing. */
      }

      icstate.write(i2cstate);
   }
}

void pn532::trace(sc_trace_file *tf) {
   sc_trace(tf, icstate, icstate.name());
   pn532_base::trace(tf);
}
