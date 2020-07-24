/*******************************************************************************
 * readMifaretest.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench header for the test. It describes how
 * the model should be wired up and connected to any monitors.
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
#include <Arduino.h>
#include "uartclient.h"
#include <vector>
#include "doitesp32devkitv1_i2c.h"
#include "pn532.h"

SC_MODULE(readMifaretest) {
   /* Signals */
   gn_signal_mix i2c_sda {"i2c_sda"};
   gn_signal_mix i2c_scl {"i2c_scl"};
   gn_signal_mix pn532_reset {"pn532_reset"};
   gn_signal_mix pn532_irq {"pn532_irq"};
   sc_signal<bool> pn532_irq_dig {"pn532_irq_dig"};
   gn_signal_mix led {"led"};
   gn_signal_mix rx {"rx"};
   gn_signal_mix tx {"tx"};
   sc_signal<unsigned int> fromwifi {"fromwifi"};
   sc_signal<unsigned int> towifi {"towifi"};

   /* Note: these will soon be replaced with better interfaces. */
   sc_signal<unsigned int> fromflash {"fromflash"};
   sc_signal<unsigned int> toflash {"toflash"};

   /* Unconnected signals */
   gn_signal_mix logic_0 {"logic_0", GN_LOGIC_0};

   /* I2C UART */
   pn532 i_pn532 {"i_pn532"};

   /* blocks */
   doitesp32devkitv1_i2c i_esp{"i_esp"};
   uartclient i_uartclient{"i_uartclient"};
   netcon_booltomix i_netcon{"i_netcon"};

   /* Processes */
   void testbench(void);
   void serflush(void);

   /* Tests */
   unsigned int tn; /* Testcase number */
   void t0();

   // Constructor
   SC_CTOR(readMifaretest) {

      /* UART 0 - we connect the wires to the corresponding tasks. Yes, the
       * RX and TX need to be switched.
       */
      i_esp.d3(rx); i_uartclient.tx(rx);
      i_esp.d1(tx); i_uartclient.rx(tx);

      /* LED */
      i_esp.d2_a12(led);

      /* Other interfaces, none are used so they are just left floating. */
      i_esp.wrx(fromwifi); i_esp.wtx(towifi);
      /* Note: these two will soon be replaced with the real flash and I2C
       * interfaces, as soon as someone takes the time to implement them. */
      i_esp.frx(fromflash); i_esp.ftx(toflash);
      i_esp.d13_a14(pn532_reset);
      i_esp.d12_a15(pn532_irq);

      /* I2C BUS */
      i_esp.d21(i2c_sda); i_esp.d22(i2c_scl);
      i_pn532.sda(i2c_sda); i_pn532.scl(i2c_scl); i_pn532.irq(pn532_irq_dig);
      i_netcon.a(pn532_irq_dig); i_netcon.b(pn532_irq);

      /* Pins not used in this simulation */
      i_esp.d0_a11(logic_0); /* BOOT pin */
      i_esp.d26_a19(logic_0); i_esp.d27_a17(logic_0);
      i_esp.d4_a10(logic_0); i_esp.d5(logic_0);
      i_esp.d14_a16(logic_0);
      i_esp.d15_a13(logic_0); i_esp.d16(logic_0);
      i_esp.d17(logic_0); i_esp.d18(logic_0); i_esp.d19(logic_0);
      i_esp.d23(logic_0);
      i_esp.d25_a18(logic_0);
      i_esp.d33_a5(logic_0); i_esp.d34_a6(logic_0);
      i_esp.d35_a7(logic_0); i_esp.d36_a0(logic_0); i_esp.d37_a1(logic_0);
      i_esp.d38_a2(logic_0); i_esp.d39_a3(logic_0);
      i_esp.d32_a4(logic_0);

      SC_THREAD(testbench);
      SC_THREAD(serflush);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
