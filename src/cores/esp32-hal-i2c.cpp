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

#include "esp32-hal-i2c.h"
#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp_attr.h"
#include "esp32-hal-cpu.h" // cpu clock change support 31DEC2018
//#define I2C_DEV(i)   (volatile i2c_dev_t *)((i)?DR_REG_I2C1_EXT_BASE:DR_REG_I2C_EXT_BASE)
//#define I2C_DEV(i)   ((i2c_dev_t *)(REG_I2C_BASE(i)))
#define I2C_SCL_IDX(p)  ((p==0)?I2CEXT0_SCL_OUT_IDX:((p==1)?I2CEXT1_SCL_OUT_IDX:0))
#define I2C_SDA_IDX(p) ((p==0)?I2CEXT0_SDA_OUT_IDX:((p==1)?I2CEXT1_SDA_OUT_IDX:0))

#define DR_REG_I2C_EXT_BASE_FIXED               0x60013000
#define DR_REG_I2C1_EXT_BASE_FIXED              0x60027000

struct i2c_struct_t {
//    i2c_dev_t * dev;
//#if !CONFIG_DISABLE_HAL_LOCKS
//    xSemaphoreHandle lock;
//#endif
    uint8_t num;
    int8_t sda;
    int8_t scl;
//    I2C_MODE_t mode;
//    I2C_STAGE_t stage;
//    I2C_ERROR_t error;
//    EventGroupHandle_t i2c_event; // a way to monitor ISR process
    // maybe use it to trigger callback for OnRequest()
//    intr_handle_t intr_handle;       /*!< I2C interrupt handle*/
//    I2C_DATA_QUEUE_t * dq;
    uint16_t queueCount; // number of dq entries in queue.
    uint16_t queuePos; // current queue that still has or needs data (out/in)
    int16_t  errorByteCnt;  // byte pos where error happened, -1 devId, 0..(length-1) data
    uint16_t errorQueue; // errorByteCnt is in this queue,(for error locus)
    uint32_t exitCode;
    uint32_t debugFlags;
};


static i2c_t _i2c_bus_array[2] = {
    {0, -1, -1,0,0,0,0,0},
    {1, -1, -1,0,0,0,0,0}
};

//static i2c_t _i2c_bus_array[2] = {
//    {(volatile i2c_dev_t *)(DR_REG_I2C_EXT_BASE_FIXED), 0, -1, -1,I2C_NONE,I2C_NONE,I2C_ERROR_OK,NULL,NULL,NULL,0,0,0,0,0},
//    {(volatile i2c_dev_t *)(DR_REG_I2C1_EXT_BASE_FIXED), 1, -1, -1,I2C_NONE,I2C_NONE,I2C_ERROR_OK,NULL,NULL,NULL,0,0,0,0,0}
//};

i2c_err_t i2cAttachSCL(i2c_t * i2c, int8_t scl)
{
    if(i2c == NULL) {
        return I2C_ERROR_DEV;
    }
    digitalWrite(scl, HIGH);
    pinMode(scl, OPEN_DRAIN | PULLUP | INPUT | OUTPUT);
    pinMatrixOutAttach(scl, I2C_SCL_IDX(i2c->num), false, false);
    pinMatrixInAttach(scl, I2C_SCL_IDX(i2c->num), false);
    return I2C_ERROR_OK;
}

i2c_err_t i2cDetachSCL(i2c_t * i2c, int8_t scl)
{
    if(i2c == NULL) {
        return I2C_ERROR_DEV;
    }
    pinMatrixOutDetach(scl, false, false);
    pinMatrixInDetach(I2C_SCL_IDX(i2c->num), false, false);
    pinMode(scl, INPUT | PULLUP);
    return I2C_ERROR_OK;
}

i2c_err_t i2cAttachSDA(i2c_t * i2c, int8_t sda)
{
    if(i2c == NULL) {
        return I2C_ERROR_DEV;
    }
    digitalWrite(sda, HIGH);
    pinMode(sda, OPEN_DRAIN | PULLUP | INPUT | OUTPUT );
    pinMatrixOutAttach(sda, I2C_SDA_IDX(i2c->num), false, false);
    pinMatrixInAttach(sda, I2C_SDA_IDX(i2c->num), false);
    return I2C_ERROR_OK;
}

i2c_err_t i2cDetachSDA(i2c_t * i2c, int8_t sda)
{
    if(i2c == NULL) {
        return I2C_ERROR_DEV;
    }
    pinMatrixOutDetach(sda, false, false);
    pinMatrixInDetach(I2C_SDA_IDX(i2c->num), false, false);
    pinMode(sda, INPUT | PULLUP);
    return I2C_ERROR_OK;
}

/*
 * PUBLIC API
 * */
// 24Nov17 only supports Master Mode
i2c_t * i2cInit(uint8_t i2c_num, int8_t sda, int8_t scl, uint32_t frequency) {
#ifdef ENABLE_I2C_DEBUG_BUFFER
  log_v("num=%d sda=%d scl=%d freq=%d",i2c_num, sda, scl, frequency);
#endif
    if(i2c_num > 1) {
        return NULL;
    }

    i2c_t * i2c = &_i2c_bus_array[i2c_num];

    // pins should be detached, else glitch
    if(i2c->sda >= 0){
        i2cDetachSDA(i2c, i2c->sda);
    }
    if(i2c->scl >= 0){
        i2cDetachSCL(i2c, i2c->scl);
    }
    i2c->sda = sda;
    i2c->scl = scl;

//#if !CONFIG_DISABLE_HAL_LOCKS
//    if(i2c->lock == NULL) {
//        i2c->lock = xSemaphoreCreateRecursiveMutex();
//        if(i2c->lock == NULL) {
//            return NULL;
//        }
//    }
//#endif
//    I2C_MUTEX_LOCK();

//    i2cReleaseISR(i2c); // ISR exists, release it before disabling hardware

//    if(frequency == 0) {// don't change existing frequency
//        frequency = i2cGetFrequency(i2c);
//        if(frequency == 0) {
//            frequency = 100000L;    // default to 100khz
//        }
//    }

  //  if(i2c_num == 0) {
  //      DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT0_RST); //reset hardware
  //      DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG,DPORT_I2C_EXT0_CLK_EN);
  //      DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT0_RST);//  release reset
  //  } else {
  //      DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT1_RST); //reset Hardware
  //      DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG,DPORT_I2C_EXT1_CLK_EN);
  //      DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT1_RST);
  //  }
  //  i2c->dev->ctr.val = 0;
  //  i2c->dev->ctr.ms_mode = 1;
  //  i2c->dev->ctr.sda_force_out = 1 ;
  //  i2c->dev->ctr.scl_force_out = 1 ;
  //  i2c->dev->ctr.clk_en = 1;

    //the max clock number of receiving  a data
  //  i2c->dev->timeout.tout = 400000;//clocks max=1048575
    //disable apb nonfifo access
  //  i2c->dev->fifo_conf.nonfifo_en = 0;

  //  i2c->dev->slave_addr.val = 0;
  //  I2C_MUTEX_UNLOCK();

  //  i2cSetFrequency(i2c, frequency);    // reconfigure

  //  if(!i2cCheckLineState(i2c->sda, i2c->scl)){
  //      return NULL;
  //  }

    if(i2c->sda >= 0){
        i2cAttachSDA(i2c, i2c->sda);
    }
    if(i2c->scl >= 0){
        i2cAttachSCL(i2c, i2c->scl);
    }
    return i2c;
}

void i2cRelease(i2c_t *i2c)  // release all resources, power down peripheral
{
 //   I2C_MUTEX_LOCK();

    if(i2c->sda >= 0){
        i2cDetachSDA(i2c, i2c->sda);
    }
    if(i2c->scl >= 0){
        i2cDetachSCL(i2c, i2c->scl);
    }

  //  i2cReleaseISR(i2c);

 //   if(i2c->i2c_event) {
 //       vEventGroupDelete(i2c->i2c_event);
 //       i2c->i2c_event = NULL;
 //   }

 //   i2cFlush(i2c);

    // reset the I2C hardware and shut off the clock, power it down.
 //   if(i2c->num == 0) {
 //       DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT0_RST); //reset hardware
 //       DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG,DPORT_I2C_EXT0_CLK_EN); // shutdown hardware
 //   } else {
 //       DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2C_EXT1_RST); //reset Hardware
 //       DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG,DPORT_I2C_EXT1_CLK_EN); // shutdown Hardware
 //   }

 //   I2C_MUTEX_UNLOCK();
}
