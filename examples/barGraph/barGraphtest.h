/*******************************************************************************
 * barGraphtest.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench header for the barGrapphtest.ino test. It
 * describes how the model should be wired up and connected to any monitors.
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
#include "doitesp32devkitv1.h"
#include "uartclient.h"
#include "gn_mixed.h"

SC_MODULE(barGraphtest) {
   /* Signals */
   gn_signal_mix a_in {"a_in"};
   gn_signal_mix led1 {"led1"};
   gn_signal_mix led2 {"led2"};
   gn_signal_mix led3 {"led3"};
   sc_signal_resolved led4 {"led4"};
   sc_signal_resolved led5 {"led5"};
   sc_signal_resolved led6 {"led6"};
   sc_signal_resolved led7 {"led7"};
   sc_signal_resolved led8 {"led8"};
   sc_signal_resolved led9 {"led9"};
   sc_signal_resolved led10 {"led10"};
   sc_signal_resolved led11 {"led11"};
   sc_signal<unsigned int> fromwifi {"fromwifi"};
   sc_signal<unsigned int> towifi {"towifi"};

   /* Note: these will soon be replaced with better interfaces. */
   sc_signal<unsigned int> fromflash {"fromflash"};
   sc_signal<unsigned int> toflash {"toflash"};
   sc_signal<bool> fromi2c {"fromi2c"};
   sc_signal<bool> toi2c {"toi2c"};

   /* Unconnected signals */
   sc_signal_resolved logic_0 {"logic_0"};
   gn_signal_mix logic_mix_0 {"logic_0"};

   /* blocks */
   doitesp32devkitv1 i_esp{"i_esp"};

   /* Processes */
   void testbench(void);

   /* Tests */
   unsigned int tn; /* Testcase number */
   void t0();

   // Constructor
   SC_CTOR(barGraphtest) {
      /* LED */
      i_esp.d15_a13(led1);
      i_esp.d2_a12(led2);
      i_esp.d4_a10(led3);
      i_esp.d16(led4); /* RXD2 */
      i_esp.d5(led5);
      i_esp.d18(led6);
      i_esp.d19(led7);
      i_esp.d21(led8);
      i_esp.d3(led9); /* RXD0 */
      i_esp.d1(led10); /* TXD0 */

      /* Analog in */
      i_esp.d36_a0(a_in);

      /* Other interfaces, none are used so they are just left floating. */
      i_esp.wrx(fromwifi); i_esp.wtx(towifi);
      /* Note: these two will soon be replaced with the real flash and I2C
       * interfaces, as soon as someone takes the time to implement them. */
      i_esp.frx(fromflash); i_esp.ftx(toflash);
      i_esp.irx(fromi2c); i_esp.itx(toi2c);

      /* Pins not used in this simulation */
      i_esp.d0_a11(logic_mix_0); i_esp.d11(logic_0);
      i_esp.d12_a15(logic_mix_0); i_esp.d13_a14(logic_mix_0);
      i_esp.d14_a16(logic_mix_0);
      i_esp.d17(logic_0); i_esp.d22(logic_0); i_esp.d23(logic_0);
      i_esp.d25_a18(logic_mix_0); i_esp.d26_a19(logic_mix_0);
      i_esp.d27_a17(logic_mix_0); i_esp.d32_a4(logic_mix_0);
      i_esp.d33_a5(logic_mix_0); i_esp.d34_a6(logic_mix_0);
      i_esp.d35_a7(logic_mix_0); i_esp.d37_a1(logic_mix_0);
      i_esp.d38_a2(logic_mix_0); i_esp.d39_a3(logic_mix_0);

      SC_THREAD(testbench);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
