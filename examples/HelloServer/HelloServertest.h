/*******************************************************************************
 * HelloServertest.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench header for the HelloServer.ino test. It
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
#include "webclient.h"

SC_MODULE(HelloServertest) {
   /* Signals */
   sc_signal_resolved led {"led"};
   sc_signal_resolved rx {"rx"};
   sc_signal_resolved tx {"tx"};
   sc_signal<unsigned int> fromwifi {"fromwifi"};
   sc_signal<unsigned int> towifi {"towifi"};

   /* Note: these will soon be replaced with better interfaces. */
   sc_signal<unsigned int> fromflash {"fromflash"};
   sc_signal<unsigned int> toflash {"toflash"};
   sc_signal<bool> fromi2c {"fromi2c"};
   sc_signal<bool> toi2c {"toi2c"};

   /* Unconnected signals */
   sc_signal_resolved logic_0 {"logic_0"};

   /* Analog ADC channels are connected separately so that the GPIOs do not
    * have to handle both values.
    */
   sc_signal<float> a0 {"a0"};
   sc_signal<float> a1 {"a1"};
   sc_signal<float> a2 {"a2"};
   sc_signal<float> a3 {"a3"};
   sc_signal<float> a4 {"a4"};
   sc_signal<float> a5 {"a5"};
   sc_signal<float> a6 {"a6"};
   sc_signal<float> a7 {"a7"};
   sc_signal<float> a8 {"a8"};
   sc_signal<float> a9 {"a9"};
   sc_signal<float> a10 {"a10"};
   sc_signal<float> a11 {"a11"};
   sc_signal<float> a12 {"a12"};
   sc_signal<float> a13 {"a13"};
   sc_signal<float> a14 {"a14"};
   sc_signal<float> a15 {"a15"};
   sc_signal<float> a16 {"a16"};
   sc_signal<float> a17 {"a17"};

   /* blocks */
   doitesp32devkitv1 i_esp{"i_esp"};
   webclient i_webclient{"i_webclient"};
   uartclient i_uartclient{"i_uartclient"};

   /* Processes */
   void testbench(void);
   void serflush(void);

   /* Tests */
   unsigned int tn; /* Testcase number */
   void t0();

   // Constructor
   SC_CTOR(HelloServertest) {

      /* UART 0 - we connect the wires to the corresponding tasks. Yes, the
       * RX and TX need to be switched.
       */
      i_esp.d3(rx); i_uartclient.tx(rx);
      i_esp.d1(tx); i_uartclient.rx(tx);

      /* LED */
      i_esp.d13(led);

      /* Other interfaces, none are used so they are just left floating. */
      i_esp.wrx(fromwifi); i_webclient.tx(fromwifi);
      i_esp.wtx(towifi); i_webclient.rx(towifi);
      /* Note: these two will soon be replaced with the real flash and I2C
       * interfaces, as soon as someone takes the time to implement them. */
      i_esp.frx(fromflash); i_esp.ftx(toflash);
      i_esp.irx(fromi2c); i_esp.itx(toi2c);

      /* Pins not used in this simulation */
      i_esp.d0(logic_0); /* boot pin */
      i_esp.d2(logic_0); i_esp.d4(logic_0); i_esp.d5(logic_0);
      i_esp.d11(logic_0); i_esp.d12(logic_0);
      i_esp.d14(logic_0); i_esp.d15(logic_0); i_esp.d16(logic_0);
      i_esp.d17(logic_0); i_esp.d18(logic_0); i_esp.d19(logic_0);
      i_esp.d21(logic_0); i_esp.d22(logic_0); i_esp.d23(logic_0);
      i_esp.d25(logic_0); i_esp.d26(logic_0); i_esp.d27(logic_0);
      i_esp.d32(logic_0); i_esp.d33(logic_0); i_esp.d34(logic_0);
      i_esp.d35(logic_0); i_esp.d36(logic_0);
      i_esp.d37(logic_0); i_esp.d38(logic_0); i_esp.d39(logic_0);

      /* The analog pins we simply connect. */
      i_esp.a0(a0);  i_esp.a1(a1);
      i_esp.a2(a2);  i_esp.a3(a3);
      i_esp.a4(a4);  i_esp.a5(a5);
      i_esp.a6(a6);  i_esp.a7(a7);
      i_esp.a10(a8); i_esp.a11(a9);
      i_esp.a12(a10);i_esp.a13(a11);
      i_esp.a14(a12);i_esp.a15(a13);
      i_esp.a16(a14);i_esp.a17(a15);
      i_esp.a18(a16);i_esp.a19(a17);

      SC_THREAD(testbench);
      SC_THREAD(serflush);
   }

   void start_of_simulation();
   void trace(sc_trace_file *tf);
};
