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

#include "esp32-hal-uart.h"
#include "esp32-hal.h"
#include "rom/ets_sys.h"
#include "esp_attr.h"
#include "soc/io_mux_reg.h"
#include "soc/uart_reg.h"
#include "soc/gpio_sig_map.h"
#include "TestSerial.h"
#include "uart.h"
#include <systemc.h>
#include "info.h"

#define UART_REG_BASE(u)    ((u==0)?DR_REG_UART_BASE:(      (u==1)?DR_REG_UART1_BASE:(    (u==2)?DR_REG_UART2_BASE:0)))
#define UART_RXD_IDX(u)     ((u==0)?U0RXD_IN_IDX:(          (u==1)?U1RXD_IN_IDX:(         (u==2)?U2RXD_IN_IDX:0)))
#define UART_TXD_IDX(u)     ((u==0)?U0TXD_OUT_IDX:(         (u==1)?U1TXD_OUT_IDX:(        (u==2)?U2TXD_OUT_IDX:0)))
#define UART_INTR_SOURCE(u) ((u==0)?ETS_UART0_INTR_SOURCE:( (u==1)?ETS_UART1_INTR_SOURCE:((u==2)?ETS_UART2_INTR_SOURCE:0)))

struct uart_struct_t {
    int num;
    TestSerial *t;
    uart *u;
};

TestSerial uart0(0);
TestSerial uart1(1);
TestSerial uart2(2);

static uart_struct_t _uart_bus_array[3] = {
    {0, &uart0, NULL},
    {1, &uart1, NULL},
    {2, &uart2, NULL},
};

void uartDetachRx(uart_struct_t* uart)
{
    if(uart == NULL) {
        return;
    }
    pinMatrixInDetach(UART_RXD_IDX(uart->num), false, false);
}

void uartDetachTx(uart_struct_t* uart)
{
    if(uart == NULL) {
        return;
    }
    pinMatrixOutDetach(UART_TXD_IDX(uart->num), false, false);
}

void uartAttachRx(uart_struct_t* uart, uint8_t rxPin, bool inverted)
{
    if(uart == NULL || rxPin > 39) {
        return;
    }
    pinMode(rxPin, INPUT);
    pinMatrixInAttach(rxPin, UART_RXD_IDX(uart->num), inverted);
}

void uartAttachTx(uart_struct_t* uart, uint8_t txPin, bool inverted)
{
    if(uart == NULL || txPin > 39) {
        return;
    }
    pinMode(txPin, OUTPUT);
    pinMatrixOutAttach(txPin, UART_TXD_IDX(uart->num), inverted, false);
}

uart_struct_t* uartBegin(uint8_t uart_nr, uint32_t baudrate, uint32_t config, int8_t rxPin, int8_t txPin, uint16_t queueLen, bool inverted)
{
    if(uart_nr > 2) {
        return NULL;
    }

    if(rxPin == -1 && txPin == -1) {
        return NULL;
    }

    uart_struct_t* uart = &_uart_bus_array[uart_nr];

    uartFlush(uart);
    uartSetBaudRate(uart, baudrate);
    uart->u->set_stop((config & UART_STOP_BIT_NUM_M) >> UART_STOP_BIT_NUM_S);

    if(rxPin != -1) {
        uartAttachRx(uart, rxPin, inverted);
    }

    if(txPin != -1) {
        uartAttachTx(uart, txPin, inverted);
    }

    return uart;
}

void uartEnd(uart_struct_t* uart)
{
    if(uart == NULL) {
        return;
    }

    uartDetachRx(uart);
    uartDetachTx(uart);
}

size_t uartResizeRxBuffer(uart_struct_t * uart, size_t new_size) {
    if(uart == NULL) {
        return 0;
    }

    SC_REPORT_WARNING("UART", "Currently you can't change the buffer size.");

    return new_size;
}

uint32_t uartAvailable(uart_struct_t* uart)
{
    if(uart == NULL) {
        return 0;
    }
    return uart->t->available();
}

uint32_t uartAvailableForWrite(uart_struct_t* uart)
{
    if(uart == NULL) {
        return 0;
    }
    return uart->t->availableForWrite();
}

uint8_t uartRead(uart_struct_t* uart)
{
    if(uart == NULL) {
        return 0;
    }
    return uart->t->read();
}

uint8_t uartPeek(uart_struct_t* uart)
{
    if(uart == NULL) {
        return 0;
    }
    return uart->t->peek();
}

void uartWrite(uart_struct_t* uart, uint8_t c)
{
    if(uart == NULL) {
        return;
    }
    uart->t->write(c);
}

void uartWriteBuf(uart_struct_t* uart, const uint8_t * data, size_t len)
{
    if(uart == NULL) {
        return;
    }
    uart->t->write((const char *)data, len);
}

void uartFlush(uart_struct_t* uart)
{
    uartFlushTxOnly(uart,false);
}

void uartFlushTxOnly(uart_struct_t* uart, bool txOnly)
{
    if(uart == NULL) {
        return;
    }
}

void uartSetBaudRate(uart_struct_t* uart, uint32_t baud_rate)
{
    if(uart == NULL) {
        return;
    }
    if(uart->u == NULL) {
       PRINTF_ERROR("UART", "Uart %d has not been initialized", uart->num);
    }
    uart->u->set_baud(baud_rate);
}

uint32_t uartGetBaudRate(uart_struct_t* uart)
{
    if(uart == NULL) {
        return 0;
    }
    if(uart->u == NULL) {
       PRINTF_ERROR("UART", "Uart %d has not been initialized", uart->num);
    }
    return uart->u->get_baud();
}

void uartSetDebug(uart_struct_t* uart)
{
    if(uart == NULL) {
        PRINTF_ERROR("UART", "Accessing NULL UART");
    }
    if(uart->u == NULL) {
       PRINTF_ERROR("UART", "Uart %d has not been initialized", uart->num);
    }
    /* This is not quite the intended code, but works for now. */
    uart->u->set_debug(true);
}

int uartGetDebug()
{
   if (_uart_bus_array[0].u == NULL) {
       PRINTF_ERROR("UART", "Uart 0 has not been initialized");
   }
   if (_uart_bus_array[1].u == NULL) {
       PRINTF_ERROR("UART", "Uart 1 has not been initialized");
   }
   if (_uart_bus_array[2].u == NULL) {
       PRINTF_ERROR("UART", "Uart 2 has not been initialized");
   }
   if (_uart_bus_array[0].u->get_debug()) return 0;
   else if (_uart_bus_array[1].u->get_debug()) return 1;
   else if (_uart_bus_array[2].u->get_debug()) return 2;
   else return 0;
}

/*
 * if enough pulses are detected return the minimum high pulse duration + minimum low pulse duration divided by two. 
 * This equals one bit period. If flag is true the function return inmediately, otherwise it waits for enough pulses.
 */
unsigned long uartBaudrateDetect(uart_struct_t *uart, bool flg)
{
    /* This function will wait for the first start bit and measure it. This
     * will be the autodetection.
     */
    if(uart == NULL) {
        PRINTF_ERROR("UART", "Accessing NULL UART");
    }
    if(uart->u == NULL) {
       PRINTF_ERROR("UART", "Uart %d has not been initialized", uart->num);
    }
    return uart->u->getautorate();
}

/*
 * To start detection of baud rate with the uart the auto_baud.en bit needs to be cleared and set. The bit period is 
 * detected calling uartBadrateDetect(). The raw baudrate is computed using the UART_CLK_FREQ. The raw baudrate is 
 * rounded to the closed real baudrate.
*/
void uartStartDetectBaudrate(uart_struct_t *uart) {
    if(uart == NULL) {
        PRINTF_ERROR("UART", "Accessing NULL UART");
    }
    if(uart->u == NULL) {
       PRINTF_ERROR("UART", "Uart %d has not been initialized", uart->num);
    }
    uart->u->setautodetect();
}

unsigned long
uartDetectBaudrate(uart_struct_t *uart)
{
   return uartBaudrateDetect(uart, false);
}

/*
 * Returns the status of the RX state machine, if the value is non-zero the state machine is active.
 */
bool uartRxActive(uart_struct_t* uart) {
   SC_REPORT_WARNING("UARTCPP", "The rx active is not yet supported.");
   return false;
}

/* We need to set the uart to point to the right module. */
void uartInit(int modn, void *_to, void *_from, void *_mod) {
   if (modn > 2) {
      PRINTF_ERROR("UARTCPP", "Illegal uart %d", modn);
      return;
   }
   _uart_bus_array[modn].t->setports((sc_fifo<unsigned char> *)_to,
      (sc_fifo<unsigned char> *)_from);
   _uart_bus_array[modn].u = (uart *)_mod;
}

int uartReadTimeout(uart_struct_t* uart, unsigned long int tmout)
{
    if(uart == NULL) {
        return -1;
    }
    return uart->t->bl_read(sc_time(tmout, SC_MS));
}

int uartPeekTimeout(uart_struct_t* uart, unsigned long int tmout)
{
    if(uart == NULL) {
        return -1;
    }
    return uart->t->bl_peek(sc_time(tmout, SC_MS));
}
