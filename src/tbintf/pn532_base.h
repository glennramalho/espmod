/*******************************************************************************
 * pn532_base.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a crude model for the PN532 with firmware v1.6. It will work for
 *   basic work with a PN532 system. This is the base class, without I/Os.
 *   Usually one of the derived classes should be used.
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

#ifndef _PN532_BASE_H
#define _PN532_BASE_H

#include <systemc.h>
#include "gn_mixed.h"
#include <map>

struct data_block_t {
   char data[16];
   bool authenticated;
   data_block_t() {
      int i;
      for (i = 0; i < 16; i = i + 1) data[i] = 0;
      authenticated = false;
   }
};

SC_MODULE(pn532_base) {
   /* Signals */
   sc_out<gn_mixed> irq {"irq"};

   sc_signal<int> opstate {"opstate"};
   sc_signal<int> pnstate {"pnstate"};
   sc_signal<unsigned char> intoken {"intoken"};
   sc_signal<unsigned char> outtoken {"outtoken"};

   sc_fifo<unsigned char> to {"to"};
   sc_fifo<unsigned char> from {"from"};
   sc_event ack_ev {"ack_ev"};
   sc_event newcommand_ev {"newcommand_ev"};

   /* Functions */
   void trace(sc_trace_file *tf);
   void setcardnotpresent();
   void setcardpresent(uint32_t uid);
   void start_of_simulation();
   /* Processes */
   void process_th(void);
   void resp_th(void);
   void irqmanage_th(void);

   /* I2C */
   struct {
      unsigned char tags;
      unsigned int sens_res;
      unsigned char sel_res;
      unsigned char uidLength;
      uint32_t uidValue;
      unsigned char cmd;
      unsigned char mode;
      bool cmdbad;
      unsigned short int lastincmd;
      unsigned short int bn;
      unsigned short int maxtg;
      unsigned short int brty;
      std::map<int, data_block_t> mem;
      int len;
      int timeout;
      bool useirq;
      unsigned int delay;
      unsigned int predelay;
   } mif;

   // Constructor
   SC_CTOR(pn532_base) {
      SC_THREAD(process_th);
      SC_THREAD(resp_th);
      SC_THREAD(irqmanage_th);
   }

   void mifset(int pos, const char *value);
   void mifsetn(int pos, const uint8_t *value, int len);

   /* Private routines. */
   protected:
   typedef enum {OPOFF, OPIDLE, OPACK, OPACKRDOUT, OPPREDELAY, OPBUSY,
      OPREADOUT} op_t;
   void pushack();
   void pushsyntaxerr();
   void pushpreamble(int len, bool hosttopn, int cmd, unsigned char *c);
   void pushresp();
   void flush_to() { while(to.num_available() != 0) (void)to.read(); }
   unsigned char grab(unsigned char *c) {
      unsigned char m = from.read();
      intoken.write(m);
      if (c != NULL) *c = 0xff & (*c + m);
      return m;
   }
   void pushandcalc(const unsigned char m, unsigned char *cksum) {
      *cksum = 0xff & (*cksum + m);
      to.write(m);
   }
};

#endif
