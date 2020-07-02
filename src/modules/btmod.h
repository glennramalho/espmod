/*******************************************************************************
 * btmod.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 BT module.
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

#ifndef _BTMOD_H
#define _BTMOD_H

#include <systemc.h>
#include "cchan.h"
#include "TestSerial.h"
#include "esp_bt.h"

class btmod : public cchan {
   public:
   sc_event connected_ev;
   sc_event disconnected_ev;
   sc_event running_ev;

   protected:
   esp_power_level_t power_level[ESP_BLE_PWR_TYPE_NUM];
   esp_power_level_t minpower, maxpower;

   public:
   btmod(sc_module_name name, int tx_buffer_size, int rx_buffer_size):
      cchan(name, tx_buffer_size, rx_buffer_size) {
   };
   void set_power(esp_ble_power_type_t _pt, esp_power_level_t _pl) {
      if (_pt < ESP_BLE_PWR_TYPE_NUM) power_level[_pt] = _pl;
   }
   esp_power_level_t get_power(esp_ble_power_type_t _pt) {
      /* As long as the PT is valid, we return the level. If the pt is
       * illegal, we return anything just because we have to return something.
       */
      if (_pt < ESP_BLE_PWR_TYPE_NUM) return power_level[_pt];
      else return power_level[ESP_BLE_PWR_TYPE_NUM-1];
   }
   bool bredr_tx_pwr_set(esp_power_level_t _min, esp_power_level_t _max) {
      if (_min < ESP_PWR_LVL_N12 || _min > ESP_PWR_LVL_P7) return false;
      if (_max < ESP_PWR_LVL_N12 || _max > ESP_PWR_LVL_P7) return false;
      if (_min > _max) return false;
      minpower = _min;
      maxpower = _max;
      return true;
   }
   int bredr_tx_pwr_get(int *_min, int *_max) {
      if (_min != NULL) *_min = (int)minpower;
      if (_max != NULL) *_max = (int)maxpower;
      return 0;
   }
};

extern btmod *btptr;
extern TestSerial BTserial;

#endif
