/*******************************************************************************
 * esp32adc2.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include "info.h"
#include "esp32adc2.h"
#include <cmath>
#include "wifistat.h"

void esp32adc2::soc(int channel) {
   /* I could not get a good explanation of the ADC timing, so I am guessing
    * a value from a different spec. We take the 2Msps value from the spec
    * and multiply it by the width plus a 4 cycle overhead that many other ADCs
    * use.
    */
   if (!power_on) return;
   if (channel < 0 || channel > 10)
      PRINTF_WARN("ADC2",
         "Attempting to start conversion on illegal channel %d", channel);
   runningonchannel = channel;
   endtime = sc_time_stamp() + sc_time(500, SC_NS) * (width + 4);
   corrupted = false;
}

bool esp32adc2::busy() {
   if (corrupted) return false;
   if (!power_on) return false;
   /* If the WiFi has been initialized, we loose access to this ADC. */
   if (wifistat.initialized) return true;
   if (endtime == SC_ZERO_TIME) return false;
   if (runningonchannel == -1) return false;
   return sc_time_stamp() >= endtime;
}

void esp32adc2::wait_eoc() {
   if (!busy()) return;
   else wait(endtime - sc_time_stamp());
}

int esp32adc2::getraw() {
   int bitmask;
   float top, low, step, raw;
   float sample;

   if (busy()) return -1;
   if (corrupted) return -1;
   if (!power_on) return false;
   /* If the wifi is on nothing can be measured. */
   if (wifistat.initialized) return -1;
   switch(runningonchannel) {
      case 0: sample = channel_0->read().lvl; break;
      case 1: sample = channel_1->read().lvl; break;
      case 2: sample = channel_2->read().lvl; break;
      case 3: sample = channel_3->read().lvl; break;
      case 4: sample = channel_4->read().lvl; break;
      case 5: sample = channel_5->read().lvl; break;
      case 6: sample = channel_6->read().lvl; break;
      case 7: sample = channel_7->read().lvl; break;
      case 8: sample = channel_8->read().lvl; break;
      case 9: sample = channel_9->read().lvl; break;
      default: return -1;
   }

   /* Now we need to convert it to raw. */
   switch (atten[runningonchannel]) {
      case ADC_ATTEN_DB_0: top = 1.1; low = 0.0; break;
      case ADC_ATTEN_DB_2_5: top = 1.5; low = 0.0; break;
      case ADC_ATTEN_DB_6: top = 2.2; low = 0.0; break;
      default: top = 3.9; low = 0.0; break;
   }
   
   /* We need to cap the results, the ADC will never give a value outside
    * the valid range. So, if we get something above the top, it wont
    * necessarily destroy the ADC, but it will not pass the all ones
    * result. Also, if you give something below the minimum it will never
    * return a negative value.
    */
   bitmask = (1 << width) - 1;
   if (sample >= top) return bitmask;
   if (sample <= low) return 0;

   /* Now, we know we have something within the range. We then need to find
    * out what is our ADC step so that we can calculate the result. Remember,
    * the ADC is linear so the steps shouls be equally spaced.
    */
   step = (top - low)/bitmask;

   /* Once we have the step, all we do is convert the sampled value into the
    * number of steps to represent it. We then round it down and return the
    * resulting value.
    */
   raw = trunc((sample - low)/step);
   return (int)raw;
}

int esp32adc2::get_width() { return width; }

void esp32adc2::set_width(int _w) {
   if (_w < 9 || _w > 12)
      PRINTF_WARN("ADC2", "Attempting to set illegal width %d", _w);
   if (busy()) {
      PRINTF_WARN("ADC2",
         "Attempting to change width while the ADC is running");
      corrupted = true;
   }
   width = _w;
}

adc_atten_t esp32adc2::get_atten(int channel) {
   if (channel < 0 || channel > 9)
      PRINTF_WARN("ADC2",
         "Attempting to set attenuation of illegal channel %d", channel);
   return atten[channel];
}

void esp32adc2::set_atten(int channel, adc_atten_t _a) {
   if (channel < 0 || channel > 9)
      PRINTF_WARN("ADC2",
         "Attempting to set attenuation of illegal channel %d", channel);
   if ((int)_a < 0 || _a >= ADC_ATTEN_MAX)
      PRINTF_WARN("ADC2",
         "Attempting to illegal attenuation to channel %d", channel);
   if (busy() && runningonchannel == channel) {
      PRINTF_WARN("ADC2",
         "Attempting to change attenuation while the ADC is running");
      corrupted = true;
   }
   atten[channel] = _a;
}

void esp32adc2::switchon(bool on) {
   corrupted = true;
   endtime = SC_ZERO_TIME;
   power_on = on;
}
