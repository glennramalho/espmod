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

#include <systemc.h>
#include "update.h"
#include "info.h"
#include "esp_bt.h"
#include "btmod.h"
/**
 * @brief  Set BLE TX power
 *         Connection Tx power should only be set after connection created.
 * @param  power_type : The type of which tx power, could set Advertising/Connection/Default and etc
 * @param  power_level: Power level(index) corresponding to absolute value(dbm)
 * @return              ESP_OK - success, other - failed
 */
esp_err_t esp_ble_tx_power_set(esp_ble_power_type_t power_type, esp_power_level_t power_level)
{
    if (power_type >= ESP_BLE_PWR_TYPE_NUM) {
        return ESP_ERR_INVALID_ARG;
    }
    if (power_level > ESP_PWR_LVL_P7) {
        return ESP_ERR_INVALID_ARG;
    }
    else if (btptr == NULL) {
       PRINTF_INFO("BTH", "Btptr was not set");
       return ESP_ERR_INVALID_ARG;
    }
    btptr->set_power(power_type, power_level);

    return ESP_OK;
}


/**
 * @brief  Get BLE TX power
 *         Connection Tx power should only be get after connection created.
 * @param  power_type : The type of which tx power, could set Advertising/Connection/Default and etc
 * @return             >= 0 - Power level, < 0 - Invalid
 */
esp_power_level_t esp_ble_tx_power_get(esp_ble_power_type_t power_type)
{
    return btptr->get_power(power_type);
}

/**
 * @brief  Set BR/EDR TX power
 *         BR/EDR power control will use the power in range of minimum value and maximum value.
 *         The power level will effect the global BR/EDR TX power, such inquire, page, connection and so on.
 *         Please call the function after esp_bt_controller_enable and before any function which cause RF do TX.
 *         So you can call the function before doing discovery, profile init and so on.
 *         For example, if you want BR/EDR use the new TX power to do inquire, you should call
 *         this function before inquire. Another word, If call this function when BR/EDR is in inquire(ING),
 *         please do inquire again after call this function.
 *         Default minimum power level is ESP_PWR_LVL_N0, and maximum power level is ESP_PWR_LVL_P3.
 * @param  min_power_level: The minimum power level
 * @param  max_power_level: The maximum power level
 * @return              ESP_OK - success, other - failed
 */
esp_err_t esp_bredr_tx_power_set(esp_power_level_t min_power_level, esp_power_level_t max_power_level)
{
    esp_err_t err;
    int ret;

    if (btptr == NULL) {
       PRINTF_INFO("BTH", "Btptr was not set");
       return ESP_ERR_INVALID_ARG;
    }
    ret = btptr->bredr_tx_pwr_set(min_power_level, max_power_level);

    if (ret == 0) {
        err = ESP_OK;
    } else if (ret == -1) {
        err = ESP_ERR_INVALID_ARG;
    } else {
        err = ESP_ERR_INVALID_STATE;
    }

    return err;
}


/**
 * @brief  Get BR/EDR TX power
 *         If the argument is not NULL, then store the corresponding value.
 * @param  min_power_level: The minimum power level
 * @param  max_power_level: The maximum power level
 * @return              ESP_OK - success, other - failed
 */
esp_err_t esp_bredr_tx_power_get(esp_power_level_t *min_power_level, esp_power_level_t *max_power_level) {
    if (btptr->bredr_tx_pwr_get((int *)min_power_level, (int *)max_power_level) != 0) {
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}
/**
 * @brief  set default SCO data path
 *         Should be called after controller is enabled, and before (e)SCO link is established
 * @param  data_path: SCO data path
 * @return              ESP_OK - success, other - failed
 */
//esp_err_t esp_bredr_sco_datapath_set(esp_sco_data_path_t data_path);

/**
 * @brief       Initialize BT controller to allocate task and other resource.
 *              This function should be called only once, before any other BT functions are called.
 * @param  cfg: Initial configuration of BT controller. Different from previous version, there's a mode and some
 *              connection configuration in "cfg" to configure controller work mode and allocate the resource which is needed.
 * @return      ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_init(esp_bt_controller_config_t *cfg);

/**
 * @brief  De-initialize BT controller to free resource and delete task.
 *
 * This function should be called only once, after any other BT functions are called.
 * @return  ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_deinit(void);

/**
 * @brief Enable BT controller.
 *               Due to a known issue, you cannot call esp_bt_controller_enable() a second time
 *               to change the controller mode dynamically. To change controller mode, call
 *               esp_bt_controller_disable() and then call esp_bt_controller_enable() with the new mode.
 * @param mode : the mode(BLE/BT/BTDM) to enable. For compatible of API, retain this argument. This mode must be
 *               equal as the mode in "cfg" of esp_bt_controller_init().
 * @return       ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_enable(esp_bt_mode_t mode);

/**
 * @brief  Disable BT controller
 * @return       ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_disable(void);

/**
 * @brief  Get BT controller is initialised/de-initialised/enabled/disabled
 * @return status value
 */
esp_bt_controller_status_t esp_bt_controller_get_status(void);

/** @brief esp_vhci_host_check_send_available
 *  used for check actively if the host can send packet to controller or not.
 *  @return true for ready to send, false means cannot send packet
 */
bool esp_vhci_host_check_send_available(void);

/** @brief esp_vhci_host_send_packet
 * host send packet to controller
 *
 * Should not call this function from within a critical section
 * or when the scheduler is suspended.
 *
 * @param data the packet point
 * @param len the packet length
 */
void esp_vhci_host_send_packet(uint8_t *data, uint16_t len);

/** @brief esp_vhci_host_register_callback
 * register the vhci reference callback
 * struct defined by vhci_host_callback structure.
 * @param callback esp_vhci_host_callback type variable
 * @return ESP_OK - success, ESP_FAIL - failed
 */
esp_err_t esp_vhci_host_register_callback(const esp_vhci_host_callback_t *callback);

/** @brief esp_bt_controller_mem_release
 * release the controller memory as per the mode
 *
 * This function releases the BSS, data and other sections of the controller to heap. The total size is about 70k bytes.
 *
 * esp_bt_controller_mem_release(mode) should be called only before esp_bt_controller_init()
 * or after esp_bt_controller_deinit().
 *
 * Note that once BT controller memory is released, the process cannot be reversed. It means you cannot use the bluetooth
 * mode which you have released by this function.
 *
 * If your firmware will later upgrade the Bluetooth controller mode (BLE -> BT Classic or disabled -> enabled)
 * then do not call this function.
 *
 * If the app calls esp_bt_controller_enable(ESP_BT_MODE_BLE) to use BLE only then it is safe to call
 * esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT) at initialization time to free unused BT Classic memory.
 *
 * If the mode is ESP_BT_MODE_BTDM, then it may be useful to call API esp_bt_mem_release(ESP_BT_MODE_BTDM) instead,
 * which internally calls esp_bt_controller_mem_release(ESP_BT_MODE_BTDM) and additionally releases the BSS and data
 * consumed by the BT/BLE host stack to heap. For more details about usage please refer to the documentation of
 * esp_bt_mem_release() function
 *
 * @param mode : the mode want to release memory
 * @return ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_mem_release(esp_bt_mode_t mode);

/** @brief esp_bt_mem_release
 * release controller memory and BSS and data section of the BT/BLE host stack as per the mode
 *
 * This function first releases controller memory by internally calling esp_bt_controller_mem_release().
 * Additionally, if the mode is set to ESP_BT_MODE_BTDM, it also releases the BSS and data consumed by the BT/BLE host stack to heap
 *
 * Note that once BT memory is released, the process cannot be reversed. It means you cannot use the bluetooth
 * mode which you have released by this function.
 *
 * If your firmware will later upgrade the Bluetooth controller mode (BLE -> BT Classic or disabled -> enabled)
 * then do not call this function.
 *
 * If you never intend to use bluetooth in a current boot-up cycle, you can call esp_bt_mem_release(ESP_BT_MODE_BTDM)
 * before esp_bt_controller_init or after esp_bt_controller_deinit.
 *
 * For example, if a user only uses bluetooth for setting the WiFi configuration, and does not use bluetooth in the rest of the product operation".
 * In such cases, after receiving the WiFi configuration, you can disable/deinit bluetooth and release its memory.
 * Below is the sequence of APIs to be called for such scenarios:
 *
 *      esp_bluedroid_disable();
 *      esp_bluedroid_deinit();
 *      esp_bt_controller_disable();
 *      esp_bt_controller_deinit();
 *      esp_bt_mem_release(ESP_BT_MODE_BTDM);
 *
 * @note In case of NimBLE host, to release BSS and data memory to heap, the mode needs to be
 * set to ESP_BT_MODE_BTDM as controller is dual mode.
 * @param mode : the mode whose memory is to be released
 * @return ESP_OK - success, other - failed
 */
esp_err_t esp_bt_mem_release(esp_bt_mode_t mode);

/**
 * @brief enable bluetooth to enter modem sleep
 *
 * Note that this function shall not be invoked before esp_bt_controller_enable()
 *
 * There are currently two options for bluetooth modem sleep, one is ORIG mode, and another is EVED Mode. EVED Mode is intended for BLE only.
 *
 * For ORIG mode:
 * Bluetooth modem sleep is enabled in controller start up by default if CONFIG_BTDM_MODEM_SLEEP is set and "ORIG mode" is selected. In ORIG modem sleep mode, bluetooth controller will switch off some components and pause to work every now and then, if there is no event to process; and wakeup according to the scheduled interval and resume the work. It can also wakeup earlier upon external request using function "esp_bt_controller_wakeup_request".
 *
 * @return
 *                  - ESP_OK : success
 *                  - other  : failed
 */
esp_err_t esp_bt_sleep_enable (void)
{
   /*
    esp_err_t status;
    if (btdm_controller_status != ESP_BT_CONTROLLER_STATUS_ENABLED) {
        return ESP_ERR_INVALID_STATE;
    }
    if (btdm_controller_get_sleep_mode() == BTDM_MODEM_SLEEP_MODE_ORIG) {
        esp_modem_sleep_register(MODEM_BLE_MODULE);
        esp_modem_sleep_register(MODEM_CLASSIC_BT_MODULE);
        btdm_controller_enable_sleep (true);
        status = ESP_OK;
    } else if (btdm_controller_get_sleep_mode() == BTDM_MODEM_SLEEP_MODE_EVED) {
        esp_modem_sleep_register(MODEM_BLE_MODULE);
        btdm_controller_enable_sleep (true);
        status = ESP_OK;
    } else {
        status = ESP_ERR_NOT_SUPPORTED;
    }

    return status;
    */
   /* We can do something later, but for now we do not have a sleep mode. */
   return ESP_ERR_NOT_SUPPORTED;
}



/**
 * @brief disable bluetooth modem sleep
 *
 * Note that this function shall not be invoked before esp_bt_controller_enable()
 *
 * If esp_bt_sleep_disable() is called, bluetooth controller will not be allowed to enter modem sleep;
 *
 * If ORIG modem sleep mode is in use, if this function is called, bluetooth controller may not immediately wake up if it is dormant then.
 * In this case, esp_bt_controller_wakeup_request() can be used to shorten the time for wakeup.
 *
 * @return
 *                  - ESP_OK : success
 *                  - other  : failed
 */
esp_err_t esp_bt_sleep_disable (void)
{
   /*
    esp_err_t status;
    if (btdm_controller_status != ESP_BT_CONTROLLER_STATUS_ENABLED) {
        return ESP_ERR_INVALID_STATE;
    }
    if (btdm_controller_get_sleep_mode() == BTDM_MODEM_SLEEP_MODE_ORIG) {
        esp_modem_sleep_deregister(MODEM_BLE_MODULE);
        esp_modem_sleep_deregister(MODEM_CLASSIC_BT_MODULE);
        btdm_controller_enable_sleep (false);
        status = ESP_OK;
    } else if (btdm_controller_get_sleep_mode() == BTDM_MODEM_SLEEP_MODE_EVED) {
        esp_modem_sleep_deregister(MODEM_BLE_MODULE);
        btdm_controller_enable_sleep (false);
        status = ESP_OK;
    } else {
        status = ESP_ERR_NOT_SUPPORTED;
    }

    return status;
   */
    return ESP_OK;
}


/**
 * @brief to check whether bluetooth controller is sleeping at the instant, if modem sleep is enabled
 *
 * Note that this function shall not be invoked before esp_bt_controller_enable()
 * This function is supposed to be used ORIG mode of modem sleep
 *
 * @return  true if in modem sleep state, false otherwise
 */
bool esp_bt_controller_is_sleeping(void) { 
   return false;
}

/**
 * @brief request controller to wakeup from sleeping state during sleep mode
 *
 * Note that this function shall not be invoked before esp_bt_controller_enable()
 * Note that this function is supposed to be used ORIG mode of modem sleep
 * Note that after this request, bluetooth controller may again enter sleep as long as the modem sleep is enabled
 *
 * Profiling shows that it takes several milliseconds to wakeup from modem sleep after this request.
 * Generally it takes longer if 32kHz XTAL is used than the main XTAL, due to the lower frequency of the former as the bluetooth low power clock source.
 */
void esp_bt_controller_wakeup_request(void);

/**
 * @brief Manually clear scan duplicate list
 *
 * Note that scan duplicate list will be automatically cleared when the maximum amount of device in the filter is reached
 * the amount of device in the filter can be configured in menuconfig.
 *
 *
 * @return
 *                  - ESP_OK : success
 *                  - other  : failed
 */
esp_err_t esp_ble_scan_dupilcate_list_flush(void);

#include "TestSerial.h"
TestSerial BTserial;
