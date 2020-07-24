/*******************************************************************************
 * pn532.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
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
#include "gn_mixed.h"

SC_MODULE(pn532) {
   /* Signals */
   sc_inout<gn_mixed> sda {"sda"};
   sc_inout<gn_mixed> scl {"scl"};
   gn_signal_mix reset {"reset"};
   sc_out<bool> irq {"irq"};

   sc_signal<int> pnstate {"pnstate"};
   sc_signal<int> icstate {"icstate"};
   sc_signal<unsigned char> intoken {"intoken"};
   sc_signal<unsigned char> outtoken {"outtoken"};

   sc_fifo<unsigned char> to {"to"};
   sc_fifo<unsigned char> from {"from"};
   sc_event ackirq_ev {"ackirq_ev"};

   /* Functions */
   void trace(sc_trace_file *tf);
   void setcardnotpresent();
   void setcardpresent();
   void start_of_simulation();
   /* Processes */
   void i2c_th(void);
   void process_th(void);

   /* I2C */
   struct {
      unsigned char tags;
      unsigned int sens_res;
      unsigned char sel_res;
      unsigned char uidLength;
      uint32_t uidValue;
      unsigned char cmd;
      unsigned char mode;
      int len;
      int timeout;
      bool useirq;
   } mif;

   // Constructor
   SC_CTOR(pn532) {
      SC_THREAD(i2c_th);
      SC_THREAD(process_th);
   }

   /* Private routines. */
   private:
   void pushack();
   void pushpreamble(int len, bool hosttopn, int cmd, unsigned char *c);
   void pushresp();
   unsigned char put() {
      unsigned char m = to.read();
      outtoken.write(m);
      return m;
   }
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
