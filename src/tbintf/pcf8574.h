/*******************************************************************************
 * pcf8574.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a model for the PC8574 I2C GPIO expander.
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

#define PCF8574_PINS 8
SC_MODULE(pcf8574) {
   /* Signals */
   sc_inout<gn_mixed> sda {"sda"};
   sc_inout<gn_mixed> scl {"scl"};
   sc_port<sc_signal_inout_if<gn_mixed>,PCF8574_PINS> sig {"sig"};
   sc_out<gn_mixed> intr {"intr"};
   sc_event clearintr_ev {"clearintr_ev"};
   sc_signal<int> state {"state"};

   /* Processes */
   void intr_th(void);
   void i2c_th(void);

   /* Functions */
   void trace(sc_trace_file *tf);
   /* The device ID can change. If this is a PCF8574, the ID can go from
    * 20h to 27h. If this is a PCF8574A it can go from 38h to 3Fh. The default
    * value is 20h.
    */
   void set_addr(unsigned char _ad) { device_id = _ad; }

   // Constructor
   SC_CTOR(pcf8574) {
      i2cstate = IDLE;
      device_id = 0x20;
      drive = 0xff;

      SC_THREAD(intr_th);

      SC_THREAD(i2c_th);
      sensitive << scl << sda;
   }

   private:
   enum {IDLE, DEVID, ACKWR, ACKRD, RETZ, ACKWRD, WRDATA, READ, ACKNACK}
      i2cstate;
   unsigned char device_id;
   unsigned char drive;
   unsigned char sampleport();
};
