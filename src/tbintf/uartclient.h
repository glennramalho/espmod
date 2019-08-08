/*******************************************************************************
 * uartclient.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a model for a generic UART client. It is intended to be used
 *   to ease testing UART interfaces. Simply connect it to the corresponding
 *   signals and then use the tasks in it to drive the test.
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

#ifndef _UARTCLIENT_H
#define _UARTCLIENT_H
#include <systemc.h>
#include "netcon.h"
#include "uart.h"

SC_MODULE(uartclient) {
   sc_inout<gn_mixed> rx {"rx"};
   sc_inout<gn_mixed> tx {"tx"};
   sc_signal<bool> rx_dig {"rx_dig"};
   sc_signal<bool> tx_dig {"tx_dig"};

   /* Receiving Uarts */
   uart i_uart {"i_uart", 64, 64};

   /* The uarts do not handle RV vectors */
   netcon_mixtobool i_uart_rx_netcon {"i_uart_rx_netcon"};
   netcon_booltomix i_uart_tx_netcon {"i_uart_tx_netcon"};

   /* Tasks */
   void send(const char *string);
   std::string get();
   void expect(const char *string);
   void dump();

   SC_CTOR(uartclient) {
      /* We connect the netcons RV side to the pins. */
      i_uart_rx_netcon.a(rx); i_uart_rx_netcon.b(rx_dig);
      i_uart_tx_netcon.b(tx); i_uart_tx_netcon.a(tx_dig);
      i_uart.rx(rx_dig);
      i_uart.tx(tx_dig);
   }
};

#endif
