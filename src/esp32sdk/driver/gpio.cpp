/*******************************************************************************
 * gpio.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports GPIO functions from the ESP32 libraries to the ESPMOD
 *   SystemC Model. It was based off the functions from Espressif Systems.
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
#include "clockpacer.h"
#include "gpio.h"
#include "gpioset.h"
#include "soc/gpio_periph.h"
#include "info.h"
#include "esp_log.h"
#include "soc/gpio_struct.h"
#include "update.h"

static const char* GPIO_TAG = "GPIODRV";
#define GPIO_CHECK(a, str, ret_val) \
    if (!(a)) { \
        ESP_LOGE(GPIO_TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val); \
    }

esp_err_t gpio_config(const gpio_config_t *pGPIOConfig) {
   uint64_t gpio_pin_mask = (pGPIOConfig->pin_bit_mask);
   uint32_t io_reg = 0;
   gpio_num_t io_num = GPIO_NUM_0;
   int input_en, output_en, od_en;

   if (pGPIOConfig->pin_bit_mask == 0 || pGPIOConfig->pin_bit_mask >=
         (((uint64_t) 1) << GPIO_PIN_COUNT)) {
      PRINTF_WARN("GPIODRV", "GPIO_PIN mask error");
      return ESP_ERR_INVALID_ARG;
   }
   if ((pGPIOConfig->mode) & (GPIO_MODE_DEF_OUTPUT)) {
      //GPIO 34/35/36/37/38/39 can only be used as input mode;
      if ((gpio_pin_mask & ( GPIO_SEL_34 | GPIO_SEL_35 | GPIO_SEL_36
            | GPIO_SEL_37 | GPIO_SEL_38 | GPIO_SEL_39))) {
         PRINTF_WARN("GPIODRV", "GPIO34-39 can only be used as input mode");
         return ESP_ERR_INVALID_ARG;
      }
   }
   do {
      io_reg = GPIO_PIN_MUX_REG[io_num];
      if (((gpio_pin_mask >> io_num) & BIT(0))) {
         if (!io_reg) {
            PRINTF_WARN("GPIODRV", "IO%d is not a valid GPIO", io_num);
            return ESP_ERR_INVALID_ARG;
         }
         /* RTC is not yet supported.
         if(RTC_GPIO_IS_VALID_GPIO(io_num)){
            rtc_gpio_deinit(io_num);
         }
         */
         if ((pGPIOConfig->mode) & GPIO_MODE_DEF_INPUT) {
            input_en = 1;
         } else {
            input_en = 0;
         }
         if ((pGPIOConfig->mode) & GPIO_MODE_DEF_OD) {
            od_en = 1;
         } else {
            od_en = 0;
         }
         if ((pGPIOConfig->mode) & GPIO_MODE_DEF_OUTPUT) {
            output_en = 1;
         } else {
            output_en = 0;
         }

         if (input_en) gpio_set_input(io_num);
         else gpio_clr_input(io_num);
         if (output_en) gpio_output_enable(io_num);
         else gpio_output_disable(io_num);
         if (od_en) {
            io_mux *gpin = getgpio(io_num);
            GPIO.pin[io_num].pad_driver = 1;
            gpin->set_oe();
         }
         else {
            io_mux *gpin = getgpio(io_num);
            GPIO.pin[io_num].pad_driver = 0;
            gpin->clr_od();
         }

         if (pGPIOConfig->pull_up_en) {
             gpio_pullup_en(io_num);
         } else {
             gpio_pullup_dis(io_num);
         }
         if (pGPIOConfig->pull_down_en) {
             gpio_pulldown_en(io_num);
         } else {
             gpio_pulldown_dis(io_num);
         }
         /* Interrupts is not yet supported
         gpio_set_intr_type(io_num, pGPIOConfig->intr_type);
         if (pGPIOConfig->intr_type) {
            gpio_intr_enable(io_num);
         } else {
            gpio_intr_disable(io_num);
         }
         */
         /* We always set the function to the GPIO function. */
         PIN_FUNC_SELECT(io_reg, PIN_FUNC_GPIO);
      }
      io_num = (gpio_num_t)((int)io_num + 1);
   } while (io_num < GPIO_PIN_COUNT);

   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
 * @brief Reset an gpio to default state (select gpio function, enable pullup and disable input and output).
 *
 * @param gpio_num GPIO number.
 *
 * @note This function also configures the IOMUX for this pin to the GPIO
 *       function, and disconnects any other peripheral output configured via GPIO
 *       Matrix.
 *
 * @return Always return ESP_OK.
 */
esp_err_t gpio_reset_pin(gpio_num_t gpio_num) {
   clockpacer.wait_next_apb_clk();
   assert(gpio_num >= 0 && GPIO_IS_VALID_GPIO(gpio_num));
   gpio_config_t cfg = {
      .pin_bit_mask = BIT64(gpio_num),
      .mode = GPIO_MODE_DISABLE,
      //for powersave reasons, the GPIO should not be floating, select pullup
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&cfg);
  return ESP_OK;
}

/**
 * @brief  GPIO set interrupt trigger type
 *
 * @param  gpio_num GPIO number. If you want to set the trigger type of e.g. of GPIO16, gpio_num should be GPIO_NUM_16 (16);
 * @param  intr_type Interrupt type, select from gpio_int_type_t
 *
 * @return
 *     - ESP_OK  Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *
 */
esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
 * @brief  Enable GPIO module interrupt signal
 *
 * @note Please do not use the interrupt of GPIO36 and GPIO39 when using ADC.
 *       Please refer to the comments of `adc1_get_raw`.
 *       Please refer to section 3.11 of 'ECO_and_Workarounds_for_Bugs_in_ESP32' for the description of this issue.
 *
 * @param  gpio_num GPIO number. If you want to enable an interrupt on e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *
 */
esp_err_t gpio_intr_enable(gpio_num_t gpio_num) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
 * @brief  Disable GPIO module interrupt signal
 *
 * @param  gpio_num GPIO number. If you want to disable the interrupt of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *
 */
esp_err_t gpio_intr_disable(gpio_num_t gpio_num) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIODRV", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (level) {
      if (gpio_num < 32) {
         GPIO.out_w1ts = (1 << gpio_num);
      } else {
         GPIO.out1_w1ts.data = (1 << (gpio_num - 32));
      }
   } else {
      if (gpio_num < 32) {
         GPIO.out_w1tc = (1 << gpio_num);
      } else {
         GPIO.out1_w1tc.data = (1 << (gpio_num - 32));
      }
   }
   update_gpio_reg();
   return ESP_OK;
}

int gpio_get_level(gpio_num_t gpio_num) {
   bool resp;
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIODRV", "No gpio defined for pin %d", gpio_num);
      return 0;
   }
   resp = gpin->get_val();
   clockpacer.wait_next_apb_clk();
   return (resp)?1:0;
}

/**
 * @brief	 GPIO set direction
 *
 * Configure GPIO direction,such as output_only,input_only,output_and_input
 *
 * @param  gpio_num  Configure GPIO pins number, it should be GPIO number. If you want to set direction of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 * @param  mode GPIO direction
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG GPIO error
 *
 */
esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode) {
   bool resp;
   io_mux *gpin;
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   if (gpio_num >= 34 && (mode & GPIO_MODE_DEF_OUTPUT)) {
      ESP_LOGE(GPIO_TAG, "io_num=%d can only be input", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIODRV", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (mode & GPIO_MODE_DEF_INPUT) resp = gpin->set_ie();
   else resp = gpin->clr_ie();
   if (mode & GPIO_MODE_DEF_OUTPUT)
      resp = resp && (ESP_OK == gpio_output_enable(gpio_num));
   else resp = resp && (ESP_OK == gpio_output_disable(gpio_num));
   if (mode & GPIO_MODE_DEF_OD) {
      GPIO.pin[gpio_num].pad_driver = 1;
      resp = resp & gpin->set_od();
   }
   else {
        GPIO.pin[gpio_num].pad_driver = 0;
        resp = resp & gpin->clr_od();
   }
   clockpacer.wait_next_apb_clk();

   if (resp) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

esp_err_t gpio_set_input(gpio_num_t gpio_num) {
   io_mux *gpin;
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIODRV", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->set_ie()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

esp_err_t gpio_clr_input(gpio_num_t gpio_num) {
   io_mux *gpin;
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIODRV", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->clr_ie()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

esp_err_t gpio_output_disable(gpio_num_t gpio_num)
{
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error",
      ESP_ERR_INVALID_ARG);
   if (gpio_num < 32) {
       GPIO.enable_w1tc = (0x1 << gpio_num);
   } else {
       GPIO.enable1_w1tc.data = (0x1 << (gpio_num - 32));
   }
   // Ensure no other output signal is routed via GPIO matrix to this pin
   GPIO.func_out_sel_cfg[gpio_num].func_sel = SIG_GPIO_OUT_IDX;
   update_gpio_oe();

   return ESP_OK;
}

esp_err_t gpio_output_enable(gpio_num_t gpio_num)
{
   GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "GPIO output gpio_num error", ESP_ERR_INVALID_ARG);
   if (gpio_num < 32) {
      GPIO.enable_w1ts = (0x1 << gpio_num);
   } else {
      GPIO.enable1_w1ts.data = (0x1 << (gpio_num - 32));
   }
   update_gpio_oe();
   gpio_matrix_out(gpio_num, SIG_GPIO_OUT_IDX, false, false);
   return ESP_OK;
}

/**
 * @brief  Configure GPIO pull-up/pull-down resistors
 *
 * Only pins that support both input & output have integrated pull-up and pull-down resistors. Input-only GPIOs 34-39 do not.
 *
 * @param  gpio_num GPIO number. If you want to set pull up or down mode for e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16);
 * @param  pull GPIO pull up/down mode.
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG : Parameter error
 *
 */
esp_err_t gpio_set_pull_mode(gpio_num_t gpio_num, gpio_pull_mode_t pull) {
   bool resp;
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIO", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   switch(pull) {
      case GPIO_PULLUP_ONLY:
         resp=gpin->set_wpu();
         resp=resp&gpin->clr_wpd();
         break;
      case GPIO_PULLDOWN_ONLY:
         resp=gpin->set_wpd();
         resp=resp&gpin->clr_wpu();
         break;
      case GPIO_PULLUP_PULLDOWN:
         resp=gpin->set_wpd();
         resp=resp&gpin->set_wpu();
         break;
      case GPIO_FLOATING:
         resp=gpin->clr_wpd();
         resp=resp&gpin->clr_wpu();
         break;
      default: resp = false; break;
   }

   clockpacer.wait_next_apb_clk();

   if (!resp) {
      PRINTF_WARN("GPIO", "Attempting to set pin %d to illegal pull %d",
         gpio_num, pull);
      return ESP_ERR_INVALID_ARG;
   }

   return ESP_OK;
}

/**
 * @brief Enable GPIO wake-up function.
 *
 * @param gpio_num GPIO number.
 *
 * @param intr_type GPIO wake-up type. Only GPIO_INTR_LOW_LEVEL or GPIO_INTR_HIGH_LEVEL can be used.
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t gpio_wakeup_enable(gpio_num_t gpio_num, gpio_int_type_t intr_type) {
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   PRINTF_WARN("GPIODRV", "GPIO Wakeups are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
 * @brief Disable GPIO wake-up function.
 *
 * @param gpio_num GPIO number
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t gpio_wakeup_disable(gpio_num_t gpio_num) {
   GPIO_CHECK(GPIO_IS_VALID_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   PRINTF_WARN("GPIODRV", "GPIO Wakeups are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
 * @brief   Register GPIO interrupt handler, the handler is an ISR.
 *          The handler will be attached to the same CPU core that this function is running on.
 *
 * This ISR function is called whenever any GPIO interrupt occurs. See
 * the alternative gpio_install_isr_service() and
 * gpio_isr_handler_add() API in order to have the driver support
 * per-GPIO ISRs.
 *
 * @param  fn  Interrupt handler function.
 * @param  intr_alloc_flags Flags used to allocate the interrupt. One or multiple (ORred)
 *            ESP_INTR_FLAG_* values. See esp_intr_alloc.h for more info.
 * @param  arg  Parameter for handler function
 * @param  handle Pointer to return handle. If non-NULL, a handle for the interrupt will be returned here.
 *
 * \verbatim embed:rst:leading-asterisk
 * To disable or remove the ISR, pass the returned handle to the :doc:`interrupt allocation functions </api-reference/system/intr_alloc>`.
 * \endverbatim
 *
 * @return
 *     - ESP_OK Success ;
 *     - ESP_ERR_INVALID_ARG GPIO error
 *     - ESP_ERR_NOT_FOUND No free interrupt found with the specified flags
 */
esp_err_t gpio_isr_register(void (*fn)(void*), void * arg, int intr_alloc_flags, gpio_isr_handle_t *handle) {
   GPIO_CHECK(fn, "GPIO ISR null", ESP_ERR_INVALID_ARG);
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
  * @brief Enable pull-up on GPIO.
  *
  * @param gpio_num GPIO number
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_pullup_en(gpio_num_t gpio_num) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIO", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->set_wpu()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

/**
  * @brief Disable pull-up on GPIO.
  *
  * @param gpio_num GPIO number
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_pullup_dis(gpio_num_t gpio_num) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIO", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->clr_wpu()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

/**
  * @brief Enable pull-down on GPIO.
  *
  * @param gpio_num GPIO number
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_pulldown_en(gpio_num_t gpio_num) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIO", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->set_wpd()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}

/**
  * @brief Disable pull-down on GPIO.
  *
  * @param gpio_num GPIO number
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_pulldown_dis(gpio_num_t gpio_num) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_WARN("GPIO", "No gpio defined for pin %d", gpio_num);
      return ESP_ERR_INVALID_ARG;
   }
   if (gpin->clr_wpd()) return ESP_OK;
   else return ESP_ERR_INVALID_ARG;
}


/**
  * @brief Install the driver's GPIO ISR handler service, which allows per-pin GPIO interrupt handlers.
  *
  * This function is incompatible with gpio_isr_register() - if that function is used, a single global ISR is registered for all GPIO interrupts. If this function is used, the ISR service provides a global GPIO ISR and individual pin handlers are registered via the gpio_isr_handler_add() function.
  *
  * @param intr_alloc_flags Flags used to allocate the interrupt. One or multiple (ORred)
  *            ESP_INTR_FLAG_* values. See esp_intr_alloc.h for more info.
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_NO_MEM No memory to install this service
  *     - ESP_ERR_INVALID_STATE ISR service already installed.
  *     - ESP_ERR_NOT_FOUND No free interrupt found with the specified flags
  *     - ESP_ERR_INVALID_ARG GPIO error
  */
esp_err_t gpio_install_isr_service(int intr_alloc_flags) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
  * @brief Uninstall the driver's GPIO ISR service, freeing related resources.
  */
void gpio_uninstall_isr_service() {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
}

/**
  * @brief Add ISR handler for the corresponding GPIO pin.
  *
  * Call this function after using gpio_install_isr_service() to
  * install the driver's GPIO ISR handler service.
  *
  * The pin ISR handlers no longer need to be declared with IRAM_ATTR,
  * unless you pass the ESP_INTR_FLAG_IRAM flag when allocating the
  * ISR in gpio_install_isr_service().
  *
  * This ISR handler will be called from an ISR. So there is a stack
  * size limit (configurable as "ISR stack size" in menuconfig). This
  * limit is smaller compared to a global GPIO interrupt handler due
  * to the additional level of indirection.
  *
  * @param gpio_num GPIO number
  * @param isr_handler ISR handler function for the corresponding GPIO number.
  * @param args parameter for ISR handler.
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_STATE Wrong state, the ISR service has not been initialized.
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void* args) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
  * @brief Remove ISR handler for the corresponding GPIO pin.
  *
  * @param gpio_num GPIO number
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_STATE Wrong state, the ISR service has not been initialized.
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_isr_handler_remove(gpio_num_t gpio_num) {
   PRINTF_WARN("GPIODRV", "GPIO Interrupts are not yet supported.");
   clockpacer.wait_next_apb_clk();
   return ESP_OK;
}

/**
  * @brief Set GPIO pad drive capability
  *
  * @param gpio_num GPIO number, only support output GPIOs
  * @param strength Drive capability of the pad
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
gpio_drive_cap_t __gpio_pin_drive_cap[40] = {
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT, GPIO_DRIVE_CAP_DEFAULT,
   GPIO_DRIVE_CAP_DEFAULT};

esp_err_t gpio_set_drive_capability(gpio_num_t gpio_num, gpio_drive_cap_t strength) {
    GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
    GPIO_CHECK(strength < GPIO_DRIVE_CAP_MAX, "GPIO drive capability error", ESP_ERR_INVALID_ARG);

    PRINTF_INFO("GPIODRV",
      "Setting drive strength for IO%d to %d. Note: not visible in the model.",
      gpio_num, strength);
    __gpio_pin_drive_cap[gpio_num] = strength;

    return ESP_OK;
}

/**
  * @brief Get GPIO pad drive capability
  *
  * @param gpio_num GPIO number, only support output GPIOs
  * @param strength Pointer to accept drive capability of the pad
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_INVALID_ARG Parameter error
  */
esp_err_t gpio_get_drive_capability(gpio_num_t gpio_num, gpio_drive_cap_t* strength) {
   GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);
   GPIO_CHECK(strength != NULL, "GPIO drive capability pointer error", ESP_ERR_INVALID_ARG);
   /* We do not have a real level, so we simply return the last set value. */
   *strength = __gpio_pin_drive_cap[gpio_num];
   return ESP_OK;
}

/**
  * @brief Enable gpio pad hold function.
  *
  * The gpio pad hold function works in both input and output modes, but must be output-capable gpios.
  * If pad hold enabled:
  *   in output mode: the output level of the pad will be force locked and can not be changed.
  *   in input mode: the input value read will not change, regardless the changes of input signal.
  *
  * The state of digital gpio cannot be held during Deep-sleep, and it will resume the hold function
  * when the chip wakes up from Deep-sleep. If the digital gpio also needs to be held during Deep-sleep,
  * `gpio_deep_sleep_hold_en` should also be called.
  *
  * Power down or call gpio_hold_dis will disable this function.
  *
  * @param gpio_num GPIO number, only support output-capable GPIOs
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_NOT_SUPPORTED Not support pad hold function
  */
static const uint32_t GPIO_HOLD_MASK[34] = {
    0,
    GPIO_SEL_1,
    0,
    GPIO_SEL_0,
    0,
    GPIO_SEL_8,
    GPIO_SEL_2,
    GPIO_SEL_3,
    GPIO_SEL_4,
    GPIO_SEL_5,
    GPIO_SEL_6,
    GPIO_SEL_7,
    0,
    0,
    0,
    0,
    GPIO_SEL_9,
    GPIO_SEL_10,
    GPIO_SEL_11,
    GPIO_SEL_12,
    0,
    GPIO_SEL_14,
    GPIO_SEL_15,
    GPIO_SEL_16,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
esp_err_t gpio_hold_en(gpio_num_t gpio_num) {
   GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "Only output-capable GPIO support this function", ESP_ERR_NOT_SUPPORTED);
   esp_err_t r = ESP_OK;
   /* hold is not yet implemented as there is no deep sleep. */
   return r == ESP_OK ? ESP_OK : ESP_ERR_NOT_SUPPORTED;
}

/**
  * @brief Disable gpio pad hold function.
  *
  * When the chip is woken up from Deep-sleep, the gpio will be set to the default mode, so, the gpio will output
  * the default level if this function is called. If you dont't want the level changes, the gpio should be configured to
  * a known state before this function is called.
  *  e.g.
  *     If you hold gpio18 high during Deep-sleep, after the chip is woken up and `gpio_hold_dis` is called, 
  *     gpio18 will output low level(because gpio18 is input mode by default). If you don't want this behavior,
  *     you should configure gpio18 as output mode and set it to hight level before calling `gpio_hold_dis`.
  *
  * @param gpio_num GPIO number, only support output-capable GPIOs
  *
  * @return
  *     - ESP_OK Success
  *     - ESP_ERR_NOT_SUPPORTED Not support pad hold function
  */
esp_err_t gpio_hold_dis(gpio_num_t gpio_num) {
   GPIO_CHECK(GPIO_IS_VALID_OUTPUT_GPIO(gpio_num), "Only output-capable GPIO support this function", ESP_ERR_NOT_SUPPORTED);
   esp_err_t r = ESP_OK;
   /* Not implemented as there is no deep sleep. */
   return r == ESP_OK ? ESP_OK : ESP_ERR_NOT_SUPPORTED;
}

/**
  * @brief Enable all digital gpio pad hold function during Deep-sleep.
  *
  * When the chip is in Deep-sleep mode, all digital gpio will hold the state before sleep, and when the chip is woken up,
  * the status of digital gpio will not be held. Note that the pad hold feature only works when the chip is in Deep-sleep mode,
  * when not in sleep mode, the digital gpio state can be changed even you have called this function.
  *
  * Power down or call gpio_hold_dis will disable this function, otherwise, the digital gpio hold feature works as long as the chip enter Deep-sleep.
  */
void gpio_deep_sleep_hold_en(void) { /* no deep sleep yet. */ }

/**
  * @brief Disable all digital gpio pad hold function during Deep-sleep.
  *
  */
void gpio_deep_sleep_hold_dis(void) { /* no deep sleep yet. */ }

/**
  * @brief Set pad input to a peripheral signal through the IOMUX.
  * @param gpio_num GPIO number of the pad.
  * @param signal_idx Peripheral signal id to input. One of the ``*_IN_IDX`` signals in ``soc/gpio_sig_map.h``.
  */
void gpio_iomux_in(uint32_t gpio_num, uint32_t signal_idx) {
   GPIO.func_in_sel_cfg[signal_idx].sig_in_sel = 0;
   update_gpio();
   PIN_INPUT_ENABLE((gpio_num_t)GPIO_PIN_MUX_REG[gpio_num]);
}

/**
  * @brief Set peripheral output to an GPIO pad through the IOMUX.
  * @param gpio_num gpio_num GPIO number of the pad.
  * @param func The function number of the peripheral pin to output pin.
  *        One of the ``FUNC_X_*`` of specified pin (X) in ``soc/io_mux_reg.h``.
  * @param oen_inv True if the output enable needs to be inversed, otherwise False.
  */
void gpio_iomux_out(uint8_t gpio_num, int func, bool oen_inv) {
   io_mux *gpin = getgpio(gpio_num);
   if (gpin == NULL) return;
   if (oen_inv)
      PRINTF_WARN("GPIODRV", "GPIO func Output invert is not yet supported.")
   if (func < 0 || func > 5) {
      PRINTF_WARN("GPIODRV", "GPIO func %d is illegal", func)
   }
   /* Illegal functions we discard as there is no way to inform the user. */
   /* GPIOs sometmes show up on two locations but in the model it is always
    * in position 3. We then need to adjust the value. Note: the macros from
    * the library go vrom 0-5 instead of 1-6.
    */
   if (funcmatrix[gpio_num][func] == UGPIO)
      gpin->set_function(3);
   else gpin->set_function(func+1);
}
