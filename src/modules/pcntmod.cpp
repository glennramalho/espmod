/*******************************************************************************
 * pcntmod.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 Pulse Counter
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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc.h>
#include <string.h>
#include "pcntmod.h"
#include "setfield.h"
#include "soc/pcnt_struct.h"
#include "soc/pcnt_reg.h"
#include "Arduino.h"

void pcntmod::updateth() {
   int un;
   while(true) {
      wait();
      for(un = 0; un < 8; un = un + 1) {
         conf0[un].write(PCNT.conf_unit[un].conf0.val);
         conf1[un].write(PCNT.conf_unit[un].conf1.val);
         conf2[un].write(PCNT.conf_unit[un].conf2.val);
      }
      int_ena.write(PCNT.int_ena.val);
      ctrl.write(PCNT.ctrl.val);
      /* We need to apply any reset bits. We can't set the values directly
       * as they are signals and they are driven by another thread, so we
       * just raise the notices.
       */
      if ((PCNT_PLUS_CNT_RST_U0_M & PCNT.ctrl.val)>0) reset_un[0].notify();
      if ((PCNT_PLUS_CNT_RST_U1_M & PCNT.ctrl.val)>0) reset_un[1].notify();
      if ((PCNT_PLUS_CNT_RST_U2_M & PCNT.ctrl.val)>0) reset_un[2].notify();
      if ((PCNT_PLUS_CNT_RST_U3_M & PCNT.ctrl.val)>0) reset_un[3].notify();
      if ((PCNT_PLUS_CNT_RST_U4_M & PCNT.ctrl.val)>0) reset_un[4].notify();
      if ((PCNT_PLUS_CNT_RST_U5_M & PCNT.ctrl.val)>0) reset_un[5].notify();
      if ((PCNT_PLUS_CNT_RST_U6_M & PCNT.ctrl.val)>0) reset_un[6].notify();
      if ((PCNT_PLUS_CNT_RST_U7_M & PCNT.ctrl.val)>0) reset_un[7].notify();
   }
}

void pcntmod::returnth() {
   int un;
   while(true) {
      /* We wait for one of the counters to change. When that happens we copy
       * the values to the correct fields in the PCNT struct. This is not the
       * ideal way to do this, but it should work for now.
       */
      wait(
         cnt_unit[0].value_changed_event() | cnt_unit[1].value_changed_event() |
         cnt_unit[2].value_changed_event() | cnt_unit[3].value_changed_event() |
         cnt_unit[4].value_changed_event() | cnt_unit[5].value_changed_event() |
         cnt_unit[6].value_changed_event() | cnt_unit[7].value_changed_event() |
         int_raw[0].value_changed_event() | int_raw[1].value_changed_event() |
         int_raw[2].value_changed_event() | int_raw[3].value_changed_event() |
         int_raw[4].value_changed_event() | int_raw[5].value_changed_event() |
         int_raw[6].value_changed_event() | int_raw[7].value_changed_event());

      PCNT.int_raw.val =
         ((int_raw[7].read())?0x80:0x0) |
         ((int_raw[6].read())?0x40:0x0) |
         ((int_raw[5].read())?0x20:0x0) |
         ((int_raw[4].read())?0x10:0x0) |
         ((int_raw[3].read())?0x08:0x0) |
         ((int_raw[2].read())?0x04:0x0) |
         ((int_raw[1].read())?0x02:0x0) |
         ((int_raw[0].read())?0x01:0x0);
      PCNT.int_st.val = PCNT.int_raw.val & int_ena.read();
      for(un = 0; un < 8; un = un + 1) {
         PCNT.cnt_unit[un].val = cnt_unit[un].read();
         PCNT.status_unit[un].thres0_lat =
            PCNT.cnt_unit[un].val ==
            RDFIELD(conf1[un], PCNT_CNT_THRES0_U0_M,
            PCNT_CNT_THRES0_U0_S);
         PCNT.status_unit[un].thres1_lat =
            PCNT.cnt_unit[un].val ==
            RDFIELD(conf1[un], PCNT_CNT_THRES1_U0_M,
            PCNT_CNT_THRES1_U0_S);
         PCNT.status_unit[un].l_lim_lat =
            PCNT.cnt_unit[un].val <=
            RDFIELD(conf2[un], PCNT_CNT_L_LIM_U0_M,
            PCNT_CNT_L_LIM_U0_S);
         PCNT.status_unit[un].h_lim_lat =
            PCNT.cnt_unit[un].val >=
            RDFIELD(conf2[un], PCNT_CNT_H_LIM_U0_M,
            PCNT_CNT_H_LIM_U0_S);
         PCNT.status_unit[un].zero_lat = PCNT.cnt_unit[un].val == 0;
      }
   }
}

void pcntmod::update() {
   update_ev.notify();
   wait_next_apb_clock();
}

void pcntmod::initstruct() {
   memset(&PCNT, 0, sizeof(pcnt_dev_t));
}

void pcntmod::start_of_simulation() {
   /* We spawn a thread for each channel. */
   int un;
   for(un = 0; un < pcntbus_i.size(); un = un + 1) {
      sc_spawn(sc_bind(&pcntmod::capture, this, un));
      sc_spawn(sc_bind(&pcntmod::count, this, un));
   }
}

void pcntmod::capture(int un) {
   pcntbus_t lvl;
   pcntbus_t lastlvl;
   unsigned int filter_thres;

   lastlvl.sig_ch0 = false;
   lastlvl.sig_ch1 = false;
   lastlvl.ctrl_ch0 = false;
   lastlvl.ctrl_ch1 = false;
   fctrl0[un] = sc_time(0, SC_NS);
   fctrl1[un] = sc_time(0, SC_NS);

   while(true) {
      /* We wait until an input changed. */
      wait(pcntbus_i[un]->default_event());

      /* We wait until the next clock edge. */
      wait_next_apb_clock();

      /* Now we sample the inputs. Note that we might have lost some thing
       * but that is expected as this is a sampled protocol.
       */
      lvl = pcntbus_i[un]->read();

      /* We get the filter enable and value. */
      if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0)
         filter_thres = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
             PCNT_FILTER_THRES_U0_S);
      else filter_thres = 0;

      /* Now we need to look at each signal and find out if it should go high
       * now or if it should go high only when filtered.
       */
      if (lvl.ctrl_ch0 != lastlvl.ctrl_ch0) {
         /* If the next transition is in the future, all we do is cancel the
          * transition. It has been filtered out.
          */
         if (fctrl0[un] >= sc_time_stamp()) fctrl0[un] = sc_time(0, SC_NS);
         /* If it is in the past, then we record it. We then check the
          * filtering. If it is positive, we set it X clock cycles later.
          */
         else if (filter_thres > 0) {
            fctrl0[un] = sc_time_stamp() +
               sc_time(filter_thres * APB_CLOCK_PERIOD, SC_NS);
         }
         /* If no filtering was specified, we toggle the signal now. */
         else fctrl0[un] = sc_time_stamp();
      }
      /* And we redo it for the other channel. */
      if (lvl.ctrl_ch1 != lastlvl.ctrl_ch1) {
         if (fctrl0[un] >= sc_time_stamp()) fctrl0[un] = sc_time(0, SC_NS);
         else if (filter_thres > 0) {
            fctrl1[un] = sc_time_stamp() +
               sc_time(filter_thres * APB_CLOCK_PERIOD, SC_NS);
         }
         else fctrl1[un] = sc_time_stamp();
      }

      /* Now we can do the signals. We also look at the signal to see when it
       * should be triggered. For this one, we raise a notification when the
       * signal should go high or low.
       */
      /* First we check, if we are in reset, we do nothing. */
      if (!((ctrl.read() & (PCNT_PLUS_CNT_RST_U0_M<<un*2))>0)) {
         /* If it is ok, we can check the filtering. */
         if (lvl.sig_ch0 != lastlvl.sig_ch0) {
            /* If filtering is enabled so we notify the doit function when
             * the signal should change value. If filterig is off, we notify
             * it immediatedly.
             */
            if (filter_thres > 0)
               filtered_sig0[un].notify(APB_CLOCK_PERIOD * filter_thres, SC_NS);
            else filtered_sig0[un].notify();
         }

         /* And we repeat for the other signal. */
         if (lvl.sig_ch1 != lastlvl.sig_ch1) {
            if (filter_thres > 0)
               filtered_sig1[un].notify(APB_CLOCK_PERIOD * filter_thres, SC_NS);
            else filtered_sig1[un].notify();
         }
      }

      /* Once we are done, we need to copy the current lvl into the lastlvl
       * so that we can do edge detections.
       */
      lastlvl = lvl;
   }
}

void pcntmod::count(int un) {
   pcntbus_t p;
   while(true) {
      wait(filtered_sig0[un] | filtered_sig1[un] | reset_un[un]);
      p = pcntbus_i[un]->read();
      /* If there was a reset notice, we reset the block and do nothing else.*/
      if (reset_un[un].triggered()) { cnt_unit[un].write(0); }
      /* Assuming it was not in reset, we can look at the other triggers. */
      else {
         if (filtered_sig0[un].triggered()) {
            if (sc_time_stamp() < fctrl0[un]) 
               docnt(un, p.sig_ch0, !p.ctrl_ch0, 0);
            else docnt(un, p.sig_ch0, p.ctrl_ch0, 0);
         }
         else {
            if (sc_time_stamp() < fctrl1[un]) 
               docnt(un, p.sig_ch1, !p.ctrl_ch1, 1);
            else docnt(un, p.sig_ch1, p.ctrl_ch1, 1);
         }
      }
   }
}

void pcntmod::docnt(int un, bool siglvl, bool ctrllvl, int ch) {
   int mode, lctrl, hctrl;
   int16_t nc;

   /* If it is paused or in reset, we do nothing. */
   if ((ctrl.read() & ((PCNT_PLUS_CNT_RST_U0_M|PCNT_CNT_PAUSE_U0_M)<<un*2))!=0)
      return;

   if (ch == 0) {
      if (siglvl) mode =
         RDFIELD(conf0[un], PCNT_CH0_POS_MODE_U0_M, PCNT_CH0_POS_MODE_U0_S);
      else mode =
         RDFIELD(conf0[un], PCNT_CH0_NEG_MODE_U0_M, PCNT_CH0_NEG_MODE_U0_S);
      lctrl =
         RDFIELD(conf0[un], PCNT_CH0_LCTRL_MODE_U0_M, PCNT_CH0_LCTRL_MODE_U0_S);
      hctrl =
         RDFIELD(conf0[un], PCNT_CH0_HCTRL_MODE_U0_M, PCNT_CH0_HCTRL_MODE_U0_S);
   }
   else {
      if (siglvl) mode =
         RDFIELD(conf0[un], PCNT_CH1_POS_MODE_U0_M, PCNT_CH1_POS_MODE_U0_S);
      else mode =
         RDFIELD(conf0[un], PCNT_CH1_NEG_MODE_U0_M, PCNT_CH1_NEG_MODE_U0_S);
      lctrl =
         RDFIELD(conf0[un], PCNT_CH1_LCTRL_MODE_U0_M, PCNT_CH1_LCTRL_MODE_U0_S);
      hctrl =
         RDFIELD(conf0[un], PCNT_CH1_HCTRL_MODE_U0_M, PCNT_CH1_HCTRL_MODE_U0_S);
   }

   /* If not we keep going. */
   if (ctrllvl == true) {
      if (hctrl == 0 && mode == 1) nc = cnt_unit[un].read() + 1;
      else if (hctrl == 1 && mode == 2) nc = cnt_unit[un].read() + 1;
      else if (hctrl == 0 && mode == 2) nc = cnt_unit[un].read() - 1;
      else if (hctrl == 1 && mode == 1) nc = cnt_unit[un].read() - 1;
      /* disable and mode 0 we ignore */
      else nc = cnt_unit[un].read();
   }
   else {
      if (lctrl == 0 && mode == 1) nc = cnt_unit[un].read() + 1;
      else if (lctrl == 1 && mode == 2) nc = cnt_unit[un].read() + 1;
      else if (lctrl == 0 && mode == 2) nc = cnt_unit[un].read() - 1;
      else if (lctrl == 1 && mode == 1) nc = cnt_unit[un].read() - 1;
      /* disable and mode 0 we ignore */
      else nc = cnt_unit[un].read();
   }

   /* We now check the thresholds, limits and zero comparator. */
   /* First we look at the limit comparators. */
   if (RDFIELD(conf0[un], PCNT_THR_L_LIM_EN_U0_M, PCNT_THR_L_LIM_EN_U0_S) &&
         nc <= (int16_t)RDFIELD(conf2[un], PCNT_CNT_L_LIM_U0_M,
            PCNT_CNT_L_LIM_U0_S)
      || RDFIELD(conf0[un], PCNT_THR_H_LIM_EN_U0_M, PCNT_THR_H_LIM_EN_U0_S) &&
         nc >= (int16_t)RDFIELD(conf2[un], PCNT_CNT_H_LIM_U0_M,
            PCNT_CNT_H_LIM_U0_S)) {
      nc = 0;
      int_raw[un].write(true);
   }
   /* Now the thresholds. */
   if (RDFIELD(conf0[un], PCNT_THR_THRES0_EN_U0_M, PCNT_THR_THRES0_EN_U0_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES0_U0_M,
            PCNT_CNT_THRES0_U0_S)) {
      int_raw[un].write(true);
   }
   if (RDFIELD(conf0[un], PCNT_THR_THRES1_EN_U0_M, PCNT_THR_THRES1_EN_U0_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES1_U0_M,
            PCNT_CNT_THRES1_U0_S)) {
      int_raw[un].write(true);
   }
   /* And finally the zero. */
   if (nc == 0 && RDFIELD(conf0[un], PCNT_THR_ZERO_EN_U0_M,
         PCNT_THR_ZERO_EN_U0_S)) {
      int_raw[un].write(true);
   }

   /* And we commit the new value. */
   cnt_unit[un].write(nc);
}

void pcntmod::trace(sc_trace_file *tf) {
   int un;
   std::string sigb = name();
   std::string sign = sigb + ".conf0_0";
   int digit;

   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf0[un], sign.c_str());
   }
   sign[digit-2] = '1';
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf1[un], sign.c_str());
   }
   sign[digit-2] = '2';
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf2[un], sign.c_str());
   }

   sign = sigb + ".cntun_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, cnt_unit[un], sign.c_str());
   }

   sign = sigb + ".status_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, status_unit[un], sign.c_str());
   }

   sign = sigb + ".int_raw_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, int_raw[un], sign.c_str());
   }
   sc_trace(tf, ctrl, ctrl.name());
}
