/*******************************************************************************
 * tb_gpio.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Testbench for the GPIO models.
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

#ifndef _TB_GPIO_H
#define _TB_GPIO_H

#include <systemc.h>
#include "gpio_simple.h"
#include "gpio_mix.h"
#include "gpio_mf.h"
#include "gpio_mfmix.h"
#include "gn_mixed.h"

SC_MODULE(tb_gpio) {
   public:

   sc_signal_resolved pin1 {"pin1"};
   gn_signal_mix pin_a1 {"pin_a1"};
   gn_signal_mix pin_a2 {"pin_a2"};
   sc_signal_resolved pin3 {"pin3"};

   sc_signal<bool> f1in {"f1in"}, f1out {"f1out"}, f1en {"f1en"};
   sc_signal<bool> f2in {"f2in"}, f2out {"f2out"}, f2en {"f2en"};
   sc_signal<bool> f3in {"f3in"}, f3out {"f3out"}, f3en {"f3en"};

   gpio i_gpio{"i_gpio",
      GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
      GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
      GPIOMODE_NONE};
   gpio_mix i_gpio_mix{"i_gpio_mix",
      GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
      GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
      GPIOMODE_NONE, true, GPIOMF_GPIO};
   gpio_mfmix i_gpio_mfmix{"i_gpio_mfmix",
      GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
      GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
      GPIOMODE_NONE, true, GPIOMF_GPIO};
   gpio_mf i_gpio_mf{"i_gpio_mf",
      GPIOMODE_NONE | GPIOMODE_INPUT | GPIOMODE_OUTPUT | GPIOMODE_INOUT |
      GPIOMODE_WPU | GPIOMODE_WPD | GPIOMODE_OD,
      GPIOMODE_NONE, GPIOMF_GPIO};

   /* Testenches */
   int tn;
   void testbench();
   void t0();

   void expect(const char *name, bool a, sc_logic v);
   void expect(const char *name, sc_logic a, sc_logic v);
   void expect(const char *name, gn_mixed a, sc_logic v);
   void expectfunc(const char *name, int a, int v);

   SC_CTOR(tb_gpio) {
      i_gpio.pin(pin1);
      i_gpio_mix.pin(pin_a1);

      i_gpio_mfmix.pin(pin_a2);
      /* Function 1 */
      i_gpio_mfmix.fin(f1in);
      i_gpio_mfmix.fen(f1en);
      i_gpio_mfmix.fout(f1out);
      /* Function 2 */
      i_gpio_mfmix.fin(f2in);
      i_gpio_mfmix.fen(f2en);
      i_gpio_mfmix.fout(f2out);

      i_gpio_mf.pin(pin3);
      /* Function 1 */
      i_gpio_mf.fin(f3in);
      i_gpio_mf.fen(f3en);
      i_gpio_mf.fout(f3out);

      SC_THREAD(testbench);
   }

   void trace(sc_trace_file *tf);
};

#endif
