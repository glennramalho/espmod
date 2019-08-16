/*******************************************************************************
 * adc_types.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Defines types of ESP32 ADC being used.
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

#ifndef _ADC_TYPES_H
#define _ADC_TYPES_H

#include <systemc.h>
#include "esp32adc1.h"
#include "esp32adc2.h"
typedef esp32adc1 adc1;
typedef esp32adc2 adc2;
extern adc1 *adc1ptr;
extern adc2 *adc2ptr;

#endif
