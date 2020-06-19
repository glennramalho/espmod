/*******************************************************************************
 * mux_out.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Model for the input signal mux for the GPIO Matrix.
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

#ifndef _MUX_OUT_H
#define _MUX_OUT_H

#include <systemc.h>

#define MUXOUT_DRIVE_0 256
#define MUXOUT_DRIVE_1 257
#define MUXOUT_DRIVE_Z 258

SC_MODULE(mux_out) {
   sc_out<bool> mout_o {"mout_o"};
   sc_out<bool> men_o {"men_o"};
   sc_in<bool> uart0tx_i{"uart0tx_i"};
   sc_in<bool> uart1tx_i{"uart1tx_i"};
   sc_in<bool> uart2tx_i{"uart2tx_i"};
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_hs_i{"ledc_sig_hs_i"};
   sc_port<sc_signal_in_if<bool>,8> ledc_sig_ls_i{"ledc_sig_ls_i"};
   sc_in<bool> hspi_d_oen_i {"hspi_d_oen_i"};
   sc_in<bool> hspi_d_out_i {"hspi_d_out_i"};
   sc_in<bool> hspi_q_oen_i {"hspi_q_oen_i"};
   sc_in<bool> hspi_q_out_i {"hspi_q_out_i"};
   sc_in<bool> hspi_cs0_oen_i {"hspi_cs0_oen_i"};
   sc_in<bool> hspi_cs0_out_i {"hspi_cs0_out_i"};
   sc_in<bool> hspi_clk_oen_i {"hspi_clk_oen_i"};
   sc_in<bool> hspi_clk_out_i {"hspi_clk_out_i"};
   sc_in<bool> hspi_wp_oen_i {"hspi_wp_oen_i"};
   sc_in<bool> hspi_wp_out_i {"hspi_wp_out_i"};
   sc_in<bool> hspi_hd_oen_i {"hspi_hd_oen_i"};
   sc_in<bool> hspi_hd_out_i {"hspi_hd_out_i"};
   sc_in<bool> vspi_d_oen_i {"vspi_d_oen_i"};
   sc_in<bool> vspi_d_out_i {"vspi_d_out_i"};
   sc_in<bool> vspi_q_oen_i {"vspi_q_oen_i"};
   sc_in<bool> vspi_q_out_i {"vspi_q_out_i"};
   sc_in<bool> vspi_cs0_oen_i {"vspi_cs0_oen_i"};
   sc_in<bool> vspi_cs0_out_i {"vspi_cs0_out_i"};
   sc_in<bool> vspi_cs1_oen_i {"vspi_cs1_oen_i"};
   sc_in<bool> vspi_cs1_out_i {"vspi_cs1_out_i"};
   sc_in<bool> vspi_cs2_oen_i {"vspi_cs2_oen_i"};
   sc_in<bool> vspi_cs2_out_i {"vspi_cs2_out_i"};
   sc_in<bool> vspi_clk_oen_i {"vspi_clk_oen_i"};
   sc_in<bool> vspi_clk_out_i {"vspi_clk_out_i"};
   sc_in<bool> vspi_wp_oen_i {"vspi_wp_oen_i"};
   sc_in<bool> vspi_wp_out_i {"vspi_wp_out_i"};
   sc_in<bool> vspi_hd_oen_i {"vspi_hd_oen_i"};
   sc_in<bool> vspi_hd_out_i {"vspi_hd_out_i"};

   int function;
   sc_event fchange_ev;

   /* Functions */
   void mux(int funcsel);

   /* Threads */
   void transfer(void);

   SC_CTOR(mux_out) {
      function = MUXOUT_DRIVE_Z;
      SC_THREAD(transfer);
   }
};

#endif
