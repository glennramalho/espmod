// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp32-hal-adc.h"
#include "adc_types.h"
#include "esp_attr.h"
#include "driver/adc.h"
#include <systemc.h>
#include "info.h"

static uint8_t __analogAttenuation = 3;//11db
static uint8_t __analogWidth = 3;//12 bits
static uint8_t __analogCycles = 8;
static uint8_t __analogSamples = 0;//1 sample
static uint8_t __analogClockDiv = 1;

// Width of returned answer ()
static uint8_t __analogReturnedWidth = 12;

void analogSetWidth(uint8_t bits){
   if(bits < 9){
       bits = 9;
   } else if(bits > 12){
      bits = 12;
   }

   __analogWidth = bits - 9;
   adc1ptr->set_width(bits);
   adc2ptr->set_width(bits);
}

void analogSetCycles(uint8_t cycles){
   PRINTF_WARN("ADC", "ADC cycles is not yet supported.");
   __analogCycles = cycles;
}

void analogSetSamples(uint8_t samples){
   PRINTF_WARN("ADC", "ADC samples is not yet supported.");
   if(!samples){
        return;
    }
    __analogSamples = samples - 1;
}

void analogSetClockDiv(uint8_t clockDiv){
   if(!clockDiv){
       return;
   }
   __analogClockDiv = clockDiv;
   adc_set_clk_div(clockDiv);
}

void analogSetAttenuation(adc_attenuation_t attenuation)
{
   adc_atten_t at;
   int c;

   switch(attenuation) {
      /* For most measures, we do a simple conversion of the enums. */
      case ADC_0db: at = ADC_ATTEN_DB_0; break;
      case ADC_2_5db: at = ADC_ATTEN_DB_2_5; break;
      case ADC_6db: at = ADC_ATTEN_DB_6; break;
      case ADC_11db: at = ADC_ATTEN_DB_11; break;
      /* If the value is illegal, we warn the user and assume that we can
       * simply drop the upper bits, as the original code from the Arduino IDF
       * does this.
       */
      default:
         at = (adc_atten_t)((unsigned int)attenuation & (~3U));
         PRINTF_WARN("ADC", "Using large Attenuation value, using %s",
            printatten(at));
         break;
   }
   /* Now we set the attenuation of every channel */
   for(c = 0; c < 8; c = c + 1)
      adc1_config_channel_atten((adc1_channel_t)c, at);
   for(c = 0; c < 10; c = c + 1)
      adc2_config_channel_atten((adc2_channel_t)c, at);
}

void IRAM_ATTR analogInit(){
    static bool initialized = false;
    if(initialized){
        return;
    }

    analogSetAttenuation((adc_attenuation_t)__analogAttenuation);
    analogSetCycles(__analogCycles);
    analogSetSamples(__analogSamples + 1);//in samples
    analogSetClockDiv(__analogClockDiv);
    analogSetWidth(__analogWidth + 9);//in bits

    /* We switch on the ADC in case it is not on. */
    adc_power_on();

    initialized = true;
}

void analogSetPinAttenuation(uint8_t pin, adc_attenuation_t attenuation)
{
    /* This one is a bit different from the above one, but we are following
     * the Arduino-IDF.
     */
    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0 || attenuation > 3){
        return ;
    }
    analogInit();
    if(channel > 7){
        adc2_config_channel_atten((adc2_channel_t)(channel-10),
           (adc_atten_t)attenuation);
    } else {
        adc1_config_channel_atten((adc1_channel_t)channel,
           (adc_atten_t)attenuation);
    }
}

bool IRAM_ATTR adcAttachPin(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    /* Not supported yet

    int8_t pad = digitalPinToTouchChannel(pin);
    if(pad >= 0){
        uint32_t touch = READ_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG);
        if(touch & (1 << pad)){
            touch &= ~((1 << (pad + SENS_TOUCH_PAD_OUTEN2_S))
                    | (1 << (pad + SENS_TOUCH_PAD_OUTEN1_S))
                    | (1 << (pad + SENS_TOUCH_PAD_WORKEN_S)));
            WRITE_PERI_REG(SENS_SAR_TOUCH_ENABLE_REG, touch);
        }
    } else if(pin == 25){
        CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);//stop dac1
    } else if(pin == 26){
        CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE);//stop dac2
    }
    */

    pinMode(pin, ANALOG);

    analogInit();
    return true;
}

bool IRAM_ATTR adcStart(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    if(channel > 9){
        channel -= 10;
        adc2ptr->soc((int)channel);
    } else {
        adc1ptr->soc((int)channel);
    }
    return true;
}

bool IRAM_ATTR adcBusy(uint8_t pin){

    int8_t channel = digitalPinToAnalogChannel(pin);
    if(channel < 0){
        return false;//not adc pin
    }

    if(channel > 7) adc2ptr->busy();
    return adc2ptr->busy();
}

uint16_t IRAM_ATTR adcEnd(uint8_t pin)
{
    int v;
    int8_t channel = digitalPinToAnalogChannel(pin);

    if(channel < 0){
        return 0;//not adc pin
    }
    if(channel > 7){
        adc2ptr->wait_eoc();
        v = adc2ptr->getraw();
        /* If it fails we return 0 */
        if (v < 0) return 0;
    } else {
        adc1ptr->wait_eoc();
        v = adc1ptr->getraw();
        /* If it fails we return 0 */
        if (v < 0) return 0;
    }
    return (uint16_t)v;
}

uint16_t IRAM_ATTR analogRead(uint8_t pin)
{
    if(!adcAttachPin(pin) || !adcStart(pin)){
        return 0;
    }
    return adcEnd(pin);
}

void analogReadResolution(uint8_t bits)
{
    if(!bits || bits > 16){
        return;
    }
    analogSetWidth(bits);         // hadware from 9 to 12
    __analogReturnedWidth = bits;   // software from 1 to 16
}

int hallRead()    //hall sensor without LNA
{
    PRINTF_ERROR("ADC", "Hall sensor not yet supported.");
    return 0;
}
