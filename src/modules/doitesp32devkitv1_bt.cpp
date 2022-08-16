/*******************************************************************************
 * doitesp32devkitv1_bt.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Implements a SystemC model of the doitESP32devkitV1 board.
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

#include <systemc.h>
#include "info.h"
#include "doitesp32devkitv1_bt.h"
#include "gpioset.h"
#include "lwip/sockets.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "Wire.h"
#include "driver/adc.h"
#include "reset_reason.h"
#include "soc/spi_struct.h"

void doitesp32devkit1_bt::pininit() {
   btptr = &i_bt;
   BTserial.setports(&i_bt.to, &i_bt.from);
   doitesp32devkitv1::pininit();
}

void doitesp32devkitv1_bt::trace(sc_trace_file *tf) {
   i_bt.trace(tf);
   sc_trace(tf, brx, brx.name());
   sc_trace(tf, btx, btx.name());
   doitesp32devkitv1::trace(tf);
}
