/*******************************************************************************
 * adc.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the adc functions for the ESP32 to the ESPMOD SystemC
 *   model. It was based off the functions from Espressif Systems.
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

#include <systemc.h>
#include "info.h"
#include "esp32-hal-gpio.h"
#include "driver/adc.h"
#include "adc_types.h"

const char *printadcunit(adc_unit_t adc_unit) {
   if (adc_unit == ADC_UNIT_1) return "1";
   if (adc_unit == ADC_UNIT_2) return "2";
   if (adc_unit == ADC_UNIT_BOTH) return "1 & 2";
   if (adc_unit == ADC_UNIT_ALTER) return "alternative";
   return "UNK";
}

const char *printatten(adc_atten_t atten) {
   if (atten == ADC_ATTEN_DB_0) return "0dB";
   if (atten == ADC_ATTEN_DB_2_5) return "2.5dB";
   if (atten == ADC_ATTEN_DB_6) return "6dB";
   if (atten == ADC_ATTEN_DB_11) return "11dB";
   return "UNK DB";
}

int getadcwidth(adc_bits_width_t width) {
   switch(width) {
      case ADC_WIDTH_BIT_9: return 9;
      case ADC_WIDTH_BIT_10: return 10;
      case ADC_WIDTH_BIT_11: return 11;
      case ADC_WIDTH_BIT_12: return 12;
      default: return -1;
   }
}

/**
 * @brief Get the gpio number of a specific ADC1 channel.
 * 
 * @param channel Channel to get the gpio number
 * 
 * @param gpio_num output buffer to hold the gpio number
 * 
 * @return 
 *   - ESP_OK if success
 *   - ESP_ERR_INVALID_ARG if channal not valid 
 */
esp_err_t adc1_pad_get_io_num(adc1_channel_t channel, gpio_num_t *gpio_num) {
   int pin;
   if (channel >= ADC1_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (gpio_num == NULL) return ESP_ERR_INVALID_ARG;
   for(pin = 0; pin < GPIO_PIN_COUNT; pin = pin + 1) {
      if (esp32_gpioMux[pin].adc == (int)channel) {
         *gpio_num = (gpio_num_t)pin;
         return ESP_OK;
      }
   }
   return ESP_ERR_INVALID_ARG;
}

/**
 * @brief Configure ADC1 capture width, meanwhile enable output invert for ADC1.
 * The configuration is for all channels of ADC1
 * @param width_bit Bit capture width for ADC1
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc1_config_width(adc_bits_width_t width_bit) {
   if (width_bit >= ADC_WIDTH_MAX) return ESP_ERR_INVALID_ARG;
   if (adc1ptr == NULL) return ESP_ERR_INVALID_ARG;
   PRINTF_INFO("ADC", "Configuring ADC1 to %d bits", getadcwidth(width_bit));
   adc1ptr->set_width(getadcwidth(width_bit));
   return ESP_OK;
}

/**
 * @brief Configure ADC capture width.
 * @param adc_unit ADC unit index
 * @param width_bit Bit capture width for ADC unit.
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc_set_data_width(adc_unit_t adc_unit, adc_bits_width_t width_bit) {
   if (adc_unit >= ADC_UNIT_MAX) return ESP_ERR_INVALID_ARG;
   if (width_bit >= ADC_WIDTH_MAX) return ESP_ERR_INVALID_ARG;
   if ((adc_unit == ADC_UNIT_1 || adc_unit == ADC_UNIT_BOTH) && adc1ptr == NULL)
      return ESP_ERR_INVALID_ARG;
   if ((adc_unit == ADC_UNIT_2 || adc_unit == ADC_UNIT_BOTH) && adc2ptr == NULL)
      return ESP_ERR_INVALID_ARG;
   PRINTF_INFO("Configuring ADC %s to capture %d bits",
      printadcunit(adc_unit), getadcwidth(width_bit));
   if (adc_unit == ADC_UNIT_1 || adc_unit == ADC_UNIT_BOTH)
      adc1ptr->set_width(getadcwidth(width_bit));
   if (adc_unit == ADC_UNIT_2 || adc_unit == ADC_UNIT_BOTH)
      adc2ptr->set_width(getadcwidth(width_bit));
   return ESP_OK;
}

/**
 * @brief Set the attenuation of a particular channel on ADC1, and configure its
 * associated GPIO pin mux.
 *
 * @note For any given channel, this function must be called before the first time
 * adc1_get_raw() is called for that channel.
 *
 * @note This function can be called multiple times to configure multiple
 * ADC channels simultaneously. adc1_get_raw() can then be called for any configured
 * channel.
 *
 * The default ADC full-scale voltage is 1.1V. To read higher voltages (up to the pin maximum voltage,
 * usually 3.3V) requires setting >0dB signal attenuation for that ADC channel.
 *
 * When VDD_A is 3.3V:
 *
 * - 0dB attenuaton (ADC_ATTEN_DB_0) gives full-scale voltage 1.1V
 * - 2.5dB attenuation (ADC_ATTEN_DB_2_5) gives full-scale voltage 1.5V
 * - 6dB attenuation (ADC_ATTEN_DB_6) gives full-scale voltage 2.2V
 * - 11dB attenuation (ADC_ATTEN_DB_11) gives full-scale voltage 3.9V (see note below)
 *
 * @note The full-scale voltage is the voltage corresponding to a maximum reading (depending on ADC1 configured
 * bit width, this value is: 4095 for 12-bits, 2047 for 11-bits, 1023 for 10-bits, 511 for 9 bits.)
 *
 * @note At 11dB attenuation the maximum voltage is limited by VDD_A, not the full scale voltage.
 *
 * Due to ADC characteristics, most accurate results are obtained within the following approximate voltage ranges:
 *
 * - 0dB attenuaton (ADC_ATTEN_DB_0) between 100 and 950mV
 * - 2.5dB attenuation (ADC_ATTEN_DB_2_5) between 100 and 1250mV
 * - 6dB attenuation (ADC_ATTEN_DB_6) between 150 to 1750mV
 * - 11dB attenuation (ADC_ATTEN_DB_11) between 150 to 2450mV
 *
 * For maximum accuracy, use the ADC calibration APIs and measure voltages within these recommended ranges.
 *
 * @param channel ADC1 channel to configure
 * @param atten  Attenuation level
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc1_config_channel_atten(adc1_channel_t channel, adc_atten_t atten) {
   if (channel >= ADC1_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (adc1ptr == NULL) return ESP_ERR_INVALID_ARG;
   PRINTF_INFO("ADC", "Setting ADC1 channel %0d to %s", channel,
      printatten(atten));
   adc1ptr->set_atten(channel, atten);
   return ESP_OK;
}

/**
 * @brief Take an ADC1 reading from a single channel.
 * @note When the power switch of SARADC1, SARADC2, HALL sensor and AMP sensor is turned on,
 *       the input of GPIO36 and GPIO39 will be pulled down for about 80ns.
 *       When enabling power for any of these peripherals, ignore input from GPIO36 and GPIO39.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *
 * @note Call adc1_config_width() before the first time this
 * function is called.
 *
 * @note For any given channel, adc1_config_channel_atten(channel)
 * must be called before the first time this function is called. Configuring
 * a new channel does not prevent a previously configured channel from being read.
 *
 * @param  channel ADC1 channel to read
 *
 * @return
 *     - -1: Parameter error
 *     -  Other: ADC1 channel reading.
 */
int adc1_get_raw(adc1_channel_t channel) {
   if (channel >= ADC1_CHANNEL_MAX) return -1;
   if (adc1ptr == NULL) return -1;
   /* Usually ADCs can't switch straight from one conversion to another.
    * We assume this one can't.
    */
   if (adc1ptr->busy()) return -1;

   adc1ptr->soc((int)channel);
   adc1ptr->wait_eoc();
   return adc1ptr->getraw();
}

/** @cond */    //Doxygen command to hide deprecated function from API Reference
/*
 * @note When the power switch of SARADC1, SARADC2, HALL sensor and AMP sensor is turned on,
 *       the input of GPIO36 and GPIO39 will be pulled down for about 80ns.
 *       When enabling power for any of these peripherals, ignore input from GPIO36 and GPIO39.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *       
 * @deprecated This function returns an ADC1 reading but is deprecated due to
 * a misleading name and has been changed to directly call the new function.
 * Use the new function adc1_get_raw() instead
 */
int adc1_get_voltage(adc1_channel_t channel) {
   if (channel >= ADC1_CHANNEL_MAX) return -1;
   SC_REPORT_WARNING("ADC", "Using deprecated adc1_get_voltage");
   return adc1_get_raw(channel);
}
/** @endcond */

/**
 * @brief Enable ADC power
 */
void adc_power_on() {
   SC_REPORT_INFO("ADC", "Switching ADCs ON");
   if (adc1ptr != NULL) adc1ptr->switchon(true);
   if (adc2ptr != NULL) adc2ptr->switchon(true);
}

/**
 * @brief Power off SAR ADC
 * This function will force power down for ADC
 */
void adc_power_off() {
   SC_REPORT_INFO("ADC", "Switching ADCs OFF");
   if (adc1ptr != NULL) adc1ptr->switchon(false);
   if (adc2ptr != NULL) adc2ptr->switchon(false);
}

/**
 * @brief Initialize ADC pad
 * @param adc_unit ADC unit index
 * @param channel ADC channel index
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc_gpio_init(adc_unit_t adc_unit, adc_channel_t channel) {
   if (adc_unit >= ADC_UNIT_MAX) return ESP_ERR_INVALID_ARG;
   if (adc_unit != ADC_UNIT_1 && adc_unit != ADC_UNIT_2) {
      PRINTF_INFO("ADC", "ADC unit %s not supported", printadcunit(adc_unit));
      return ESP_ERR_INVALID_ARG;
   }
   if (channel >= ADC_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (adc_unit == ADC_UNIT_1 || adc_unit == ADC_UNIT_BOTH)
   PRINTF_INFO("ADC", "Initializing ADC %s channel %d",
      printadcunit(adc_unit), channel);
   return ESP_OK;
}

/**
 * @brief Set ADC data invert
 * @param adc_unit ADC unit index
 * @param inv_en whether enable data invert
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc_set_data_inv(adc_unit_t adc_unit, bool inv_en) {
   if (adc_unit >= ADC_UNIT_MAX) return ESP_ERR_INVALID_ARG;
   PRINTF_INFO("ADC", "Setting ADC %s to %s", printadcunit(adc_unit),
      (inv_en)?"Invert":"non-invert");
   PRINTF_INFO("ADC", "ADC inversion not yet supported.");
   return ESP_OK;
}

/**
 * @brief Set ADC source clock
 * @param clk_div ADC clock divider, ADC clock is divided from APB clock
 * @return
 *     - ESP_OK success
 */
esp_err_t adc_set_clk_div(uint8_t clk_div) {
   PRINTF_INFO("ADC", "Setting ADC clock divider to %d", clk_div);
   PRINTF_INFO("ADC", "ADC clock division not yet supported.");
   return ESP_OK;
}

/**
 * @brief Set I2S data source
 * @param src I2S DMA data source, I2S DMA can get data from digital signals or from ADC.
 * @return
 *     - ESP_OK success
 */
esp_err_t adc_set_i2s_data_source(adc_i2s_source_t src) {
   PRINTF_INFO("Setting ADC I2C Data source %s",
      (src == ADC_I2S_DATA_SRC_IO_SIG)?"I2S data from GPIO Matrix signal":
      (src == ADC_I2S_DATA_SRC_ADC)?"I2S data from ADC":"UNK");
   PRINTF_INFO("ADC", "ADC data source not yet supported.");
   return ESP_OK;
}

/**
 * @brief Initialize I2S ADC mode
 * @param adc_unit ADC unit index
 * @param channel ADC channel index
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc_i2s_mode_init(adc_unit_t adc_unit, adc_channel_t channel) {
   if (adc_unit >= ADC_UNIT_MAX) return ESP_ERR_INVALID_ARG;
   if (channel >= ADC_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;

   PRINTF_INFO("Initializing ADC%s Channel %d in I2S/ADC mode",
      printadcunit(adc_unit), channel);
   PRINTF_INFO("ADC", "ADC I2S mode not yet supported.");
   return ESP_OK;
}

/**
 * @brief Configure ADC1 to be usable by the ULP
 *
 * This function reconfigures ADC1 to be controlled by the ULP.
 * Effect of this function can be reverted using adc1_get_raw function.
 *
 * Note that adc1_config_channel_atten, adc1_config_width functions need
 * to be called to configure ADC1 channels, before ADC1 is used by the ULP.
 */
void adc1_ulp_enable() {
   PRINTF_INFO("ADC", "Setting ADC to be usable by the ULP");
   PRINTF_INFO("ADC", "ULP mode not yet supported.");
}

/**
 * @brief Read Hall Sensor
 *
 * @note When the power switch of SARADC1, SARADC2, HALL sensor and AMP sensor is turned on,
 *       the input of GPIO36 and GPIO39 will be pulled down for about 80ns.
 *       When enabling power for any of these peripherals, ignore input from GPIO36 and GPIO39.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *
 * @note The Hall Sensor uses channels 0 and 3 of ADC1. Do not configure
 * these channels for use as ADC channels.
 *
 * @note The ADC1 module must be enabled by calling
 *       adc1_config_width() before calling hall_sensor_read(). ADC1
 *       should be configured for 12 bit readings, as the hall sensor
 *       readings are low values and do not cover the full range of the
 *       ADC.
 *
 * @return The hall sensor reading.
 */
int hall_sensor_read() {
   SC_REPORT_INFO("ADC", "Using HALL sensor, not yet supported");
   return rand(); /* TODO put a value */
}

/**
 * @brief Get the gpio number of a specific ADC2 channel.
 * 
 * @param channel Channel to get the gpio number
 * 
 * @param gpio_num output buffer to hold the gpio number
 * 
 * @return 
 *   - ESP_OK if success
 *   - ESP_ERR_INVALID_ARG if channal not valid 
 */
esp_err_t adc2_pad_get_io_num(adc2_channel_t channel, gpio_num_t *gpio_num) {
   int pin;
   if (channel >= ADC2_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (gpio_num == NULL) return ESP_ERR_INVALID_ARG;
   for(pin = 0; pin < GPIO_PIN_COUNT; pin = pin + 1) {
      if (esp32_gpioMux[pin].adc-8 == (int)channel) {
         *gpio_num = (gpio_num_t)pin;
         return ESP_OK;
      }
   }
   return ESP_ERR_INVALID_ARG;
}

/**
 * @brief Configure the ADC2 channel, including setting attenuation.
 *
 * @note This function also configures the input GPIO pin mux to
 * connect it to the ADC2 channel. It must be called before calling
 * ``adc2_get_raw()`` for this channel.
 *
 * The default ADC full-scale voltage is 1.1V. To read higher voltages (up to the pin maximum voltage,
 * usually 3.3V) requires setting >0dB signal attenuation for that ADC channel.
 *
 * When VDD_A is 3.3V:
 *
 * - 0dB attenuaton (ADC_ATTEN_0db) gives full-scale voltage 1.1V
 * - 2.5dB attenuation (ADC_ATTEN_2_5db) gives full-scale voltage 1.5V
 * - 6dB attenuation (ADC_ATTEN_6db) gives full-scale voltage 2.2V
 * - 11dB attenuation (ADC_ATTEN_11db) gives full-scale voltage 3.9V (see note below)
 *
 * @note The full-scale voltage is the voltage corresponding to a maximum reading 
 * (depending on ADC2 configured bit width, this value is: 4095 for 12-bits, 2047 
 * for 11-bits, 1023 for 10-bits, 511 for 9 bits.)
 *
 * @note At 11dB attenuation the maximum voltage is limited by VDD_A, not the full scale voltage.
 *
 * @param channel ADC2 channel to configure
 * @param atten  Attenuation level
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t adc2_config_channel_atten(adc2_channel_t channel, adc_atten_t atten) {
   if (channel >= ADC2_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (adc2ptr == NULL) return ESP_ERR_INVALID_ARG;
   PRINTF_INFO("ADC", "Setting ADC2 channel %0d to %s", channel,
      printatten(atten));
   adc2ptr->set_atten(channel, atten);

   return ESP_OK;
}

/**
 * @brief Take an ADC2 reading on a single channel
 *
 * @note When the power switch of SARADC1, SARADC2, HALL sensor and AMP sensor is turned on,
 *       the input of GPIO36 and GPIO39 will be pulled down for about 80ns.
 *       When enabling power for any of these peripherals, ignore input from GPIO36 and GPIO39.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *
 * @note For a given channel, ``adc2_config_channel_atten()``
 * must be called before the first time this function is called. If Wi-Fi is started via ``esp_wifi_start()``, this
 * function will always fail with ``ESP_ERR_TIMEOUT``.
 *
 * @param  channel ADC2 channel to read
 * 
 * @param width_bit Bit capture width for ADC2
 * 
 * @param raw_out the variable to hold the output data.
 *
 * @return
 *     - ESP_OK if success
 *     - ESP_ERR_TIMEOUT the WIFI is started, using the ADC2
 */
esp_err_t adc2_get_raw(adc2_channel_t channel, adc_bits_width_t width_bit, int* raw_out) {
   if (channel > ADC2_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
   if (width_bit >= ADC_WIDTH_MAX) return ESP_ERR_INVALID_ARG;
   if (raw_out == NULL) return ESP_ERR_INVALID_ARG;
   if (adc2ptr == NULL) return ESP_ERR_INVALID_ARG;
   /* We are using the wifi, so we will return a timeout always. */
   if (adc2ptr->busy()) return ESP_ERR_TIMEOUT;

   adc2ptr->wait_eoc();
   *raw_out = adc2ptr->getraw();
   return ESP_OK;
}

/**
 *  @brief Output ADC2 reference voltage to gpio 25 or 26 or 27
 *
 *  This function utilizes the testing mux exclusive to ADC 2 to route the
 *  reference voltage one of ADC2's channels. Supported gpios are gpios
 *  25, 26, and 27. This refernce voltage can be manually read from the pin
 *  and used in the esp_adc_cal component.
 *
 *  @param[in]  gpio    GPIO number (gpios 25,26,27 supported)
 *
 *  @return
 *                  - ESP_OK: v_ref successfully routed to selected gpio
 *                  - ESP_ERR_INVALID_ARG: Unsupported gpio
 */
esp_err_t adc2_vref_to_gpio(gpio_num_t gpio) {
   if (gpio != 25 && gpio != 26 && gpio != 27) {
      SC_REPORT_ERROR("ADC", "ADC2 set to reference unsupported GPIO");
      return ESP_ERR_INVALID_ARG;
   }
   PRINTF_INFO("ADC",
      "Setting ADC2 reference voltage to GPIO %0d (not yet in the model)",
      gpio);

   return ESP_OK;
}
