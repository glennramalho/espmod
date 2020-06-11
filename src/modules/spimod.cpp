/*******************************************************************************
 * spimod.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 SPI module.
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
#include "spimod.h"
#include "setfield.h"
#include "soc/spi_struct.h"
#include "soc/spi_reg.h"
#include "clockpacer.h"
#include "info.h"

void spimod::update_th() {
   while(true) {
      wait();

      /* We check the pointer. If it is null or went null, something went wrong, so
       * we raise an alarm.
       */
      if (spistruct == NULL) {
         SC_REPORT_FATAL("SPIMOD", "spistruct pointer is not set");
      }

      /* If we got a lower USR bit, we lower it. */
      if (lowerusrbit_ev.triggered) {
         *spistruct.cmd.usr = 0;
         WRFIELD(cmd, SPI_USR_M, SPI_USR_S) = 0;
      }

      /* In other cases, we take in the updated bits. */
      if (update_ev.triggered()) {
         /* Most registers we simply copy in. This is useful for debugging issues. */
         cmd.write(*spistruct.cmd.val);
         addr.write(*spistruct.addr.val);
         ctrl.write(*spistruct.ctrl.val);
         ctrl2.write(*spistruct.ctrl2.val);
         clock.write(*spistruct.clock.val);
         user.write(*spistruct.user.val);
         user1.write(*spistruct.user1.val);
         user2.write(*spistruct.user2.val);
         mosi_dlen.write(*spistruct.mosi_dlen.val);
         miso_dlen.write(*spistruct.miso_dlen.val);
         pin.write(*spistruct.pin.val);
         slave.write(*spistruct.slave.val);
         slave1.write(*spistruct.slave1.val);
         slave2.write(*spistruct.slave2.val);
         slv_wrbuf_dlen.write(*spistruct.slv_wrbuf_dlen.val);
         slv_rdbuf_dlen.write(*spistruct.slv_rdbuf_dlen.val);
         slv_rd_bit.write(*spistruct.slv_rd_bit.val);

         /* We also trigger the events, if needed. */
         if (*spistruct.cmd.usr) start_ev.notify();
         if (*spistruct.slave.sync_reset) reset_ev.notify();
      }
   }
}

void spimod::return_th() {
   int un;
   /* We wait for one of the responses to get triggered, and then we copy the
    * R/O register back to the struct.
    */
   while (true) {
      wait(slv_wr_status.value_changed_event());

      *spistruct.slv_wr_status.val = slv_wr_status.read();
   }
}

void spimod::update() {
   update_ev.notify();
   clockpacer.wait_next_apb_clk();
}

void spimod::initstruct(spi_struct *_spistruct) {
   spistruct = _spistruct;
   memset(spistruct, 0, sizeof(spi_struct));
   actclk = false;
}

void spimod::start_of_simulation() {
   /* We spawn a thread for each channel and timer. */
   int un;
   int_ena.write(0); ctrl.write(0); ctrl2.write(0);
   clock.write(0);
   pin.write(0);
   slave.write(0); slave1.write(0); slave2.write(0);
   slv_wr_status.write(0); slv_wrbuf_dlen.write(0); slv_rdbuf_dlen.write(0);
   slv_rd_bit.write(0);
   cmd.write(0); addr.write(0); user.write(0); user1.write(0); user2.write(0);
   mosi_dlen.write(0);

   d_oen_o.write(false); d_o.write(false);
   q_oen_o.write(false); q_o.write(false);
   cs0_oen_o.write(false); cs0_o.write(false);
   cs1_oen_o.write(false); cs1_o.write(false);
   cs2_oen_o.write(false); cs2_o.write(false);
   clk_oen_o.write(false); clk_o.write(false);
   wp_oen_o.write(false); wp_o.write(false);
   hd_oen_o.write(false); hd_o.write(false);
}

void spimod::calcnextbit(int bitpos, bool littleendian, bool msbfirst) {
   /* For MSB we start with the top bit in each byte. */
   if (msbfirst) {
      /* If we are in the last bit in a byte, we need to know which byte we go to
       * next. For little endian, we just advance, but for bigendian we need to
       * do some jumping.
       */
      if ((bitpos & 0x7) == 0x0 && littleendian) bitpos = bitpos + 8;
      else if ((bitpos & 0x7) == 0x0 && (bitpos & 0x18) == 0x0) bitpos = bitpos + 63;
      else if ((bitpos & 0x7) == 0x0) bitpos = bitpos - 1;
      /* If we are not changing byte, we decrement. */
      else bitpos = bitpos - 1;
   }
   /* For LSB first, we start from bit zero. */
   else {
      /* If we are in the last bit in a byte, we need to know which byte we go to
       * next. For little endian, we just advance, but for bigendian we need to
       * do some jumping.
       */
      if ((bitpos & 0x7) == 0x7 && littleendian) bitpos = bitpos + 1;
      else if ((bitpos & 0x7) == 0x7 && (bitpos & 0x18) == 0x0) bitpos = bitpos + 48;
      else if ((bitpos & 0x7) == 0x7) bitpos = bitpos - 15;
      /* If we are not changing byte, we just increment. */
      else bitpos = bitpos + 1;
   }
}

int spimod::converttoendian(littleendian, msbfirst, pos) {
   /* First we check the msb/lsb. For this we jump to the end of the selected bit.*/
   if (msbfirst) pos = pos & ~0x7 | (7 - (pos & 0x7));

   /* Now we handle the byte order. */
   if (!littleendian) return pos = (pos & ~0x18) | ~(pos & 0x18);

   return pos;
}

int spimod::getdly(int mode) {
   bool ck_out_or_i_out =
      RDFIELD(user, SPI_CK_OUT_EDGE_M, SPI_CK_OUT_EDGE_M) 
      || RDFIELD(user, SPI_I_OUT_EDGE_M, SPI_I_OUT_EDGE_M);
   int dlyout;

   if (mode == 0) dlyout = 0;
   else if (mode == 1 && ck_out_or_i_out) dlyout = 1;
   else if (mode == 2 && !ck_out_or_i_out) dlyout = 1;
   else dlyout = 2;

   return dlyout;
}

void spimod::activatecs(bool withdelay) {
   if (!RDFIELD(pin, SPI_CS2_DIS_M, SPI_CS2_DIS_S)) cs2_o.write(false);
   else cs0_o.write(true);
   if (!RDFIELD(pin, SPI_CS1_DIS_M, SPI_CS1_DIS_S)) cs1_o.write(false);
   else cs1_o.write(true);
   if (!RDFIELD(pin, SPI_CS0_DIS_M, SPI_CS0_DIS_S)) cs0_o.write(false);
   else cs2_o.write(true);

   /* We wait the setup time, if one was defined */
   if (withdelay && RDFIELD(user, SPI_CS_SETUP_M, SPI_CS_SETUP_S)) {
      wait(period * RDFIELD(ctrl2, SPI_SETUP_M, SPI_SETUP_S));
   }
}

void spimod::deactivatecs(bool withdelay) {
   /* If there is a delay to switch off we wait for it too. */
   if (withdelay && RDFIELD(user, SPI_CS_HOLD_M, SPI_CS_HOLD_S)) {
      wait(period * RDFIELD(ctrl2, SPI_HOLD_M, SPI_HOLD_S));
   }

   if (!RDFIELD(pin, SPI_CS2_DIS_M, SPI_CS2_DIS_S)) cs2_o.write(false);
   else cs0_o.write(true);
   if (!RDFIELD(pin, SPI_CS1_DIS_M, SPI_CS1_DIS_S)) cs1_o.write(false);
   else cs1_o.write(true);
   if (!RDFIELD(pin, SPI_CS0_DIS_M, SPI_CS0_DIS_S)) cs0_o.write(false);
   else cs2_o.write(true);
}

void spimod::setupmasterslave() {
   clockpacer.wait_next_apb_clk();

   /* Whenever we get a reset, we lower all data lines. */
   d_o.write(false);
   q_o.write(false);
   wp_o.write(false);
   hd_o.write(false);

   /* We get the idle clock edge style and set it as the clock should be
    * idle now. Note: the variable has the active time, not the idle time.
    */
   actclk = RDFIELD(pin, SPI_CK_IDLE_EDGE_M, SPI_CK_IDLE_EDGE_S) == 0;
   clk_o.write(!actclk);

   /* If we are in master mode and the customer set the keepactive bit, we set the
    * CS bits immediately.
    */
   if (master.read() && RDFIELD(pin, SPI_CS_KEEP_ACTIVE_M, SPI_CS_KEEP_ACTIVE_S))
      activate_cs(false);
   /* In slave mode, we shut off the CS bits. Not really necessary as we have
    * already changed the CS OE bits, but it is nice to do.
    */
   else deactivatecs(false);

   /* Not sure if this is needed, but it is nice to have. When a reset is issued,
    * we lower the USR bit.
    */
   lowerusrbit_ev.notify();
}

void spimod::setupmaster() {
   /* First we check if the clock is to be the SYSCLK. */
   if (RDFIELD(clock, SPI_CLK_EQU_SYSCLK_M, SPI_CLK_EQU_SUSCLK_S)) {
      /* The system clock for the SPI is always the APB clock. */
      period = clockpacer.get_apb_period();
      halftime = sc_time(clockpacer.get_apb_period()/2);
      lowtime = period - halftime;
   }
   /* If not, we get the clock configuraion. */
   else {
      period = sc_time(clockpacer.get_apb_period() *
        (RDFIELD(clock, SPI_CLKDIV_PRE_M, SPI_CLKDIV_PRE_S) + 1)
        (RDFIELD(clock, SPI_CLKDIV_N_M, SPI_CLKDIV_N_S) + 1));
      hightime = sc_time(clockpacer.get_apb_period() *
        (RDFIELD(clock, SPI_CLKDIV_PRE_M, SPI_CLKDIV_PRE_S) + 1)
        (RDFIELD(clock, SPI_CLKDIV_H_M, SPI_CLKDIV_H_S) + 1));
      /* We ignore the L field as it must be equal to the N register. */
      lowtime = period - halftime;
   }

   /* We need to determine where the data when data will change relative
    * to the clock. The MOSI and MISO have the same rules but they come from
    * different registers. Note that there are many possible illegal combinations.
    * Firmware has to get it right.
    */
   dlywr = getdly(RDFIELD(ctrl2, SPI_MOSI_DELAY_M, SPI_MOSI_DELAY_MODE_S));
   dlyrd = getdly(RDFIELD(ctrl2, SPI_MISO_DELAY_M, SPI_MISO_DELAY_MODE_S));

   /* We also have these extra delays to add to the beginning of a transfer. */
   precycwr = RDFIELD(ctrl2, SPI_MOSI_DELAY_NUM_M, SPI_MOSI_DELAY_NUM_S);
   precycrd = RDFIELD(ctrl2, SPI_MISO_DELAY_NUM_M, SPI_MISO_DELAY_NUM_S);

   /* If the field is enabled, we can transmit it. */
   if (RDFIELD(user, SPI_MOSI_M, SPI_MOSI_S) == 0) {
      limitbit = -1;
      startbit = -1;
   }
   else {
      wrlittleendian = RDFIELD(user, SPI_WR_BYTE_ORDER_M, SPI_WR_BYTE_ORDER_S) == 0;
      wrmsbfirst = RDFIELD(ctrl, SPI_WR_BIT_ORDER_M, SPI_WR_BIT_ORDER_S) == 0;
      /* For the start bit, we guess for the LSB firsrt/little endian case and then
       * we adjust the value.
       */
      startbit = 32 * ((RDFIELD(user,
         SPI_USR_MOSI_HIGHPART_M, SPI_USR_MOSI_HIGHPART_S) == 1)?8:0);
      lastbitrd = converttoendian(wrlittleendian, wrmsbfirst,
            startbit + mosi_dlen.read());
      startbitrd = converttoendian(wrlittleendian, wrmsbfirst, startbit);
   }

   /* We do the same for reading. */
   if (RDFIELD(user, SPI_MISO_M, SPI_MISO_S) == 0) {
      limitbitrd = -1;
      startbitrd = -1;
   }
   else {
      rdlittleendian = RDFIELD(user, SPI_RD_BYTE_ORDER_M, SPI_RD_BYTE_ORDER_S) == 0;
      rdmsbfirst = RDFIELD(ctrl, SPI_RD_BIT_ORDER_M, SPI_RD_BIT_ORDER_S) == 0;
      /* For the start bit, we guess for the LSB firsrt/little endian case and then
       * we adjust the value.
       */
      startbitrd = 32 * ((RDFIELD(user,
         SPI_USR_MISO_HIGHPART_M, SPI_USR_MISO_HIGHPART_S) == 1)?8:0);
      lastbitrd = converttoendian(rdlittleendian, rdmsbfirst,
            startbitrd + miso_dlen.read());
      startbitrd = converttoendian(rdlittleendian, rdmsbfirst, startbitrd);
   }
}

void spimod::setupslave() {
   SC_REPORT_FATAL("SPIMOD", "SPI Slave mode not yet ready");
}

void spimod::transfer_th() {
   bool bittosend;
   bool bitreceived;

   /* We start off setting bit to -1 as that indicates there is no transmission in
    * progress.
    */
   int bit = -1;
   int bitwr = -1;
   while(1) {
      /* If we are not enabled, we need to wait for a start. This only happens on the
       * beginning. We also accept reset and master change.
       */
      if (running == false) wait(start_ev | reset_ev | master);

      /* We now have to check for master or reset events. We treat both as resets. */
      if (master.event() || reset_ev.triggered()) {
         setupmasterslave();
         bit = -1;
         continue;
      }

      /* If we are starting a new command, we need to reset the bit counters and
       * perhaps activate the cs bits.
       */
      if (start_ev.triggered()) {
         clockpacer.wait_next_apb_clk();
         if (master.read()) setupmaster();
         else setupslave();
         bit = startbit;
         bitrd = startbitrd;
         if (master.read()) activatecs(true);
      }

      /********************** DELAY 0 *******************************************/
      /* We need to find out what bit we send. If we are in the pre-write phase, we
       * send zero. We need to keep track of where in the data to send we are.
       * We need to check that the pointer is inside the data to send. It could
       * pass in an odd case, if the customer gave unalined requests. We then give
       * him zeroes.
       */
      if (precycwr > 0 || bit > lastbit) bittosend = false;
      else bittosend = ((buffer[bit >> 5] & (1<<(bit & 0x1f)))>0);

      /* The clock could be wrong, so we go ahead and drive it. This should be
       * redundant, but it is nice to do.
       */
      if (master.read()) clk_o.write(actclk);

      /* We check the delaywrite mode to see if we send now or not. We do the same
       * for the sample. */
      if (dlywr == 0) d_o.write(bittosend);
      if (dlyrd == 0) bitreceived = d_i.read();

      /********************** DELAY 1 *******************************************/
      /* We either wait half time or  wait the high time and toggle the clock.
       * Note that we can also get a reset, in what case we apply it.
       */
      if (master.read() && sendok) {
         clk_o.write(!actclk);
         wait(hightime, master | reset_ev);
      }
      else wait(clk_i | master | reset_ev);
      if (master.event() || reset_ev.triggered()) {
         setupmasterslave();
         bit = -1;
         continue;
      }

      /* If there is a half-cycle delay on the data we change it now. */
      if (dlywr == 1) d_o.write(bittosend);
      if (dlyrd == 1) bitreceived = d_i.read();

      /********************** DELAY 2 *******************************************/
      /* We wait for the period to end. */
      if (master.read()) {
         clk_o.write(actclk);
         wait(lowtime, master | reset_ev);
      }
      else wait(clk_i | master | reset_ev);
      if (master.event() || reset_ev.triggered()) {
         setupmasterslave();
         bit = -1;
         continue;
      }

      /* And if the delay was a full-cycle, we send it now, before we
       * do the next clock edge. This is not a common setting, usually the delay
       * mode is 0 or 1.
       */
      if (dlywr == 2) d_o.write(bittosend);
      if (dlyrd == 2) bitreceived = d_i.read();

      /* We now store the data sampled, but only if we are not in the initial
       * delay area. We also need to make sure the bit position is inside the
       * valid range.
       */
      if (bitrd <= lastrdbit && precycrd == 0 && bitreceived == true) {
         buffer[bitrd >> 5] = buffer[bitrd >> 5] | (1<<(bitrd & 0x1f));
      }
      else if (bitrd <= lastrdbit && precycrd == 0 && bitreceived == false) {
         buffer[bitrd >> 5] = buffer[bitrd >> 5] & ~(1<<(bitrd & 0x1f));
      }

      /* Now we prepare for the next cycle. This depends on the settings. */
      /* The bit we set to -1 when it finished. If it has not finished, we check
       * the next bit. Note that it depends on the MSB vs LSB, word and byte order.
       */
      if (bit == lastbit) bit = -1;
      else bit = calcnextbit(bit, wrlittleendian, wrmsbfirst);
      if (rdbit == lastrdbit) rdbit = -1;
      else rdbit = calcnextbit(rdbit, rdlittleendian, rdmsbfirst);

      /* If both read and write went idle we need to lower the USR bit. If we do not
       * have the keepactive, we also deactivate the CS lines.
       */
      if (bit == -1 && rdbit == -1) {
         lowerusrbit_ev.notify();
         if (!RDFIELD(pin, SPI_CS_KEEP_ACTIVE_M, SPI_CS_KEEP_ACTIVE_S))
            deactivatecs(true);
      }

      /* We also need to decrement the hold counters. */
      if (precycwr == 0) precycwr = precycwr - 1;
      if (precycrd == 0) precycrd = precycrd - 1;
   }
}

/*****************
 * Method: configure_meth()
 * Inputs: none
 * Outputs: none
 * Return Value: none
 * Description:
 *    Loads the configuration registers and sets all that must take place imediately.
 *    The transfer task handles all configurations which can wait for the end of the
 *    command.
 */
void spimod::configure_meth() {
   /* If we get a reset, we reset the configuration part. */
   if (reset_ev.triggered()) {
      cs0_oen_o.write(false);
      cs1_oen_o.write(false);
      cs2_oen_o.write(false);
   }

   /* Master/slave configurations we take in only if the SPI is not running.
    * We then change the pin directions immediately.
    */
   if (master.read() && !RDFIELD(slave, SPI_SLAVE_MODE_M, SPI_SLAVE_MODE_S)) {
      master.write(false);
      d_oen_o.write(false);
      q_oen_o.write(true);
      wp_oen_o.write(false);
      hd_oen_o.write(false);
      cs0_oen_o.write(false);
      cs1_oen_o.write(false);
      cs2_oen_o.write(false);
   }
   else if (!master.read() && RDFIELD(slave,SPI_SLAVE_MODE_M,SPI_SLAVE_MODE_S)) {
      master.write(true);
      d_oen_o.write(true);
      q_oen_o.write(false);
      wp_oen_o.write(true);
      hd_oen_o.write(true);
      /* Perhaps this is not the right way, but we set the output enables to the
       * CS OEN bits all high. If one does not want them, he can disable them
       * in the matrix. Then the DIS bits in the pin register just specifies if
       * they will be high or low.
       */
      cs0_oen_o.write(true);
      cs1_oen_o.write(true);
      cs2_oen_o.write(true);
   }
}

void spimod::trace(sc_trace_file *tf) {
   sc_trace(tf, ctrl, ctrl.name());
   sc_trace(tf, ctrl2, ctrl2.name());
   sc_trace(tf, clock, clock.name());
   sc_trace(tf, pin, pin.name());
   sc_trace(tf, slave, slave.name());
   sc_trace(tf, slave1, slave1.name());
   sc_trace(tf, slave2, slave2.name());
   sc_trace(tf, slv_wr_status, slv_wr_status.name());
   sc_trace(tf, slv_wrbuf_dlen, slv_wrbuf_dlen.name());
   sc_trace(tf, slv_rdbuf_dlen, slv_rdbuf_dlen.name());
   sc_trace(tf, slv_rd_bit, slv_rd_bit.name());
   sc_trace(tf, cmd, cmd.name());
   sc_trace(tf, addr, addr.name());
   sc_trace(tf, user, user.name());
   sc_trace(tf, user1, user1.name());
   sc_trace(tf, user2, user2.name());
   sc_trace(tf, mosi_dlen, mosi_dlen.name());
   sc_trace(tf, miso_dlen, miso_dlen.name());
   sc_trace(tf, master, master.name());
}
