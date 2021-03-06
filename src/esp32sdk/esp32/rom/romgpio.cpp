/*******************************************************************************
 * romgpio.cpp -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports the rom/gpio.h functions for the ESP32 to the ESPMOD
 *   SystemC model. It was based off the functions from Espressif Systems.
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

#include "gpio.h"
#include "driver/gpio.h"
#include "Arduino.h"
#include "esp32-hal-gpio.h"
#include "gpioset.h"
#include "soc/gpio_struct.h"
#include "gpio_matrix.h"
#include "info.h"

/**
  * @brief Initialize GPIO. This includes reading the GPIO Configuration DataSet
  *        to initialize "output enables" and pin configurations for each gpio pin.
  *        Please do not call this function in SDK.
  *
  * @param  None
  *
  * @return None
  */
void gpio_init(void) {}

/**
  * @brief Change GPIO(0-31) pin output by setting, clearing, or disabling pins, GPIO0<->BIT(0).
  *         There is no particular ordering guaranteed; so if the order of writes is significant,
  *         calling code should divide a single call into multiple calls.
  *
  * @param  uint32_t set_mask : the gpios that need high level.
  *
  * @param  uint32_t clear_mask : the gpios that need low level.
  *
  * @param  uint32_t enable_mask : the gpios that need be changed.
  *
  * @param  uint32_t disable_mask : the gpios that need diable output.
  *
  * @return None
  */
void gpio_output_set(uint32_t set_mask, uint32_t clear_mask, uint32_t enable_mask, uint32_t disable_mask) {
   GPIO.out_w1ts = set_mask & enable_mask;
   GPIO.out_w1tc = clear_mask & enable_mask;
   GPIO.enable_w1tc = disable_mask;
   update_gpio_reg();
   update_gpio_oe();
}

/**
  * @brief Change GPIO(32-39) pin output by setting, clearing, or disabling pins, GPIO32<->BIT(0).
  *         There is no particular ordering guaranteed; so if the order of writes is significant,
  *         calling code should divide a single call into multiple calls.
  *
  * @param  uint32_t set_mask : the gpios that need high level.
  *
  * @param  uint32_t clear_mask : the gpios that need low level.
  *
  * @param  uint32_t enable_mask : the gpios that need be changed.
  *
  * @param  uint32_t disable_mask : the gpios that need diable output.
  *
  * @return None
  */
void gpio_output_set_high(uint32_t set_mask, uint32_t clear_mask, uint32_t enable_mask, uint32_t disable_mask) {
   GPIO.out1_w1ts.data = set_mask & enable_mask;
   GPIO.out1_w1tc.data = clear_mask & enable_mask;
   GPIO.enable1_w1tc.data = disable_mask;
   update_gpio_reg();
   update_gpio_oe();
}

/**
  * @brief Sample the value of GPIO input pins(0-31) and returns a bitmask.
  *
  * @param None
  *
  * @return uint32_t : bitmask for GPIO input pins, BIT(0) for GPIO0.
  */
uint32_t gpio_input_get(void) {
   uint32_t mask;
   uint32_t res;
   int g;

   del1cycle();
   for (g = (int)GPIO_NUM_0, mask = 0x1UL; g < (int)GPIO_NUM_32;
         g = g + 1, mask = mask << 1) {
      /* Not all exist and some were not implemented, so we return zero
       * for these bits.
       */
      if (getgpio(g) != NULL && digitalRead_nodel(g) == HIGH) res = res | mask;
   }

   return res;
}

/**
  * @brief Sample the value of GPIO input pins(32-39) and returns a bitmask.
  *
  * @param None
  *
  * @return uint32_t : bitmask for GPIO input pins, BIT(0) for GPIO32.
  */
uint32_t gpio_input_get_high(void) {
   uint32_t mask;
   uint32_t res;
   int g;

   del1cycle();
   for (g = (int)GPIO_NUM_32, mask = 0x1UL; g < (int)GPIO_NUM_MAX;
         g = g + 1, mask = mask << 1) {
      /* Not all exist and some were not implemented, so we return zero
       * for these bits.
       */
      if (getgpio(g) != NULL && digitalRead_nodel(g) == HIGH) res = res | mask;
   }

   return res;
}

/**
  * @brief Register an application-specific interrupt handler for GPIO pin interrupts.
  *        Once the interrupt handler is called, it will not be called again until after a call to gpio_intr_ack.
  *        Please do not call this function in SDK.
  *
  * @param gpio_intr_handler_fn_t fn : gpio application-specific interrupt handler
  *
  * @param void *arg : gpio application-specific interrupt handler argument.
  *
  * @return None
  */
void gpio_intr_handler_register(gpio_intr_handler_fn_t fn, void *arg);

/**
  * @brief Get gpio interrupts which happens but not processed.
  *        Please do not call this function in SDK.
  *
  * @param None
  *
  * @return uint32_t : bitmask for GPIO pending interrupts, BIT(0) for GPIO0.
  */
uint32_t gpio_intr_pending(void);

/**
  * @brief Get gpio interrupts which happens but not processed.
  *        Please do not call this function in SDK.
  *
  * @param None
  *
  * @return uint32_t : bitmask for GPIO pending interrupts, BIT(0) for GPIO32.
  */
uint32_t gpio_intr_pending_high(void);

/**
  * @brief Ack gpio interrupts to process pending interrupts.
  *        Please do not call this function in SDK.
  *
  * @param uint32_t ack_mask: bitmask for GPIO ack interrupts, BIT(0) for GPIO0.
  *
  * @return None
  */
void gpio_intr_ack(uint32_t ack_mask);

/**
  * @brief Ack gpio interrupts to process pending interrupts.
  *        Please do not call this function in SDK.
  *
  * @param uint32_t ack_mask: bitmask for GPIO ack interrupts, BIT(0) for GPIO32.
  *
  * @return None
  */
void gpio_intr_ack_high(uint32_t ack_mask);

/**
  * @brief Set GPIO to wakeup the ESP32.
  *        Please do not call this function in SDK.
  *
  * @param uint32_t i: gpio number.
  *
  * @param GPIO_INT_TYPE intr_state : only GPIO_PIN_INTR_LOLEVEL\GPIO_PIN_INTR_HILEVEL can be used
  *
  * @return None
  */
void gpio_pin_wakeup_enable(uint32_t i, GPIO_INT_TYPE intr_state);

/**
  * @brief disable GPIOs to wakeup the ESP32.
  *        Please do not call this function in SDK.
  *
  * @param None
  *
  * @return None
  */
void gpio_pin_wakeup_disable(void);

/**
  * @brief set gpio input to a signal, one gpio can input to several signals.
  *
  * @param uint32_t gpio : gpio number, 0~0x27
  *                        gpio == 0x30, input 0 to signal
  *                        gpio == 0x34, ???
  *                        gpio == 0x38, input 1 to signal
  *
  * @param uint32_t signal_idx : signal index.
  *
  * @param bool inv : the signal is inv or not
  *
  * @return None
  */
void gpio_matrix_in(uint32_t gpio, uint32_t signal_idx, bool inv) {
   uint32_t gpiofunc;
   /* We first check the signal, if it is illegal, there is nothing we can
    * do about it.
    */
   if (signal_idx >= 256) {
      PRINTF_WARN("ROMGPIO", "Attempting to set illegal signal %d", signal_idx);
      return;
   }
   /* If the invert is used, we issue a warning. */
   if (inv && gpio < GPIO_PIN_COUNT) {
      PRINTF_WARN("ROMGPIO",
            "invert gpio matrix function is not yet supported.");
   }
   /* We next check that the gpio matrix function. Note that most are simply
    * the gpios but there are two extra ones.
    */
   if (gpio < GPIO_PIN_COUNT) gpiofunc = gpio;
   else if (gpio == 0x30 && !inv || gpio == 0x38 && inv)
      gpiofunc = GPIOMATRIX_LOGIC0;
   else if (gpio == 0x38 && !inv || gpio == 0x30 && inv)
      gpiofunc = GPIOMATRIX_LOGIC1;
   /* Any other ones we issue a warning and default to logic 0. */
   else  {
      PRINTF_WARN("ROMGPIO", "Attempting to set illegal GPIO function %d",gpio);
      gpiofunc = (inv)?GPIOMATRIX_LOGIC1:GPIOMATRIX_LOGIC0;
   }
   GPIO.func_in_sel_cfg[signal_idx].func_sel = gpiofunc;
   GPIO.func_in_sel_cfg[signal_idx].sig_in_sel = 1;
   GPIO.func_in_sel_cfg[signal_idx].sig_in_inv = inv;
   update_gpio();
}

/**
  * @brief set signal output to gpio, one signal can output to several gpios.
  *
  * @param uint32_t gpio : gpio number, 0~0x27
  *
  * @param uint32_t signal_idx : signal index.
  *                        signal_idx == 0x100, cancel output put to the gpio
  *
  * @param bool out_inv : the signal output is inv or not
  *
  * @param bool oen_inv : the signal output enable is inv or not
  *
  * @return None
  */
void gpio_matrix_out(uint32_t gpio, uint32_t signal_idx, bool out_inv, bool oen_inv) {
   /* We first check the GPIO number. If it is illegal there is nothing we can
    * do.
    */
   if (gpio >= GPIO_PIN_COUNT) {
      PRINTF_WARN("ROMGPIO", "Attempting to set illegal GPIO %d", gpio);
      return;
   }
   /* The function we simply hand over to the output mux. It knows what to do.
    * If it is illegal, it handles it too.
    */
   GPIO.func_out_sel_cfg[gpio].func_sel = signal_idx;
   GPIO.func_out_sel_cfg[gpio].inv_sel = out_inv;
   GPIO.func_out_sel_cfg[gpio].oen_sel = 0;
   GPIO.func_out_sel_cfg[gpio].oen_inv_sel = oen_inv;
   update_gpio();
}

/**
  * @brief Select pad as a gpio function from IOMUX.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @return None
  */
void gpio_pad_select_gpio(uint8_t gpio_num) {
   io_mux* gpin = getgpio(gpio_num);
   if (gpin == NULL) {
      PRINTF_ERROR("GPIO", "Attemping to set mode to non existant GPIO%d",
         gpio_num);
      return;
   }
   gpin->set_function(3);
}

/**
  * @brief Set pad driver capability.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @param uint8_t drv : 0-3
  *
  * @return None
  */
void gpio_pad_set_drv(uint8_t gpio_num, uint8_t drv) {
   gpio_set_drive_capability((gpio_num_t)gpio_num, (gpio_drive_cap_t)drv);
}

/**
  * @brief Pull up the pad from gpio number.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @return None
  */
void gpio_pad_pullup(uint8_t gpio_num) {
   gpio_pullup_en((gpio_num_t)gpio_num);
}

/**
  * @brief Pull down the pad from gpio number.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @return None
  */
void gpio_pad_pulldown(uint8_t gpio_num) {
   gpio_pulldown_en((gpio_num_t)gpio_num);
}

/**
  * @brief Unhold the pad from gpio number.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @return None
  */
void gpio_pad_unhold(uint8_t gpio_num) {
   gpio_hold_dis((gpio_num_t)gpio_num);
}

/**
  * @brief Hold the pad from gpio number.
  *
  * @param uint32_t gpio_num : gpio number, 0~0x27
  *
  * @return None
  */
void gpio_pad_hold(uint8_t gpio_num) {
   gpio_hold_en((gpio_num_t)gpio_num);
}

/**
  * @}
  */
