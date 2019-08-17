/*******************************************************************************
 * gpio_periph.c -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a modified gpio_periph.c which sets all register numbers to the
 *   gpio number. It is then used by the SystemC model to set functions.
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
 *
 * This file was based off the work covered by the license below:
 *    Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "gpio_periph.h"
const uint32_t GPIO_PIN_MUX_REG[GPIO_PIN_COUNT] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    0,
    21,
    22,
    23,
    0,
    25,
    26,
    27,
    0,
    0,
    0,
    0,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
};
