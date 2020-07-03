// Copyright 2018 Evandro Luis Copercini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sdkconfig.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "esp_gap_bt_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <systemc.h>
#include "info.h"
#include "btmod.h"


#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include "BluetoothSerial.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include <esp_log.h>

#include "esp32-hal-log.h"

const char * _spp_server_name = "ESP32SPP";

#define RX_QUEUE_SIZE 512
#define TX_QUEUE_SIZE 32
static uint32_t _spp_client = 0;
static boolean secondConnectionAttempt;
static esp_spp_cb_t * custom_spp_callback = NULL;
static BluetoothSerialDataCb custom_data_callback = NULL;

#define INQ_LEN 0x10
#define INQ_NUM_RSPS 20
#define READY_TIMEOUT (10 * 1000)
#define SCAN_TIMEOUT (INQ_LEN * 2 * 1000)
static esp_bd_addr_t _peer_bd_addr;
static char _remote_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
static bool _isRemoteAddressSet;
static bool _isMaster;
static esp_bt_pin_code_t _pin_code;
static int _pin_len;
static bool _isPinSet;
static bool _enableSSP;

#define SPP_RUNNING     0x01
#define SPP_CONNECTED   0x02
#define SPP_CONGESTED   0x04
#define SPP_DISCONNECTED 0x08

typedef struct {
        size_t len;
        uint8_t data[];
} spp_packet_t;

#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
static char *bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }

  uint8_t *p = bda;
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          p[0], p[1], p[2], p[3], p[4], p[5]);
  return str;
}
#endif

static bool get_name_from_eir(uint8_t *eir, char *bdname, uint8_t *bdname_len)
{
    if (!eir || !bdname || !bdname_len) {
        return false;
    }

    uint8_t *rmt_bdname, rmt_bdname_len;
    *bdname = *bdname_len = rmt_bdname_len = 0;

    rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
    if (!rmt_bdname) {
        rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
    }
    if (rmt_bdname) {
        rmt_bdname_len = rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN ? ESP_BT_GAP_MAX_BDNAME_LEN : rmt_bdname_len;
        memcpy(bdname, rmt_bdname, rmt_bdname_len);
        bdname[rmt_bdname_len] = 0;
        *bdname_len = rmt_bdname_len;
        return true;
    }
    return false;
}

static bool btSetPin() {
    esp_bt_pin_type_t pin_type;
    if (_isPinSet) {
        if (_pin_len) {
            log_i("pin set");
            pin_type = ESP_BT_PIN_TYPE_FIXED;
        } else {
            _isPinSet = false;
            log_i("pin reset");
            pin_type = ESP_BT_PIN_TYPE_VARIABLE; // pin_code would be ignored (default)
        }
        return (esp_bt_gap_set_pin(pin_type, _pin_len, _pin_code) == ESP_OK);        
    }
    return false;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
/*
    switch (event)
    {
    case ESP_SPP_INIT_EVT:
        log_i("ESP_SPP_INIT_EVT");
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        if (!_isMaster) {
            log_i("ESP_SPP_INIT_EVT: slave: start");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, _spp_server_name);
        }
        break;

    case ESP_SPP_SRV_OPEN_EVT://Server connection open
        log_i("ESP_SPP_SRV_OPEN_EVT");
        if (!_spp_client){
            _spp_client = param->open.handle;
        } else {
            secondConnectionAttempt = true;
            esp_spp_disconnect(param->open.handle);
        }
        break;

    case ESP_SPP_CLOSE_EVT://Client connection closed
        log_i("ESP_SPP_CLOSE_EVT");
        if(secondConnectionAttempt) {
            secondConnectionAttempt = false;
        } else {
            _spp_client = 0;
        }        
        break;

    case ESP_SPP_CONG_EVT://connection congestion status changed
        log_v("ESP_SPP_CONG_EVT: %s", param->cong.cong?"CONGESTED":"FREE");
        break;

    case ESP_SPP_WRITE_EVT://write operation completed
        log_v("ESP_SPP_WRITE_EVT: %u %s", param->write.len, param->write.cong?"CONGESTED":"FREE");
        break;

    case ESP_SPP_DATA_IND_EVT://connection received data
        log_v("ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);
        //esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len); //for low level debug
        //ets_printf("r:%u\n", param->data_ind.len);

        if(custom_data_callback){
            custom_data_callback(param->data_ind.data, param->data_ind.len);
        }
        break;

    case ESP_SPP_DISCOVERY_COMP_EVT://discovery complete
        log_i("ESP_SPP_DISCOVERY_COMP_EVT");
        if (param->disc_comp.status == ESP_SPP_SUCCESS) {
            log_i("ESP_SPP_DISCOVERY_COMP_EVT: spp connect to remote");
            esp_spp_connect(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_MASTER, param->disc_comp.scn[0], _peer_bd_addr);
        }
        break;

    case ESP_SPP_OPEN_EVT://Client connection open
        log_i("ESP_SPP_OPEN_EVT");
        if (!_spp_client){
                _spp_client = param->open.handle;
        } else {
            secondConnectionAttempt = true;
            esp_spp_disconnect(param->open.handle);
        }
        break;

    case ESP_SPP_START_EVT://server started
        log_i("ESP_SPP_START_EVT");
        break;

    case ESP_SPP_CL_INIT_EVT://client initiated a connection
        log_i("ESP_SPP_CL_INIT_EVT");
        break;

    default:
        break;
    }
    if(custom_spp_callback)(*custom_spp_callback)(event, param);
    */
}

void BluetoothSerial::onData(BluetoothSerialDataCb cb){
    custom_data_callback = cb;
}

static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    /*
    switch(event){
        case ESP_BT_GAP_DISC_RES_EVT:
            log_i("ESP_BT_GAP_DISC_RES_EVT");
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
            char bda_str[18];
            log_i("Scanned device: %s", bda2str(param->disc_res.bda, bda_str, 18));
#endif
            for (int i = 0; i < param->disc_res.num_prop; i++) {
                uint8_t peer_bdname_len;
                char peer_bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
                switch(param->disc_res.prop[i].type) {
                    case ESP_BT_GAP_DEV_PROP_EIR:  
                        if (get_name_from_eir((uint8_t*)param->disc_res.prop[i].val, peer_bdname, &peer_bdname_len)) {
                            log_i("ESP_BT_GAP_DISC_RES_EVT : EIR : %s : %d", peer_bdname, peer_bdname_len);
                            if (strlen(_remote_name) == peer_bdname_len
                                && strncmp(peer_bdname, _remote_name, peer_bdname_len) == 0) {
                                log_v("ESP_BT_GAP_DISC_RES_EVT : SPP_START_DISCOVERY_EIR : %s", peer_bdname, peer_bdname_len);
                                _isRemoteAddressSet = true;
                                memcpy(_peer_bd_addr, param->disc_res.bda, ESP_BD_ADDR_LEN);
                                esp_bt_gap_cancel_discovery();
                                esp_spp_start_discovery(_peer_bd_addr);
                            }
                        }
                        break;

                    case ESP_BT_GAP_DEV_PROP_BDNAME:
                        peer_bdname_len = param->disc_res.prop[i].len;
                        memcpy(peer_bdname, param->disc_res.prop[i].val, peer_bdname_len);
                        peer_bdname_len--; // len includes 0 terminator
                        log_v("ESP_BT_GAP_DISC_RES_EVT : BDNAME :  %s : %d", peer_bdname, peer_bdname_len);
                        if (strlen(_remote_name) == peer_bdname_len
                            && strncmp(peer_bdname, _remote_name, peer_bdname_len) == 0) {
                            log_i("ESP_BT_GAP_DISC_RES_EVT : SPP_START_DISCOVERY_BDNAME : %s", peer_bdname);
                            _isRemoteAddressSet = true;
                            memcpy(_peer_bd_addr, param->disc_res.bda, ESP_BD_ADDR_LEN);
                            esp_bt_gap_cancel_discovery();
                            esp_spp_start_discovery(_peer_bd_addr);
                        } 
                        break;

                    case ESP_BT_GAP_DEV_PROP_COD:
                        log_d("ESP_BT_GAP_DEV_PROP_COD");
                        break;

                    case ESP_BT_GAP_DEV_PROP_RSSI:
                        log_d("ESP_BT_GAP_DEV_PROP_RSSI");
                        break;
                        
                    default:
                        break;
                }
                if (_isRemoteAddressSet)
                    break;
            }
            break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            log_i("ESP_BT_GAP_DISC_STATE_CHANGED_EVT");
            break;

        case ESP_BT_GAP_RMT_SRVCS_EVT:
            log_i( "ESP_BT_GAP_RMT_SRVCS_EVT");
            break;

        case ESP_BT_GAP_RMT_SRVC_REC_EVT:
            log_i("ESP_BT_GAP_RMT_SRVC_REC_EVT");
            break;

        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                log_v("authentication success: %s", param->auth_cmpl.device_name);
            } else {
                log_e("authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;

        case ESP_BT_GAP_PIN_REQ_EVT:
            // default pairing pins
            log_i("ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                log_i("Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code;
                memset(pin_code, '0', ESP_BT_PIN_CODE_LEN);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                log_i("Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                memcpy(pin_code, "1234", 4);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
       
        case ESP_BT_GAP_CFM_REQ_EVT:
            log_i("ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;

        case ESP_BT_GAP_KEY_NOTIF_EVT:
            log_i("ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
            break;

        case ESP_BT_GAP_KEY_REQ_EVT:
            log_i("ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
            break;

        default:
            break;
    }
    */
}

static bool _init_bt(const char *deviceName)
{
    if (!btStarted() && !btStart()){
        log_e("initialize controller failed");
        return false;
    }
    
    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            log_e("initialize bluedroid failed");
            return false;
        }
    }
    
    if (bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            log_e("enable bluedroid failed");
            return false;
        }
    }

    if (_isMaster && esp_bt_gap_register_callback(esp_bt_gap_cb) != ESP_OK) {
        log_e("gap register failed");
        return false;
    }

    /*
    if (esp_spp_register_callback(esp_spp_cb) != ESP_OK){
        log_e("spp register failed");
        return false;
    }
    */

    /*
    if (esp_spp_init(ESP_SPP_MODE_CB) != ESP_OK){
        log_e("spp init failed");
        return false;
    }
    */

    if (esp_bt_sleep_disable() != ESP_OK){
        log_e("esp_bt_sleep_disable failed");
    }

    log_i("device name set");
    //esp_bt_dev_set_device_name(deviceName);

    if (_isPinSet) {
        btSetPin();
    }

    if (_enableSSP) {
        log_i("Simple Secure Pairing");
        //esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
        //esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
        //esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
    }

    // the default BTA_DM_COD_LOUDSPEAKER does not work with the macOS BT stack
    esp_bt_cod_t cod;
    cod.major = 0b00001;
    cod.minor = 0b000100;
    cod.service = 0b00000010110;
    if (esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD) != ESP_OK) {
        log_e("set cod failed");
        return false;
    }
    return true;
}

static bool _stop_bt()
{
    if (btStarted()){
        /*
        if(_spp_client)
            esp_spp_disconnect(_spp_client);
        esp_spp_deinit();
        */
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        btStop();
    }
    _spp_client = 0;
    return true;
}

static bool waitForConnect(int timeout) {
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return false;
    }
    wait(sc_time(timeout, SC_MS), btptr->connected_ev);
    if (btptr->connected_ev.triggered()) return true;
    else return false;
}

/*
 * Serial Bluetooth Arduino
 *
 * */

BluetoothSerial::BluetoothSerial()
{
    local_name = "ESP32"; //default bluetooth name
}

BluetoothSerial::~BluetoothSerial(void)
{
    _stop_bt();
}

bool BluetoothSerial::begin(String localName, bool isMaster)
{
    _isMaster = isMaster;
    if (localName.length()){
        local_name = localName;
    }
    return _init_bt(local_name.c_str());
}

int BluetoothSerial::available(void)
{
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return -1;
    }
    return BTserial.available();
}

int BluetoothSerial::peek(void)
{
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return -1;
    }
    return BTserial.peek();
}

bool BluetoothSerial::hasClient(void)
{
    return _spp_client > 0;
}

int BluetoothSerial::read(void)
{

    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return 0;
    }
    return BTserial.read();
}

size_t BluetoothSerial::write(uint8_t c)
{
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return 0;
    }
    return BTserial.write(c);
}

size_t BluetoothSerial::write(const uint8_t *buffer, size_t size)
{
    if (!_spp_client){
        return 0;
    }
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
       return 0;
    }
    return BTserial.write((char *)buffer, size);
}

void BluetoothSerial::flush()
{
    if(btptr == NULL) {
       PRINTF_ERROR("BSER", "BTPTR not setup");
    }
    else BTserial.flush();
}

void BluetoothSerial::end()
{
    _stop_bt();
}

esp_err_t BluetoothSerial::register_callback(esp_spp_cb_t * callback)
{
    custom_spp_callback = callback;
    return ESP_OK;
}

//Simple Secure Pairing
void BluetoothSerial::enableSSP() {
    _enableSSP = true;
}
/*
     * Set default parameters for Legacy Pairing
     * Use fixed pin code
*/
bool BluetoothSerial::setPin(const char *pin) {
    bool isEmpty =  !(pin  && *pin);
    if (isEmpty && !_isPinSet) {
        return true; // nothing to do
    } else if (!isEmpty){
        _pin_len = strlen(pin);
        memcpy(_pin_code, pin, _pin_len);
    } else {
        _pin_len = 0; // resetting pin to none (default)
    }
    _pin_code[_pin_len] = 0;
    _isPinSet = true;
    if (isReady(false, READY_TIMEOUT)) {
        btSetPin();
    }
    return true;
}

bool BluetoothSerial::connect(String remoteName)
{
    if (!isReady(true, READY_TIMEOUT)) return false;
    if (remoteName && remoteName.length() < 1) {
        log_e("No remote name is provided");
        return false; 
    }
    disconnect();
    _isRemoteAddressSet = false;
    strncpy(_remote_name, remoteName.c_str(), ESP_BT_GAP_MAX_BDNAME_LEN);
    _remote_name[ESP_BT_GAP_MAX_BDNAME_LEN] = 0;
    log_i("master : remoteName");
    // will first resolve name to address
    //esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
    //if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, INQ_LEN, INQ_NUM_RSPS) == ESP_OK) {
    //    return waitForConnect(SCAN_TIMEOUT);
    //}
    //return false;
    return true;
}

bool BluetoothSerial::connect(uint8_t remoteAddress[])
{
    if (!isReady(true, READY_TIMEOUT)) return false;
    if (!remoteAddress) {
        log_e("No remote address is provided");
        return false; 
    }
    disconnect();
    _remote_name[0] = 0;
    _isRemoteAddressSet = true;
    memcpy(_peer_bd_addr, remoteAddress, ESP_BD_ADDR_LEN);
    log_i("master : remoteAddress");
    /*
    if (esp_spp_start_discovery(_peer_bd_addr) == ESP_OK) {
        return waitForConnect(READY_TIMEOUT);
    }
    return false;
    */
    return true;
}

bool BluetoothSerial::connect()
{
    if (!isReady(true, READY_TIMEOUT)) return false;
    if (_isRemoteAddressSet){
        disconnect();
        // use resolved or set address first
        log_i("master : remoteAddress");
        //if (esp_spp_start_discovery(_peer_bd_addr) == ESP_OK) {
        //    return waitForConnect(READY_TIMEOUT);
        //}
        //return false;
        return true;
    } else if (_remote_name[0]) {
        disconnect();
        log_i("master : remoteName");
        // will resolve name to address first - it may take a while
        //esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        //if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, INQ_LEN, INQ_NUM_RSPS) == ESP_OK) {
        //    return waitForConnect(SCAN_TIMEOUT);
        //}
        //return false;
        return true;
    }
    log_e("Neither Remote name nor address was provided");
    return false;
}

bool BluetoothSerial::disconnect() {
    /*
    if (_spp_client) {
        flush();
        log_i("disconnecting");
        if (esp_spp_disconnect(_spp_client) == ESP_OK) {
            wait(sc_time(READY_TIMEOUT, SC_MS), btptr->disconnected_ev);
            if (btptr->disconnected_ev.triggered()) return true;
            else return false;
        }
    }
    return false;
    */
    return true;
}

bool BluetoothSerial::unpairDevice(uint8_t remoteAddress[]) {
    if (isReady(false, READY_TIMEOUT)) {
        log_i("removing bonded device");
        return (esp_bt_gap_remove_bond_device(remoteAddress) == ESP_OK);
    }
    return false;
}

bool BluetoothSerial::connected(int timeout) {
    return waitForConnect(timeout);
}

bool BluetoothSerial::isReady(bool checkMaster, int timeout) {
    if (checkMaster && !_isMaster) {
        log_e("Master mode is not active. Call begin(localName, true) to enable Master mode");
        return false;
    }
    if (!btStarted()) {
        log_e("BT is not initialized. Call begin() first");
        return false;
    }
    wait(sc_time(timeout, SC_MS), btptr->running_ev);
    if (btptr->running_ev.triggered()) return true;
    else return false;
}
#endif
