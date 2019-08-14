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
#include "pcntmod.h"
#include "setfield.h"
#include "soc/pcnt_reg.h"

void pcntmod::capture() {
   std::vector<bool> lastsig0(sig_ch0_un.size(), false);
   std::vector<bool> lastctrl0(ctrl_ch0_un.size(), false);
   std::vector<bool> lastsig1(sig_ch1_un.size(), false);
   std::vector<bool> lastctrl1(ctrl_ch1_un.size(), false);
   std::vector<bool> ctrl0_del(ctrl_ch0_un.size(), false);
   std::vector<bool> ctrl1_del(ctrl_ch1_un.size(), false);
   std::vector<int> fctrl0(ctrl_ch0_un.size(), 0);
   std::vector<int> fctrl1(ctrl_ch1_un.size(), 0);
   std::vector<int> fsig0(sig_ch0_un.size(), 0);
   std::vector<int> fsig1(sig_ch1_un.size(), 0);
   int un;

   while(true) {
      wait();

      /* First we calculate the control signal levels. */
      for(un = 0; un < ctrl_ch0_un.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            /* If the signal has changed, we store the value of edges we
             * want to see.
             */
            if (ctrl_ch0_un[un]->read() != lastctrl0[un])
               fctrl0[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            /* If it did not change, we check that the remaining edges. If it is
             * 0, no edge change has been seen. If it is 1, we are ready to
             * detect a signal change. If it is more than 1, then we cound down.
             */
            else if (fctrl0[un] > 2) fctrl0[un] = fctrl0[un] - 1;
            /* And we update the internal level. */
            else if (fctrl0[un] == 1) {
               ctrl0_del[un] = ctrl_ch0_un[un]->read();
               fctrl0[un] = 0;
            }
         }
         /* If filtering is disabled, we simply copy the value. */
         else ctrl0_del[un] = ctrl0_del[un];
         /* And we record the signal level so we can do edge detection. */
         lastctrl0[un] = ctrl_ch0_un[un];
      }
      /* And we redo it for the other channel. */
      for(un = 0; un < ctrl_ch1_un.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            if (ctrl_ch1_un[un]->read() != lastctrl1[un])
               fctrl1[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
               PCNT_FILTER_THRES_U0_S);
            else if (fctrl1[un] > 2) fctrl1[un] = fctrl1[un] - 1;
            else if (fctrl1[un] == 1) {
               ctrl1_del[un] = ctrl_ch1_un[un];
               fctrl1[un] = 0;
            }
         }
         /* If filtering is disabled, we simply copy the value. */
         else ctrl1_del[un] = ctrl_ch1_un[un];
         /* And we record the signal level so we can do edge detection. */
         lastctrl1[un] = ctrl_ch1_un[un];
      }

      /* Now we can do the signals. These are edge detected, so when we pass
       * the filtering, we can already handle them. Besides that, it is the
       * same.
       */
      for(un = 0; un < sig_ch0_un.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            /* If the signal has changed, we store the value of edges we
             * want to see.
             */
            if (sig_ch0_un[un]->read() != lastsig0[un])
               fsig0[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            /* If it did not change, we check that the remaining edges. If it is
             * 0, no edge change has been seen. If it is 1, we are ready to
             * detect a signal change. If it is more than 1, then we cound down.
             */
            else if (fsig0[un] > 2) fsig0[un] = fsig0[un] - 1;
            /* And we update the internal level. */
            else if (fsig0[un] == 1) docnt(un, ctrl0_del[un], 0);
         }
         /* If filtering is disabled, we simply copy the value. */
         else docnt(un, ctrl0_del[un], 0);
         /* And we record the signal level so we can do edge detection. */
         lastsig0[un] = sig_ch0_un[un];
      }
      for(un = 0; un < sig_ch1_un.size(); un = un + 1) {
         if (RDFIELD(conf0[un], PCNT_FILTER_EN_U0_M, PCNT_FILTER_EN_U0_S)>0
               && RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S)>0) {
            /* Filtering is enabled. */
            if (sig_ch1_un[un]->read() != lastsig1[un])
               fsig1[un] = RDFIELD(conf0[un], PCNT_FILTER_THRES_U0_M,
                  PCNT_FILTER_THRES_U0_S);
            else if (fsig1[un] > 2) fsig1[un] = fsig1[un] - 1;
            /* And we update the internal level. */
            else if (fsig1[un] == 1) docnt(un, ctrl1_del[un], 0);
         }
         /* If filtering is disabled, we simply copy the value. */
         else docnt(un, ctrl1_del[un], 1);
         /* And we record the signal level so we can do edge detection. */
         lastsig1[un] = sig_ch1_un[un];
      }
   }
}

void pcntmod::docnt(int un, bool ctrllvl, int ch) {
   int mode, lctrl, hctrl;
   bool siglvl;
   int16_t nc;
   if (ch == 0) {
      siglvl = sig_ch0_un[un]->read();
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
      siglvl = sig_ch1_un[un]->read();
      if (siglvl) mode =
         RDFIELD(conf0[un], PCNT_CH0_POS_MODE_U0_M, PCNT_CH0_POS_MODE_U0_S);
      else mode =
         RDFIELD(conf0[un], PCNT_CH0_NEG_MODE_U0_M, PCNT_CH0_NEG_MODE_U0_S);
      lctrl =
         RDFIELD(conf0[un], PCNT_CH0_LCTRL_MODE_U0_M, PCNT_CH0_LCTRL_MODE_U0_S);
      hctrl =
         RDFIELD(conf0[un], PCNT_CH0_HCTRL_MODE_U0_M, PCNT_CH0_HCTRL_MODE_U0_S);
   }

   if (ctrllvl == false) {
      if (hctrl == 0 && mode == 1) nc = cnt_unit[un].read() + 1;
      else if (hctrl == 1 && mode == 2) nc = cnt_unit[un].read() + 1;
      else if (hctrl == 0 && mode == 2) nc = cnt_unit[un].read() - 1;
      else if (hctrl == 1 && mode == 1) nc = cnt_unit[un].read() - 1;
      /* disable and mode 0 we ignore */
   }
   else {
      if (lctrl == 0 && mode == 1) nc = cnt_unit[un].read() + 1;
      else if (lctrl == 1 && mode == 2) nc = cnt_unit[un].read() + 1;
      else if (lctrl == 0 && mode == 2) nc = cnt_unit[un].read() - 1;
      else if (lctrl == 1 && mode == 1) nc = cnt_unit[un].read() - 1;
      /* disable and mode 0 we ignore */
   }

   /* We now check the thresholds, limits and zero comparator. */
   /* First we look at the limit comparators. */
   if (RDFIELD(conf0[un], PCNT_THR_L_LIM_EN_U0_M, PCNT_THR_L_LIM_EN_U0_S) &&
         nc <= (int16_t)RDFIELD(conf2[un], PCNT_CNT_L_LIM_U0_M,
            PCNT_CNT_L_LIM_U0_S)
      || RDFIELD(conf0[un], PCNT_THR_H_LIM_EN_U0_M, PCNT_THR_L_LIM_EN_U0_S) &&
         nc >= (int16_t)RDFIELD(conf2[un], PCNT_CNT_L_LIM_U0_M,
            PCNT_CNT_L_LIM_U0_S)) {
      nc = 0;
      int_raw.write(int_raw.read() | (1 << un));
   }
   /* Now the thresholds. */
   if (RDFIELD(conf0[un], PCNT_THR_THRES0_EN_U0_M, PCNT_THR_THRES0_EN_U1_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES0_U0_M,
            PCNT_CNT_THRES0_U1_S)) {
      int_raw.write(int_raw.read() | (1 << un));
   }
   if (RDFIELD(conf0[un], PCNT_THR_THRES1_EN_U0_M, PCNT_THR_THRES1_EN_U1_S) &&
         nc == (int16_t)RDFIELD(conf1[un], PCNT_CNT_THRES1_U0_M,
            PCNT_CNT_THRES1_U1_S)) {
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
   std::string sign = sigb + "conf0[0]";
   int digit;

   digit = sign.length() - 2;
   for(un = 0; un < 0; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf0[un], sign.c_str());
   }
   sign[digit-2] = '1';
   for(un = 0; un < 0; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf1[un], sign.c_str());
   }
   sign[digit-2] = '2';
   for(un = 0; un < 0; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf2[un], sign.c_str());
   }

   sign = sigb + "cntun[0]";
   digit = sign.length() - 2;
   for(un = 0; un < 0; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, cnt_unit[un], sign.c_str());
   }

   sign = sigb + "status[0]";
   digit = sign.length() - 2;
   for(un = 0; un < 0; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, status_unit[un], sign.c_str());
   }
   sc_trace(tf, int_raw, int_raw.name());
   sc_trace(tf, ctrl, ctrl.name());
}
