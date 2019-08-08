/*******************************************************************************
 * tb_gpio.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "tb_gpio.h"
#include "info.h"

void tb_gpio::expect(const char *name, bool a, sc_logic v) {
   if (a != v) PRINTF_WARN("TEST", "Expected %s to be %c but got %c",
      name, v.to_char(), (a)?'1':'0')
   else PRINTF_INFO("TEST", "SUCCESS: got %s to be %c", name, v.to_char());
}

void tb_gpio::expect(const char *name, sc_logic a, sc_logic v) {
   if (a != v) PRINTF_WARN("TEST", "Expected %s to be %c but got %c",
      name, v.to_char(), a.to_char())
   else PRINTF_INFO("TEST", "SUCCESS: got %s to be %c", name, v.to_char());
}

void tb_gpio::expect(const char *name, gn_mixed a, sc_logic v) {
   if (a != v) PRINTF_WARN("TEST", "Expected %s to be %c but got %c",
      name, v.to_char(), a.to_char())
   else PRINTF_INFO("TEST", "SUCCESS: got %s to be %c", name, v.to_char());
}

void tb_gpio::expectfunc(const char *name, int a, int v) {
   if (a != v)
      PRINTF_WARN("TEST", "Expected %s to be Function %d but got %c",name,v,a)
   else PRINTF_INFO("TEST", "SUCCESS: got %s to be Function %d", name, v);
}

void tb_gpio::t0(void) {
   wait(SC_ZERO_TIME);

   /* We start off checking the pins. */
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_Z);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_Z);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_Z);
   expect(pin3.name(), pin3.read(), SC_LOGIC_Z);
   expectfunc(i_gpio.name(), i_gpio.get_function(), GPIOMF_GPIO);
   expectfunc(i_gpio_mix.name(), i_gpio_mix.get_function(), GPIOMF_GPIO);
   expectfunc(i_gpio_mfmix.name(), i_gpio_mfmix.get_function(), GPIOMF_GPIO);
   expectfunc(i_gpio_mf.name(), i_gpio_mf.get_function(), GPIOMF_GPIO);

   /* We now change the GPIOs to be Output and check the outputs. */
   wait(10, SC_NS); printf("\n\n**** Changing GPIOs to Output ****\n");
   i_gpio.set_dir(GPIODIR_OUTPUT);
   i_gpio_mix.set_dir(GPIODIR_OUTPUT);
   i_gpio_mfmix.set_dir(GPIODIR_OUTPUT);
   i_gpio_mf.set_dir(GPIODIR_OUTPUT);
   wait(SC_ZERO_TIME);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_0);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0);
   expect(pin3.name(), pin3.read(), SC_LOGIC_0);
   /* We drive something and check it. */
   wait(10, SC_NS);
   i_gpio.set_val(true);
   i_gpio_mix.set_val(true);
   i_gpio_mfmix.set_val(true);
   i_gpio_mf.set_val(true);
   wait(SC_ZERO_TIME);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_1);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_1);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_1);
   expect(pin3.name(), pin3.read(), SC_LOGIC_1);

   /* We change them to WPU or OD and recheck it. */
   printf("\n\n**** Checking weak pull-up/OD ****\n");
   i_gpio.set_wpu();
   i_gpio_mix.set_od();
   i_gpio_mfmix.set_wpu();
   i_gpio_mf.set_wpu();
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_Z); /* WPU but looks like Z */
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_Z); /* OD */
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_1); /* WPU */
   expect(pin3.name(), pin3.read(), SC_LOGIC_Z); /* WPU but looks like Z */
   /* And we go low. */
   i_gpio.set_val(false);
   i_gpio_mix.set_val(false);
   i_gpio_mfmix.set_val(false);
   i_gpio_mf.set_val(false);
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_0);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0);
   expect(pin3.name(), pin3.read(), SC_LOGIC_0);
   /* Now we switch the OD and WPU pins and try it again. */
   printf("\n\n**** Switching weak pull-up and OD ****\n");
   i_gpio.set_val(true);
   i_gpio_mix.set_val(true);
   i_gpio_mfmix.set_val(true);
   i_gpio_mf.set_val(true);
   i_gpio.clr_wpu(); i_gpio.set_od();
   i_gpio_mix.clr_od(); i_gpio_mix.set_wpu();
   i_gpio_mfmix.clr_wpu(); i_gpio_mfmix.set_od();
   i_gpio_mf.clr_wpu(); i_gpio_mf.set_od();
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_Z); /* OD */
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_1); /* WPU */
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_Z); /* OD */
   expect(pin3.name(), pin3.read(), SC_LOGIC_Z); /* OD */
   /* And we go low again. */
   i_gpio.set_val(false);
   i_gpio_mix.set_val(false);
   i_gpio_mfmix.set_val(false);
   i_gpio_mf.set_val(false);
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_0);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0);
   expect(pin3.name(), pin3.read(), SC_LOGIC_0);

   /* Now we go with a weak pull-down. */
   printf("\n\n**** Checking weak pull-down ****\n");
   i_gpio.set_wpd(); i_gpio.clr_od();
   i_gpio_mix.set_wpd(); i_gpio_mix.clr_wpu();
   i_gpio_mfmix.set_wpd(); i_gpio_mfmix.clr_od();
   i_gpio_mf.set_wpd(); i_gpio_mf.clr_od();
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_Z); /* WPD but looks like Z */
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0); /* WPD */
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0); /* WPD */
   expect(pin3.name(), pin3.read(), SC_LOGIC_Z); /* WPD but looks like Z */
   /* And we raise the signals. */
   i_gpio.set_val(true);
   i_gpio_mix.set_val(true);
   i_gpio_mfmix.set_val(true);
   i_gpio_mf.set_val(true);
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_1);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_1);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_1);
   expect(pin3.name(), pin3.read(), SC_LOGIC_1);
   /* Now we go with a weak pull-down. */
   i_gpio.set_val(false);
   i_gpio_mix.set_val(false);
   i_gpio_mfmix.set_val(false);
   i_gpio_mf.set_val(false);
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_Z);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0);
   expect(pin3.name(), pin3.read(), SC_LOGIC_Z);
   /* And we shut off the pull downs. */
   i_gpio.clr_wpd();
   i_gpio_mix.clr_wpd();
   i_gpio_mfmix.clr_wpd();
   i_gpio_mf.clr_wpd();
   wait(10, SC_NS);
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);
   expect(pin1.name(), pin1.read(), SC_LOGIC_0);
   expect(pin_a1.name(), pin_a1.read(), SC_LOGIC_0);
   expect(pin_a2.name(), pin_a2.read(), SC_LOGIC_0);
   expect(pin3.name(), pin3.read(), SC_LOGIC_0);

   /* Now we go to GPIO In */
   printf("\n\n**** Changing GPIOs to Input ****\n");
   pin1.write(SC_LOGIC_1);
   pin_a1.write(SC_LOGIC_0);
   pin_a2.write(SC_LOGIC_1);
   pin3.write(SC_LOGIC_0);
   i_gpio.set_dir(GPIODIR_INPUT);
   i_gpio_mix.set_dir(GPIODIR_INPUT);
   i_gpio_mfmix.set_dir(GPIODIR_INPUT);
   i_gpio_mf.set_dir(GPIODIR_INPUT);
   wait(10, SC_NS);
   expect(i_gpio.name(), i_gpio.get_val(), SC_LOGIC_1);
   expect(i_gpio_mix.name(), i_gpio_mix.get_val(), SC_LOGIC_0);
   expect(i_gpio_mfmix.name(), i_gpio_mfmix.get_val(), SC_LOGIC_1);
   expect(i_gpio_mf.name(), i_gpio_mf.get_val(), SC_LOGIC_0);
   pin1.write(SC_LOGIC_0);
   pin_a1.write(SC_LOGIC_1);
   pin_a2.write(SC_LOGIC_0);
   pin3.write(SC_LOGIC_1);
   wait(10, SC_NS);
   expect(pin1.name(), i_gpio.get_val(), SC_LOGIC_0);
   expect(pin_a1.name(), i_gpio_mix.get_val(), SC_LOGIC_1);
   expect(pin_a2.name(), i_gpio_mfmix.get_val(), SC_LOGIC_0);
   expect(pin3.name(), i_gpio_mf.get_val(), SC_LOGIC_1);
   /* These one should still be low as the function was not selected. */
   expect(f1out.name(), f1out.read(), SC_LOGIC_0);
   expect(f2out.name(), f2out.read(), SC_LOGIC_0);
   expect(f3out.name(), f3out.read(), SC_LOGIC_0);

   /* Now we change values. */
   printf("\n\n**** Changing GPIOs to I/O ****\n");
   pin1.write(SC_LOGIC_1);
   pin_a1.write(SC_LOGIC_0);
   pin_a2.write(SC_LOGIC_1);
   pin3.write(SC_LOGIC_0);
   i_gpio.set_dir(GPIODIR_INOUT); i_gpio.set_val(true);
   i_gpio_mix.set_dir(GPIODIR_INOUT); i_gpio_mix.set_val(true);
   i_gpio_mfmix.set_dir(GPIODIR_INOUT); i_gpio_mfmix.set_val(true);
   i_gpio_mf.set_dir(GPIODIR_INOUT); i_gpio_mf.set_val(true);
   wait(10, SC_NS);
   expect(pin1.name(), pin1, SC_LOGIC_1);
   expect(pin_a1.name(), pin_a1, SC_LOGIC_X);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_1); /* It was high. */
   expect(pin3.name(), pin3, SC_LOGIC_X);
   pin1.write(SC_LOGIC_Z);
   pin_a1.write(SC_LOGIC_Z);
   pin_a2.write(SC_LOGIC_Z);
   pin3.write(SC_LOGIC_Z);
   wait(10, SC_NS);
   expect(pin1.name(), pin1, sc_logic(i_gpio.get_val()));
   expect(pin_a1.name(), pin_a1, sc_logic(i_gpio.get_val()));
   expect(pin_a2.name(), pin_a2, sc_logic(i_gpio.get_val()));
   expect(pin3.name(), pin3, sc_logic(i_gpio.get_val()));

   /* Now we change the modes. */
   printf("\n\n**** Function ANALOG ****\n");
   i_gpio_mix.set_function(GPIOMF_ANALOG);
   i_gpio_mfmix.set_function(GPIOMF_ANALOG);
   wait(10, SC_NS);
   expect(pin1.name(), pin1, sc_logic(i_gpio.get_val()));
   expect(pin_a1.name(), pin_a1, SC_LOGIC_Z);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_Z);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_0);

   printf("\n\n**** Funtion GPIO ****\n");
   /* We drive all functions high and the internal values low and check
    * that the pins follow signal.
    */
   i_gpio_mfmix.set_function(GPIOMF_GPIO);
   i_gpio_mfmix.set_dir(GPIODIR_INOUT);
   i_gpio_mfmix.set_val(false);
   i_gpio_mf.set_function(GPIOMF_GPIO);
   i_gpio_mf.set_dir(GPIODIR_INOUT);
   i_gpio_mf.set_val(false);
   f1in.write(true);
   f1en.write(true);
   f2in.write(true);
   f2en.write(true);
   f3in.write(true);
   f3en.write(true);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_0);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   expect(pin3.name(), pin3, SC_LOGIC_0);
   expect(f3out.name(), f3out, SC_LOGIC_0);
   /* We toggle a bit more the pins */
   f2in.write(false);
   f2en.write(true);
   f3in.write(false);
   f3en.write(true);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_0);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   expect(pin3.name(), pin3, SC_LOGIC_0);
   expect(f3out.name(), f3out, SC_LOGIC_0);
   printf("\n\n**** Funtion 1 ****\n");
   /* We now switch to function 1 and check that the functions are passed
    * through.
    */
   i_gpio_mfmix.set_function(1);
   i_gpio_mf.set_function(1);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_1);
   expect(f1out.name(), f1out, SC_LOGIC_1);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   expect(pin3.name(), pin3, SC_LOGIC_0);
   expect(f3out.name(), f3out, SC_LOGIC_0);
   f2in.write(true);
   f2en.write(true);
   f3in.write(true);
   f3en.write(true);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_1);
   expect(f1out.name(), f1out, SC_LOGIC_1);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   expect(f3out.name(), f3out, SC_LOGIC_1);
   f2in.write(false);
   f2en.write(true);
   f3in.write(true);
   f3en.write(false);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_1);
   expect(pin3.name(), pin3, SC_LOGIC_Z);
   expect(f1out.name(), f1out, SC_LOGIC_1);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   expect(f3out.name(), f3out, SC_LOGIC_0); /* Z can't be put on wire f3out.*/
   printf("\n\n**** Funtion 2 ****\n");
   i_gpio_mfmix.set_function(2);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_0);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   f2in.write(true);
   f2en.write(false);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_Z);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_0);
   f2in.write(true);
   f2en.write(true);
   wait(10, SC_NS);
   expect(pin_a2.name(), pin_a2, SC_LOGIC_1);
   expect(f1out.name(), f1out, SC_LOGIC_0);
   expect(f2out.name(), f2out, SC_LOGIC_1);
}

void tb_gpio::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   PRINTF_INFO("TEST", "Starting Testbench Test%d", tn);

   if (tn == 0) t0();
   else SC_REPORT_FATAL("TEST", "Test number too large.");

   sc_stop();
}

void tb_gpio::trace(sc_trace_file *tf) {
   sc_trace(tf, pin1, pin1.name());
   sc_trace(tf, pin_a1, pin_a1.name());
   sc_trace(tf, pin_a2, pin_a2.name());
   sc_trace(tf, pin3, pin3.name());
   sc_trace(tf, f1in, f1in.name());
   sc_trace(tf, f1en, f1en.name());
   sc_trace(tf, f1out, f1out.name());
   sc_trace(tf, f2in, f2in.name());
   sc_trace(tf, f2en, f2en.name());
   sc_trace(tf, f2out, f2out.name());
   sc_trace(tf, f3in, f3in.name());
   sc_trace(tf, f3en, f3en.name());
   sc_trace(tf, f3out, f3out.name());
}
