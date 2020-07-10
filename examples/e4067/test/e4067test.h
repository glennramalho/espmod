/*******************************************************************************
 * e4067test.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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
#include "doitesp32devkitv1.h"
#include "uartclient.h"
#include "cd4067.h"

SC_MODULE(e4067test) {
   /* Signals */
   sc_signal<bool> led {"led"};
   gn_signal_mix d2_a12 {"d2_a12"};
   gn_signal_mix rx {"rx"};
   gn_signal_mix tx {"tx"};
   sc_signal<unsigned int> fromwifi {"fromwifi"};
   sc_signal<unsigned int> towifi {"towifi"};
   gn_signal_mix a0 {"a0"};
   gn_signal_mix a1 {"a1"};
   gn_signal_mix a2 {"a2"};
   gn_signal_mix adc {"adc"};

   /* Note: these will soon be replaced with better interfaces. */
   sc_signal<unsigned int> fromflash {"fromflash"};
   sc_signal<unsigned int> toflash {"toflash"};
   sc_signal<bool> fromi2c {"fromi2c"};
   sc_signal<bool> toi2c {"toi2c"};

   /* Unconnected signals */
   gn_signal_mix logic_0 {"logic_0", GN_LOGIC_0};
   gn_signal_mix d0 {"d0"}; gn_signal_mix d1 {"d1"};
   gn_signal_mix d2 {"d2"}; gn_signal_mix d3 {"d3"};
   sc_signal<bool> d0_bool {"d0_bool"}; sc_signal<bool> d1_bool {"d1_bool"};
   sc_signal<bool> d2_bool {"d2_bool"}; sc_signal<bool> d3_bool {"d3_bool"};
   sc_signal<bool> inh {"inh", false};

   /* blocks */
   doitesp32devkitv1 i_esp{"i_esp"};
   uartclient i_uartclient{"i_uartclient"};
   netcon_mixtobool i_netcon{"i_netcon"};
   netcon_mixtobool i_netcon_s0{"i_netcon_s0"};
   netcon_mixtobool i_netcon_s1{"i_netcon_s1"};
   netcon_mixtobool i_netcon_s2{"i_netcon_s2"};
   netcon_mixtobool i_netcon_s3{"i_netcon_s3"};
   cd4067 i_cd4067{"i_cd4067"};

   /* Processes */
   void testbench(void);
   void serflush(void);

   /* Tests */
   unsigned int tn; /* Testcase number */
   void t0();

   // Constructor
   SC_CTOR(e4067test) {

      /* UART 0 - we connect the wires to the corresponding tasks. Yes, the
       * RX and TX need to be switched.
       */
      i_esp.d3(rx); i_uartclient.tx(rx);
      i_esp.d1(tx); i_uartclient.rx(tx);

      /* LED */
      i_esp.d2_a12(d2_a12);
      i_netcon.a(d2_a12);
      i_netcon.b(led);

      /* We connect the waveform to these. */

      /* Other interfaces, none are used so they are just left floating. */
      i_esp.wrx(fromwifi); i_esp.wtx(towifi);
      /* Note: these two will soon be replaced with the real flash and I2C
       * interfaces, as soon as someone takes the time to implement them. */
      i_esp.frx(fromflash); i_esp.ftx(toflash);
      i_esp.irx(fromi2c); i_esp.itx(toi2c);
      i_esp.d36_a0(adc); 
      i_esp.d4_a10(d0); i_esp.d5(d1); i_esp.d14_a16(d2); i_esp.d13_a14(d3);

      /* CD4067 */
      i_netcon_s0.a(d0); i_netcon_s0.b(d0_bool);
      i_netcon_s1.a(d1); i_netcon_s1.b(d1_bool);
      i_netcon_s2.a(d2); i_netcon_s2.b(d2_bool);
      i_netcon_s3.a(d3); i_netcon_s3.b(d3_bool);
      i_cd4067.inh(inh);
      i_cd4067.channel(a0);
      i_cd4067.channel(a1);
      i_cd4067.channel(a2);
      i_cd4067.a(d0_bool);
      i_cd4067.b(d1_bool);
      i_cd4067.c(d2_bool);
      i_cd4067.d(d3_bool);
      i_cd4067.x(adc);

      /* Pins not used in this simulation */
      i_esp.d0_a11(logic_0); /* BOOT pin */
      i_esp.d12_a15(logic_0); i_esp.d21(logic_0);
      i_esp.d26_a19(logic_0); i_esp.d27_a17(logic_0);
      i_esp.d15_a13(logic_0); i_esp.d16(logic_0);
      i_esp.d17(logic_0); i_esp.d18(logic_0); i_esp.d19(logic_0);
      i_esp.d23(logic_0);
      i_esp.d25_a18(logic_0);
      i_esp.d33_a5(logic_0); i_esp.d34_a6(logic_0);
      i_esp.d35_a7(logic_0); i_esp.d37_a1(logic_0);
      i_esp.d38_a2(logic_0); i_esp.d39_a3(logic_0);
      i_esp.d22(logic_0); i_esp.d32_a4(logic_0);

      SC_THREAD(testbench);
      SC_THREAD(serflush);
   }

   void trace(sc_trace_file *tf);
   void start_of_simulation();
};
