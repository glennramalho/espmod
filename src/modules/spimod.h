/*******************************************************************************
 * spimod.h -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Models a single ESP32 SPI
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

#ifndef _SPIMOD_H
#define _SPIMOD_H

SC_MODULE(spimod) {
   public:
   sc_out<bool> d_oen_o {"d_oen_o"};
   sc_out<bool> d_o {"d_o"};
   sc_in<bool> d_i {"d_i"};
   sc_out<bool> q_oen_o {"q_oen_o"};
   sc_out<bool> q_o {"q_o"};
   sc_in<bool> q_i {"q_i"};
   sc_out<bool> cs0_oen_o {"cs0_oen_o"};
   sc_out<bool> cs0_o {"cs0_o"};
   sc_in<bool> cs0_i {"cs0_i"};
   sc_out<bool> cs1_oen_o {"cs1_oen_o"};
   sc_out<bool> cs1_o {"cs1_o"};
   sc_in<bool> cs1_i {"cs1_i"};
   sc_out<bool> cs2_oen_o {"cs2_oen_o"};
   sc_out<bool> cs2_o {"cs2_o"};
   sc_in<bool> cs2_i {"cs2_i"};
   sc_out<bool> clk_oen_o {"clk_oen_o"};
   sc_out<bool> clk_o {"clk_o"};
   sc_in<bool> clk_i {"clk_i"};
   sc_out<bool> wp_oen_o {"wp_oen_o"};
   sc_out<bool> wp_o {"wp_o"};
   sc_in<bool> wp_i {"wp_i"};
   sc_out<bool> hd_oen_o {"hd_oen_o"};
   sc_out<bool> hd_o {"hd_o"};
   sc_in<bool> hd_i {"hd_i"};

   /* Registers */
   sc_signal<uint32_t> ctrl {"ctrl"};
   sc_signal<uint32_t> ctrl2 {"ctrl2"};
   sc_signal<uint32_t> clock {"clock"};
   sc_signal<uint32_t> pin {"pin"};
   sc_signal<uint32_t> slave {"slave"};
   sc_signal<uint32_t> slave1 {"slave1"};
   sc_signal<uint32_t> slave2 {"slave2"};
   sc_signal<uint32_t> slv_wr_status {"slv_wr_status"};
   sc_signal<uint32_t> slv_wrbuf_dlen {"slv_wrbuf_dlen"};
   sc_signal<uint32_t> slv_rdbuf_dlen {"slv_rdbuf_dlen"};
   sc_signal<uint32_t> slv_rd_bit {"slv_rd_bit"};
   sc_signal<uint32_t> cmd {"cmd"};
   sc_signal<uint32_t> addr {"addr"};
   sc_signal<uint32_t> user {"user"};
   sc_signal<uint32_t> user1 {"user1"};
   sc_signal<uint32_t> user2 {"user2"};
   sc_signal<uint32_t> mosi_dlen {"mosi_dlen"};
   sc_signal<uint32_t> miso_dlen {"miso_dlen"};

   /* Signals */
   sc_signal<bool> master {"master"};

   /* Variables */
   unsigned int buffer[16];
   spi_struct *spistruct;
   int dlywr, dlyrd;
   int limitbit, limitrdbit;
   int startbit, startrdbit;
   bool wrlittleendian, rdlittleendian;
   bool wrmsbfirst, rdmsbfirst;
   sc_event start_ev, reset_ev, loweruser_ev;

   /* Simulation Interface Functions */
   void update();
   void trace(sc_trace_file *tf);

   /* Internal Functions */
   private:
   void initstruct();
   void start_of_simulation();
   int calcnextbit(int bitpos, bool littleendian, bool msbfirst);
   int converttoendian(bool littleendian, bool msbfirst, int pos);
   int getdly(int mode);
   void activatecs(bool withdelay);
   void deactivatecs(bool withdelay);
   void setupmasterslave();
   void setupmaster();
   void setupslave();

   /* Threads */
   public:
   void update_th();
   void return_th();
   void transfer_th();
   void configure_meth();

   // Constructor
   spimod(sc_module_name name, spi_struct *_spistruct): sc_module(name) {
      intistruct(_spistruct);

      SC_THREAD(update_th);
      sensitive << update_ev << lowerusrbit_ev;

      SC_THREAD(return_th);
      sensitive << updategpioreg_ev << updategpiooe_ev << update_ev;

      SC_THREAD(transfer_th);

      SC_METHOD(configure_meth);
      sensitive << reset_ev << slave;
   }
   SC_HAS_PROCESS(spimod);
};
extern spimod *hspiptr;
extern spimod *vspiptr;

#endif
