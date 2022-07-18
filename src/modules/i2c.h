/*******************************************************************************
 * i2c.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Models a single ESP32 I2C
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

#ifndef _I2C_H
#define _I2C_H

#include <systemc.h>

SC_MODULE(i2c) {
   public:
   sc_out<bool> scl_en_o {"scl_en_o"};
   sc_out<bool> sda_en_o {"sda_en_o"};
   sc_in<bool> scl_i {"scl_i"};
   sc_in<bool> sda_i {"sda_i"};

   sc_fifo<unsigned char> to {"to", 32*8};
   sc_fifo<unsigned char> from {"from", 32*8};

   sc_signal<unsigned char> snd {"snd"};

   enum {IDLE, DEVID, READING, WRITING} state;
   void transfer_th();
   void trace(sc_trace_file *tf);

   // Constructor
   SC_CTOR(i2c) {
      SC_THREAD(transfer_th);
   }
};
extern i2c *i2c0ptr;
extern i2c *i2c1ptr;

#endif
