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
         int_raw.value_changed_event());

      PCNT.int_raw.val = int_raw.read();
      PCNT.int_st.val = int_raw.read() & int_ena.read();
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
   wait(apb_clk.posedge_event());
}

void pcntmod::initstruct() {
   memset(&PCNT, 0, sizeof(pcnt_dev_t));
}

void pcntmod::capture() {
   int un;
   bool ctrl0_del[8];
   bool ctrl1_del[8];
   int fctrl0[8];
   int fctrl1[8];
   int fsig0[8];
   int fsig1[8];
   pcntbus_t lvl[8];
   pcntbus_t lastlvl[8];

   for(un = 0; un < pcntbus_i.size(); un = un + 1) {
      lastlvl[un].sig_ch0 = false;
      lastlvl[un].sig_ch1 = false;
      lastlvl[un].ctrl_ch0 = false;
      lastlvl[un].ctrl_ch1 = false;
   }

   while(true) {
      wait();

      /* First we need to get the new inputs. */
      for(un = 0; un < pcntbus_i.size(); un = un + 1)
         lvl[un] = pcntbus_i[un]->read();

      for(un = 0; un < pcntbus_i.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            /* If the signal has changed, we store the value of edges we
             * want to see.
             */
            if (lvl[un].ctrl_ch0 != lastlvl[un].ctrl_ch0)
               fctrl0[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            /* If it did not change, we check that the remaining edges. If it is
             * 0, no edge change has been seen. If it is 1, we are ready to
             * detect a signal change. If it is more than 1, then we cound down.
             */
            else if (fctrl0[un] > 2) fctrl0[un] = fctrl0[un] - 1;
            /* And we update the internal level. */
            else if (fctrl0[un] == 1) {
               ctrl0_del[un] = lvl[un].ctrl_ch0;
               fctrl0[un] = 0;
            }
         }
         /* If filtering is disabled, we simply copy the value. */
         else ctrl0_del[un] = lvl[un].ctrl_ch0;
         /* And we record the signal level so we can do edge detection. */
         lastlvl[un].ctrl_ch0 = lvl[un].ctrl_ch0;
      }
      /* And we redo it for the other channel. */
      for(un = 0; un < pcntbus_i.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            if (lvl[un].ctrl_ch1 != lastlvl[un].ctrl_ch1)
               fctrl1[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
               PCNT_FILTER_THRES_U0_S);
            else if (fctrl1[un] > 2) fctrl1[un] = fctrl1[un] - 1;
            else if (fctrl1[un] == 1) {
               ctrl1_del[un] = lvl[un].ctrl_ch1;
               fctrl1[un] = 0;
            }
         }
         /* If filtering is disabled, we simply copy the value. */
         else ctrl1_del[un] = lvl[un].ctrl_ch1;
         /* And we record the signal level so we can do edge detection. */
         lastlvl[un].ctrl_ch1 = lvl[un].ctrl_ch1;
      }

      /* Now we can do the signals. These are edge detected, so when we pass
       * the filtering, we can already handle them. Besides that, it is the
       * same.
       */
      for(un = 0; un < pcntbus_i.size(); un = un + 1) {
         /* We first check to see if the channel is in reset or pause. If it
          * is in reset, we clear it. If it is in pause we do nothing.
          */
         if ((ctrl.read() & (PCNT_PLUS_CNT_RST_U0_M<<un*2))>0) {
            cnt_unit[un].write(0);
         }
         /* If it is ok, we can check the filtering. */
         else if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            /* If the signal has changed, we store the value of edges we
             * want to see.
             */
            if (lvl[un].sig_ch0 != lastlvl[un].sig_ch0)
               fsig0[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            /* If it did not change, we check that the remaining edges. If it is
             * 0, no edge change has been seen. If it is 1, we are ready to
             * detect a signal change. If it is more than 1, then we cound down.
             */
            else if (fsig0[un] > 2) fsig0[un] = fsig0[un] - 1;
            /* And we update the internal level. */
            else if (fsig0[un] == 1)
               docnt(un, lvl[un].sig_ch0, ctrl0_del[un], 0);
         }
         /* If filtering is disabled, we simply copy the value. */
         else if (lastlvl[un].sig_ch0 != lvl[un].sig_ch0)
            docnt(un, lvl[un].sig_ch0, ctrl0_del[un], 0);
         /* And we record the signal level so we can do edge detection. */
         lastlvl[un].sig_ch0 = lvl[un].sig_ch0;
      }
      for(un = 0; un < pcntbus_i.size(); un = un + 1) {
         /* Reset is handled above. This one just follows, so we do nothing. */
         if ((ctrl.read() & (PCNT_PLUS_CNT_RST_U0_M<<un*2))>0) {
         }
         else if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            if (lvl[un].sig_ch1 != lastlvl[un].sig_ch1)
               fsig1[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            else if (fsig1[un] > 2) fsig1[un] = fsig1[un] - 1;
            /* And we update the internal level. */
            else if (fsig1[un] == 1)
               docnt(un, lvl[un].sig_ch1, ctrl1_del[un], 0);
         }
         /* If filtering is disabled, we simply copy the value. */
         else if (lastlvl[un].sig_ch1 != lvl[un].sig_ch1)
            docnt(un, lvl[un].sig_ch1, ctrl1_del[un], 1);
         /* And we record the signal level so we can do edge detection. */
         lastlvl[un].sig_ch1 = lvl[un].sig_ch1;
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
         RDFIELD(conf0[un], PCNT_CH0_POS_MODE_U0_M, PCNT_CH0_POS_MODE_U0_S);
      else mode =
         RDFIELD(conf0[un], PCNT_CH0_NEG_MODE_U0_M, PCNT_CH0_NEG_MODE_U0_S);
      lctrl =
         RDFIELD(conf0[un], PCNT_CH0_LCTRL_MODE_U0_M, PCNT_CH0_LCTRL_MODE_U0_S);
      hctrl =
         RDFIELD(conf0[un], PCNT_CH0_HCTRL_MODE_U0_M, PCNT_CH0_HCTRL_MODE_U0_S);
   }

   /* If not we keep going. */
   if (ctrllvl == false) {
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
      int_raw.write(int_raw.read() | (1 << un));
   }
   /* Now the thresholds. */
   if (RDFIELD(conf0[un], PCNT_THR_THRES0_EN_U0_M, PCNT_THR_THRES0_EN_U0_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES0_U0_M,
            PCNT_CNT_THRES0_U0_S)) {
      int_raw.write(int_raw.read() | (1 << un));
   }
   if (RDFIELD(conf0[un], PCNT_THR_THRES1_EN_U0_M, PCNT_THR_THRES1_EN_U0_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES1_U0_M,
            PCNT_CNT_THRES1_U0_S)) {
      int_raw.write(int_raw.read() | (1 << un));
   }
   /* And finally the zero. */
   if (nc == 0 && RDFIELD(conf0[un], PCNT_THR_ZERO_EN_U0_M,
         PCNT_THR_ZERO_EN_U0_S)) {
      int_raw.write(int_raw.read() | (1 << un));
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
   sc_trace(tf, int_raw, int_raw.name());
   sc_trace(tf, ctrl, ctrl.name());
}
