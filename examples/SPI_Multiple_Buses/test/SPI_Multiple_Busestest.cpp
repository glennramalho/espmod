/*******************************************************************************
 * SPI_Multiple_Busestest.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the main testbench for the SPI_Multiple_Buses.ino test.
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
#include "SPI_Multiple_Busestest.h"
#include <string>
#include <vector>
#include "info.h"


/**********************
 * Function: trace()
 * inputs: trace file
 * outputs: none
 * return: none
 * globals: none
 *
 * Traces all signals in the design. For a signal to be traced it must be listed
 * here. This function should also call tracing in any subblocks, if desired.
 */
void SPI_Multiple_Busestest::trace(sc_trace_file *tf) {
   sc_trace(tf, led, led.name());
   sc_trace(tf, rx, rx.name());
   sc_trace(tf, tx, tx.name());
   sc_trace(tf, vspi_sck, vspi_sck.name());
   sc_trace(tf, vspi_miso, vspi_miso.name());
   sc_trace(tf, vspi_mosi, vspi_mosi.name());
   sc_trace(tf, vspi_ss, vspi_ss.name());
   sc_trace(tf, hspi_sck, hspi_sck.name());
   sc_trace(tf, hspi_miso, hspi_miso.name());
   sc_trace(tf, hspi_mosi, hspi_mosi.name());
   sc_trace(tf, hspi_ss, hspi_ss.name());
   i_esp.trace(tf);
}

/**********************
 * Task: serflush():
 * inputs: none
 * outputs: none
 * return: none
 * globals: none
 *
 * Dumps everything comming from the serial interface.
 */
void SPI_Multiple_Busestest::serflush() {
   i_uartclient.dump();
}

/*******************************************************************************
** Testbenches *****************************************************************
*******************************************************************************/

void SPI_Multiple_Busestest::t0(void) {
   SC_REPORT_INFO("TEST", "Running Test T0.");
   wait(1, SC_MS); sc_stop();

   PRINTF_INFO("TEST", "Waiting for power-up");
   wait(500, SC_MS);
}

void SPI_Multiple_Busestest::testbench(void) {
   /* Now we check the test case and run the correct TB. */
   printf("Starting Testbench Test%d @%s\n", tn,
      sc_time_stamp().to_string().c_str());

   if (tn == 0) t0();
   else SC_REPORT_ERROR("TEST", "Test number too large.");

   sc_stop();
}
