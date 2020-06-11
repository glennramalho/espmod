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

void spimod::updateth() {
   int ch;
   int tim;
   sc_time base_period;

   while(true) {
      wait();
      cmd.write(*spistruct.cmd.val);
      addr.write(*spistruct.addr.val);
      ctrl.write(*spistruct.ctrl.val);
      ctrl2.write(*spistruct.ctrl2.val);
      clock.write(*spistruct.clock.val);
      user.write(*spistruct.user.val);
      user1.write(*spistruct.user1.val);
      user2.write(*spistruct.user2.val);
      mosi_dlen.write(*spistruct.mosi_dlen.val);
      pin.write(*spistruct.pin.val);
      slave.write(*spistruct.slave.val);
      slave1.write(*spistruct.slave1.val);
      slave2.write(*spistruct.slave2.val);
      slv_wrbuf_dlen.write(*spistruct.slv_wrbuf_dlen.val);
      slv_rdbuf_dlen.write(*spistruct.slv_rdbuf_dlen.val);
      slv_rd_bit.write(*spistruct.slv_rd_bit.val);
   }
}

void spimod::returnth() {
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

int spimod::calclastbit(pos, littleendian, msbfirst) {

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

void spimod::transfer() {
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
         bit = -1;
         continue;
      }

      /* If the bit was -1, we need to set it to the correct starting point. */
      if (bit == -1) bit = startbit;

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

      /* We also need to decrement the hold counters. */
      if (precycwr == 0) precycwr = precycwr - 1;
      if (precycrd == 0) precycrd = precycrd - 1;
   }
}

void spimod::configure() {
   while(1) {
      wait();

      /* If we get a reset, we stop the clock, CS and data lines. */
      if (sync.event() && RDFIELD(slave, SPI_SYNC_RESET_M, SPI_SYNC_RESET_S)) {
         /* We take the lines low, but we do not change the master/slave mode or
          * clock polarity.
          */
         d_o.write(false);
         q_o.write(false);
         wp_o.write(false);
         hd_o.write(false);
         /* We do not check the clock polarity as the clock is only used if
          * configured in the io matrix.
          */
         if (RDFIELD(pin, SPI_CK_IDLE_EDGE_M, SPI_CK_IDLE_EDGE_S)) clk_o.write(true);
         else clk_o.write(false);
         cs0_oen_o.write(false);
         cs1_oen_o.write(false);
         cs2_oen_o.write(false);
         in_data = 0;
         out_data = 0;
         /* We lower the USR bit. */
         lowerusrbit_ev.notify();
         /* We also need to notify the task. */
         reset_ev.notify();
      }

      /* We use the slave enable bit to decide if we can or cannot have a clock.
       * We also go through this when there was a change in the clock definition.
       */
      if (slave.event() || clock.event()) {
         if (!RDFIELD(slave, SPI_SLAVE_MODE, SPI_CLK_EQU_SUSCLK_S)) {
            /* Master Mode. */
            master.write(true);
            d_o.write(false); d_oen_o.write(true);
            q_o.write(false); q_oen_o.write(false);
            wp_o.write(false); wp_oen_o.write(true);
            hd_o.write(false); hd_oen_o.write(true);

            /* If we are in master mode, we need to define the clock. */
            if (RDFIELD(slave, SPI_CLK_EQU_SYSCLK_M, SPI_CLK_EQU_SUSCLK_S)) {
               period = clockpacer.get_apb_period();
               halftime = sc_time(clockpacer.get_apb_period()/2);
               lowtime = period - halftime;
            }
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

            /* The master clock can be active low or high. */
            clk_oen_o.write(true);
            if (RDFIELD(pin, SPI_CK_IDLE_EDGE_M, SPI_CK_IDLE_EDGE_S))
               clk_o.write(true);
            else clk_o.write(false);
         }
         else {
            /* Slave Mode */
            master.write(false);
            d_o.write(false); d_oen_o.write(false);
            q_o.write(false); q_oen_o.write(true);
            wp_o.write(false); wp_oen_o.write(false);
            hd_o.write(false); hd_oen_o.write(false);

            /* The master clock we deactivate. */
            clk_oen_o.write(false);
            if (RDFIELD(pin, SPI_CK_IDLE_EDGE_M, SPI_CK_IDLE_EDGE_S))
               clk_o.write(true);
            else clk_o.write(false);
         }
      }

      /* The select pins we also check when we change them or change the mode. */
      if ((slave.event() || pin.event()) {
         if (!RDFIELD(slave, SPI_SLAVE_MODE, SPI_CLK_EQU_SUSCLK_S)) {

            /* If we are in the keep active mode, we need to switch these guys now.
             * We activate the CS pins.
             */
            cs0_oen_o.write(true);
            cs1_oen_o.write(true);
            cs2_oen_o.write(true);
            if (RDFIELD(pin, SPI_CS_KEEP_ACTIVE_M,
                  SPI_CS_KEEP_ACTIVE_S)) {
               if (!RDFIELD(pin, SPI_CS2_DIS_M, SPI_CS2_DIS_S)) cs2_o.write(false);
               else cs0_o.write(true);
               if (!RDFIELD(pin, SPI_CS1_DIS_M, SPI_CS1_DIS_S)) cs1_o.write(false);
               else cs1_o.write(true);
               if (!RDFIELD(pin, SPI_CS0_DIS_M, SPI_CS0_DIS_S)) cs0_o.write(false);
               else cs2_o.write(true);
            }
            else {
               cs0_o.write(true);
               cs1_o.write(true);
               cs2_o.write(true);
            }
         }
         else {
            /* If we are switching to slave mode or got a write to the pin register
             * but did not change anything, we lower the activity signals just in
             * case.
             */
            cs0_oen_o.write(false);
            cs1_oen_o.write(false);
            cs2_oen_o.write(false);
            cs0_o.write(true);
            cs1_o.write(true);
            cs2_o.write(true);
         }
      }

      /* When we see the start bit and we are in master mode we begin. */
      if (master.read() && usr.event() && usr.read() != 0) {
         clockpacer.wait_next_apb_clk();

         /* We activate the CS pins. */
         if (!RDFIELD(pin, SPI_CS2_DIS_M, SPI_CS2_DIS_S)) cs2_o.write(false);
         if (!RDFIELD(pin, SPI_CS1_DIS_M, SPI_CS1_DIS_S)) cs1_o.write(false);
         if (!RDFIELD(pin, SPI_CS0_DIS_M, SPI_CS0_DIS_S)) cs0_o.write(false);

         /* We wait the setup time. */
         if (RDFIELD(user, SPI_CS_SETUP_M, SPI_CS_SETUP_S)) {
            wait(period * RDFIELD(ctrl2, SPI_SETUP_M, SPI_SETUP_S));
         }

         /* We need to determine where the data when data will change relative
          * to the clock. The MOSI and MISO have slightly different rules.
          */
         dlywr = getdly(RDFIELD(ctrl2, SPI_MOSI_DELAY_M, SPI_MOSI_DELAY_MODE_S));
         dlyrd = getdly(RDFIELD(ctrl2, SPI_MISO_DELAY_M, SPI_MISO_DELAY_MODE_S));

         /* If the field is enabled, we can transmit it. */
         if (RDFIELD(user, SPI_MOSI_M, SPI_MOSI_S) == 0) {
            limitbit = -1;
            startbit = -1;
         }
         else {
            wrlittleendian = RDFIELD(user,
               SPI_WR_BYTE_ORDER_M, SPI_WR_BYTE_ORDER_S) == 0;
            wrmsbfirst = RDFIELD(user,
               SPI_WR_BIT_ORDER_M, SPI_WR_BIT_ORDER_S) == 0;
            startbit = 32 * ((RDFIELD(user,
               SPI_USR_MOSI_HIGHPART_M, SPI_USR_MOSI_HIGHPART_S) == 1)?8:0);
            lastbit = calclastbit(startbit + mosi_dlen.read());
            if (wrlittleendian && wrmsbfirst) startbit = startbit + 31;
            else if (!wrlittleendian && wrmsbfirst) startbit = startbit + 7;
            else if (wrlittleendian && !wrmsbfirst) startbit = startbit + 24;

            /* And we can start. */
            start_ev.notify();
         }

         /* We do the same for reading. */
         if (RDFIELD(user, SPI_MISO_M, SPI_MISO_S) == 0) {
            limitbitrd = -1;
            startbitrd = -1;
         }
         else {
            rdlittleendian = RDFIELD(user,
               SPI_RD_BYTE_ORDER_M, SPI_RD_BYTE_ORDER_S) == 0;
            rdmsbfirst = RDFIELD(user,
               SPI_RD_BIT_ORDER_M, SPI_RD_BIT_ORDER_S) == 0;
            startbitrd = 32 * ((RDFIELD(user,
               SPI_USR_MOSI_HIGHPART_M, SPI_USR_MOSI_HIGHPART_S) == 1)?8:0);
            lastbitrd = calclastbit(startbit + miso_dlen.read());
            if (rdlittleendian && wrmsbfirst) startbit = startbit + 31;
            else if (!rdlittleendian && rdmsbfirst) startbit = startbit + 7;
            else if (rdlittleendian && !rdmsbfirst) startbit = startbit + 24;

            /* And we can start. */
            start_ev.notify();
         }
      }

         /* If we do not have the keep active, we deactivate the CS. */
      if (donewithwrite_ev.triggered()) {
         if (RDFIELD(user, SPI_CS_SETUP_M, SPI_CS_SETUP_S)) {
            wait(period * RDFIELD(ctrl2, SPI_SETUP_M, SPI_SETUP_S));
         }
         if (RDFIELD(pin, SPI_CS_KEEP_ACTIVE_M, SPI_CS_KEEP_ACTIVE_S)) {
            cs2_o.write(true);
            cs1_o.write(true);
            cs0_o.write(true);
         }
         lowerusrbit_ev.notify();
      }
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
