/*******************************************************************************
 * update.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Functions to notify the modules that the IO structures have changed.
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

#include "gpio_matrix.h"
#include "pcntmod.h"
#include "adc_types.h"
#include "soc/gpio_struct.h"
#include "soc/pcnt_struct.h"

gpio_matrix *gpiomatrixptr;
pcntmod *pcntptr;
adc1 *adc1ptr;
adc2 *adc2ptr;
gpio_dev_t GPIO;
pcnt_dev_t PCNT;

void update_pcnt() {
   pcntptr->update();
}

void update_gpio() {
   gpiomatrixptr->update();
}

void update_gpio_reg() {
   gpiomatrixptr->updategpioreg();
}

void update_gpio_oe() {
   gpiomatrixptr->updategpiooe();
}