/*******************************************************************************
 * esp32adc2.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of a ESP32 ADC.
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

#ifndef _ESP32ADC2_H
#define _ESP32ADC2_H

#include <systemc.h>
#include "driver/adc.h"
#include "gn_mixed.h"

class esp32adc2 : public sc_module {
   public:
   sc_in<gn_mixed> channel_0 {"channel_0"};
   sc_in<gn_mixed> channel_1 {"channel_1"};
   sc_in<gn_mixed> channel_2 {"channel_2"};
   sc_in<gn_mixed> channel_3 {"channel_3"};
   sc_in<gn_mixed> channel_4 {"channel_4"};
   sc_in<gn_mixed> channel_5 {"channel_5"};
   sc_in<gn_mixed> channel_6 {"channel_6"};
   sc_in<gn_mixed> channel_7 {"channel_7"};
   sc_in<gn_mixed> channel_8 {"channel_8"};
   sc_in<gn_mixed> channel_9 {"channel_9"};

   private:
   sc_time endtime;
   adc_atten_t atten[10];
   int width;
   int runningonchannel;
   bool power_on;
   bool corrupted;

   public:
   void soc(int channel);
   bool busy();
   void wait_eoc();
   int getraw();
   void set_width(int width);
   int get_width();
   void set_atten(int channel, adc_atten_t atten);
   adc_atten_t get_atten(int channel);
   void switchon(bool on);

   SC_CTOR(esp32adc2) {
      atten[0] = ADC_ATTEN_DB_0;
      atten[1] = ADC_ATTEN_DB_0;
      atten[2] = ADC_ATTEN_DB_0;
      atten[3] = ADC_ATTEN_DB_0;
      atten[4] = ADC_ATTEN_DB_0;
      atten[5] = ADC_ATTEN_DB_0;
      atten[6] = ADC_ATTEN_DB_0;
      atten[7] = ADC_ATTEN_DB_0;
      atten[8] = ADC_ATTEN_DB_0;
      atten[9] = ADC_ATTEN_DB_0;
      runningonchannel = -1;
      endtime = SC_ZERO_TIME;
      corrupted = true;
      power_on = true; /* Is this correct? Default on? TODO */
   }
};

#endif
