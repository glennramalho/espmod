/*******************************************************************************
 * ledcmod.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 PWM LEDC module.
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
#include "ledcmod.h"
#include "setfield.h"
#include "soc/ledc_struct.h"
#include "soc/ledc_reg.h"
#include "clockpacer.h"
#include "info.h"

void ledcmod::updateth() {
   int ch;
   int tim;

   while(true) {
      wait();
      for(ch = 0; ch < (LEDC_CHANNELS/2); ch = ch + 1) {
         conf0[ch].write(LEDC.channel_group[0].channel[ch].conf0.val);
         conf1[ch].write(LEDC.channel_group[0].channel[ch].conf1.val);
         hpoint[ch].write(LEDC.channel_group[0].channel[ch].hpoint.val);
         duty[ch].write(LEDC.channel_group[0].channel[ch].duty.val);
         conf0[ch+8].write(LEDC.channel_group[1].channel[ch].conf0.val);
         conf1[ch+8].write(LEDC.channel_group[1].channel[ch].conf1.val);
         hpoint[ch+8].write(LEDC.channel_group[1].channel[ch].hpoint.val);
         duty[ch+8].write(LEDC.channel_group[1].channel[ch].duty.val);
      }

      /* we do not actually build the timers, so we need to do some
       * precalculations for the channels.
       */
      sc_time base_period;
      for(tim = 0; tim < (LEDC_TIMERS/2); tim = tim + 1) {
         /* HSTIMER */
         timer_conf[tim] = LEDC.timer_group[1].timer[tim].conf.val;
         if (LEDC.timer_group[0].timer[tim].conf.tick_sel == 0)
            base_period = clockpacer.get_ref_period();
         else base_period = clockpacer.get_apb_period();

         /* TODO -- do the decimal part */
         timerinc[tim].write(
            sc_time(base_period
               * (LEDC.timer_group[0].timer[tim].conf.clock_divider>>8)));

         if (LEDC.timer_group[0].timer[tim].conf.rst
               || LEDC.timer_group[0].timer[tim].conf.pause)
            timer_ev[tim].notify();

         /* LSTIMER */
         timer_conf[tim+4] = LEDC.timer_group[1].timer[tim].conf.val;
         if (LEDC.timer_group[1].timer[tim].conf.tick_sel == 0)
            base_period = clockpacer.get_ref_period();
         else base_period = clockpacer.get_rtc8m_period();
         /* TODO -- do the decimal part */
         /* TODO -- do the pause */
         if (LEDC.timer_group[1].timer[tim].conf.low_speed_update) {
            timerinc[tim+4].write(sc_time(base_period
               * (LEDC.timer_group[1].timer[tim].conf.clock_divider>>8)));
         }
      }

      /* int_ena we simply copy. */
      int_ena.write(LEDC.int_ena.val);

      /* If we got a clear command, we trigger the event for the returnth to
       * handle it.
       */
      if (LEDC.int_clr.val != 0x0) int_clr_ev.notify();
   }
}

void ledcmod::returnth() {
   int un;
   /* We wait for one of the responses to get triggered, and then we copy the
    * R/O register back to the struct.
    */
   while (true) {
      wait(
         timer_cnt[0].value_changed_event()| timer_cnt[1].value_changed_event() |
         timer_cnt[2].value_changed_event()| timer_cnt[3].value_changed_event() |
         timer_cnt[4].value_changed_event()| timer_cnt[5].value_changed_event() |
         timer_cnt[6].value_changed_event()| timer_cnt[7].value_changed_event() |
         duty_r[0].value_changed_event()| duty_r[1].value_changed_event() |
         duty_r[2].value_changed_event()| duty_r[3].value_changed_event() |
         duty_r[4].value_changed_event()| duty_r[5].value_changed_event() |
         duty_r[6].value_changed_event()| duty_r[7].value_changed_event() |
         duty_r[8].value_changed_event()| duty_r[9].value_changed_event() |
         duty_r[10].value_changed_event()| duty_r[11].value_changed_event() |
         duty_r[12].value_changed_event()| duty_r[13].value_changed_event() |
         duty_r[14].value_changed_event()| duty_r[15].value_changed_event() |
         int_clr_ev |
         int_ev[0] | int_ev[1] | int_ev[2] | int_ev[3] | int_ev[4] | int_ev[5] |
         int_ev[6] | int_ev[7] | int_ev[8] | int_ev[9] | int_ev[10]| int_ev[11]|
         int_ev[12]| int_ev[13]| int_ev[14]| int_ev[15]| int_ev[16]| int_ev[17]|
         int_ev[18]| int_ev[19]| int_ev[20]| int_ev[21]| int_ev[22]| int_ev[23]);

      for (un = 0; un < LEDC_CHANNELS; un = un + 1) {
         LEDC.int_raw.val = 0x0;
         /* If we hit the maximum dither times, we have an interrupt and
          * clear the duty_start. */
         if (int_ev[un+8].triggered()) {
            LEDC.int_raw.val = LEDC.int_raw.val | (1<<un);
            if (un < LEDC_CHANNELS/2)
               LEDC.channel_group[0].channel[un].conf1.duty_start = false;
            else LEDC.channel_group[1].channel[un-8].conf1.duty_start = false;
         }
         /* We copy the duty values */
         if (un < LEDC_CHANNELS/2)
               LEDC.channel_group[0].channel[un].duty_rd.duty_read =
            duty_r[un].read();
         else LEDC.channel_group[1].channel[un-8].duty_rd.duty_read =
            duty_r[un-8].read();
      }
      /* We also copy over the timer values and interrupts. */
      for(un = 0; un < (LEDC_TIMERS/2); un = un + 1) {
         if (int_ev[un].triggered())
            LEDC.int_raw.val = LEDC.int_raw.val | (1<<un);
         LEDC.timer_group[0].timer[un].value.timer_cnt = timer_cnt[un].read();
         if (int_ev[un+4].triggered())
            LEDC.int_raw.val = LEDC.int_raw.val | (1<<un+4);
         LEDC.timer_group[1].timer[un].value.timer_cnt = timer_cnt[un].read();
      }

      /* If we have a clear event we take it too. */
      if (int_clr_ev.triggered()) {
         LEDC.int_raw.val = LEDC.int_raw.val & ~LEDC.int_clr.val;
         LEDC.int_clr.val = 0;
      }
      /* We update the raw. */
      LEDC.int_st.val = LEDC.int_raw.val & int_ena.read();

      /* We also drive the interrupt line */
      intr_o.write(LEDC.int_st.val != 0);
   }
}

void ledcmod::update() {
   update_ev.notify();
   clockpacer.wait_next_apb_clk();
}

void ledcmod::initstruct() {
   memset(&LEDC, 0, sizeof(ledc_dev_t));
}

void ledcmod::start_of_simulation() {
   /* We spawn a thread for each channel and timer. */
   int un;
   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      conf0[un].write(0);
      conf1[un].write(0);
      hpoint[un].write(0);
      duty[un].write(0);
   }
   for(un = 0; un < sig_out_hs_o.size(); un = un + 1) {
      sc_spawn(sc_bind(&ledcmod::channel, this, un));
      sig_out_hs_o[un]->write(false);
   }
   for(un = LEDC_CHANNELS/2; un < sig_out_ls_o.size(); un = un + 1) {
      sc_spawn(sc_bind(&ledcmod::channel, this, un));
      sig_out_ls_o[un-LEDC_CHANNELS/2]->write(false);
   }
   for(un = 0; un < LEDC_TIMERS; un = un + 1) {
      sc_spawn(sc_bind(&ledcmod::timer, this, un));
      timer_cnt[un].write(0);
      timer_lim[un].write(0);
      timerinc[un].write(sc_time(0, SC_NS));
   }
   int_ena.write(0);
}

void ledcmod::calc_points(int un, bool start_dither) {
   int lp;
   int lpoint_frac;
   bool duty_start;
   int duty_num;
   int duty_cycle;
   bool duty_inc;
   int duty_scale;

   /* We first take the values into local registers to not have to keep
    * placing these lines in the middle of the code.
    */
   if (un < LEDC_CHANNELS/2) {
      duty_start =
         (RDFIELD(conf1[un],LEDC_DUTY_START_HSCH0_M,LEDC_DUTY_START_HSCH0_S)>0);
      duty_num =
         RDFIELD(conf1[un],LEDC_DUTY_NUM_HSCH0_M,LEDC_DUTY_NUM_HSCH0_S);
      duty_cycle =
        RDFIELD(conf1[un],LEDC_DUTY_CYCLE_HSCH0_M,LEDC_DUTY_CYCLE_HSCH0_S);
      duty_inc =
        (RDFIELD(conf1[un],LEDC_DUTY_CYCLE_HSCH0_M,LEDC_DUTY_INC_HSCH0_S)>0);
      duty_scale =
        RDFIELD(conf1[un],LEDC_DUTY_CYCLE_HSCH0_M,LEDC_DUTY_SCALE_HSCH0_S);
   }
   else {
      duty_start =
         (RDFIELD(conf1[un],LEDC_DUTY_START_LSCH0_M,LEDC_DUTY_START_LSCH0_S)>0);
      duty_num =
         RDFIELD(conf1[un],LEDC_DUTY_NUM_LSCH0_M,LEDC_DUTY_NUM_LSCH0_S);
      duty_cycle =
        RDFIELD(conf1[un],LEDC_DUTY_CYCLE_LSCH0_M,LEDC_DUTY_CYCLE_LSCH0_S);
      duty_inc =
        (RDFIELD(conf1[un],LEDC_DUTY_CYCLE_LSCH0_M,LEDC_DUTY_INC_LSCH0_S)>0);
      duty_scale =
        RDFIELD(conf1[un],LEDC_DUTY_CYCLE_LSCH0_M,LEDC_DUTY_SCALE_LSCH0_S);
   }

   /* We get the lpoint and frac. */
   lp = (duty[un].read() >> 4);
   lpoint_frac = duty[un].read() & 0x0f;

   /* We adjust the lpoint according to the fraction */
   if (lpoint_frac > thiscyc[un]) lp = lp + 1;

   /* And we adjust the lpoint according to the dither. */
   if (duty_start) {
      if (start_dither) {
         /* If this is the first time, dithcycles will be zero. We then
          * start by loading the settings of the dither and go on.
          */
         dithtimes[un] = duty_num;
         dithcycles[un] = duty_cycle;
      }
      else {
         /* Other times we count the cycles and process it. */
         dithcycles[un] = dithcycles[un] - 1;

         /* If we hit the target, we are done. */
         if (dithcycles == 0 && duty_inc) {
            lp = lp + duty_scale;
            dithcycles[un] = duty_cycle;
            dithtimes[un] = dithtimes[un] - 1;
         }
         else if (dithcycles[un] == 0) {
            lp = lp - duty_scale;
            dithcycles[un] = duty_cycle;
            dithtimes[un] = dithtimes[un] - 1;
         }

         int_ev[un+8].notify();
      }
   }

   /* We put the lp value in the read/only register. */
   duty_r[un].write(lp & LEDC_DUTY_HSCH0);
   thislp[un] = lp & LEDC_DUTY_HSCH0;
}

void ledcmod::channel(int ch) {
   bool outen;
   bool recheckpoints;
   /* Sel begins with -1 and it then is switched to the correct value. */
   int sel = -1;
   while(1) {
      /* If there is no selected timer, all we do is wait for a configuration
       * change. If there is a timer specified, we wait for a timer trigger
       * or a configuration change.
       */
      if (sel < 0) wait(conf0[ch].value_changed_event());
      else wait(conf0[ch].value_changed_event() | conf1[ch].value_changed_event()
         | hpoint[ch].value_changed_event() | duty[ch].value_changed_event()
         | timer_ev[sel]);

      /* We go ahead and grab the output enable as we use it quite often. */
      if (ch < LEDC_CHANNELS/2) {
         outen =
            RDFIELD(conf0[ch], LEDC_SIG_OUT_EN_HSCH0_M, LEDC_SIG_OUT_EN_HSCH0_S);
         sel= RDFIELD(conf0[ch], LEDC_TIMER_SEL_HSCH0_M, LEDC_TIMER_SEL_HSCH0_S);
      }
      else {
         outen =
            RDFIELD(conf0[ch], LEDC_SIG_OUT_EN_LSCH0_M, LEDC_SIG_OUT_EN_LSCH0_S);
         sel= RDFIELD(conf0[ch], LEDC_TIMER_SEL_LSCH0_M, LEDC_TIMER_SEL_LSCH0_S);
      }

      /* First we process changes in the PWM. These can affect the rest. */
      /* If we got a trigger on the output and the channel is stopped, we
       * update the value.
       * The timer we ignore as the timer mux is done outside the channel.
       */
      if (conf0[ch].event() && !outen) {
         if (ch < LEDC_CHANNELS/2) sig_out_hs_o[ch]->write(
            RDFIELD(conf0[ch], LEDC_IDLE_LV_HSCH0_M, LEDC_IDLE_LV_HSCH0_S));
         else sig_out_ls_o[ch-LEDC_CHANNELS/2]->write(
            RDFIELD(conf0[ch], LEDC_IDLE_LV_LSCH0_M, LEDC_IDLE_LV_LSCH0_S));
      }

      /* If we see a change in the hpoint or the duty we need to recalculate
       * the lpoint. We also need to do this when the timer is switched or when
       * we start a new cycle.
       */
      if (duty[ch].event() || conf0[ch].event()) {
         calc_points(ch, true);
         /* We restart the cycle calculation. */
         thiscyc[ch] = 0;
         /* And we will need to recheck the hpoint and lpoint. */
         recheckpoints = true;
      }
      /* Anytime the cycle restarts (timer returns to zero) we need to
       * increment the cycle counter and adjust the jitter, if any.
       */
      else if (timer_cnt[sel].read() == 0) {
         /* We start adjusting the cycle number for the dither. */
         thiscyc[ch] = thiscyc[ch] + 1;
         if (thiscyc[ch] == LEDC_CYCLES) thiscyc[ch] = 0;
         /* We also calculate the lpoint. */
         calc_points(ch, false);
         /* And we will need to recheck the hpoint and lpoint. */
         recheckpoints = true;
      }
      else recheckpoints = false;

      /* If it was a timer tick, we need to see if it affects us. */
      if (outen && (timer_cnt[sel].event() || recheckpoints)) {
         /* And we check where we are in the flow. */
         bool nv;
         if (timer_cnt[sel].read() >= thislp[ch] + hpoint[ch].read()) nv = false;
         else if (timer_cnt[sel].read() >= hpoint[ch]) nv = true;
         else nv = false;
         if (ch < LEDC_CHANNELS/2) sig_out_hs_o[ch]->write(nv);
         else sig_out_ls_o[ch-LEDC_CHANNELS/2]->write(nv);
      }
   }
}

void ledcmod::timer(int tim) {
   int rst;
   int pause;
   while(1) {
      /* We wait for a timer tick or a change to the configuration register. */
      wait(timer_ev[tim] | timer_conf[tim].value_changed_event() |
         timerinc[tim].value_changed_event());

      /* We get the parameters first. */
      if (tim < LEDC_TIMERS/2) {
         rst = RDFIELD(timer_conf[tim], LEDC_HSTIMER0_RST_M,
            LEDC_HSTIMER0_RST_S);
         pause = RDFIELD(timer_conf[tim], LEDC_HSTIMER0_PAUSE_M,
            LEDC_HSTIMER0_PAUSE_S);
      }
      else {
         rst = RDFIELD(timer_conf[tim], LEDC_LSTIMER0_RST_M,
            LEDC_LSTIMER0_RST_S);
         pause = RDFIELD(timer_conf[tim], LEDC_LSTIMER0_PAUSE_M,
            LEDC_LSTIMER0_PAUSE_S);
      }

      /* If we are in reset, we clear any future time events and wait. The next
       * event should be then a configuration change.
       */
      if (rst) {
         timer_cnt[tim].write(0);
         timer_ev[tim].cancel();
      }
      /* If we are paused, we just cancel any future events, but we do not
       * touch the counter value.
       */
      else if (pause) { timer_ev[tim].cancel(); }
      /* If the timer increment is zero, we do the same. */
      else if (timerinc[tim].read() == sc_time(0, SC_NS))
         { timer_ev[tim].cancel(); }
      /* If we are not paused nor counting we then count. */
      else {
         /* We only count on timer event triggers. Configuration events should
          * not change the timer value.
          */
         if (timer_ev[tim].triggered() &&
            timer_cnt[tim].read() < timer_lim[tim].read() - 1)
            timer_cnt[tim].write(timer_cnt[tim].read() + 1);
         else {
            /* We hit the end we write a zero and raise the interrupt. */
            timer_cnt[tim].write(0);
            int_ev[tim+8].notify();
         }
         /* And we sleep until the next event. */
         timer_ev[tim].notify(timerinc[tim]);
      }
   }
}

void ledcmod::trace(sc_trace_file *tf) {
   int un;
   std::string sigb = name();
   std::string sign;

   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      sign = sigb + std::string(".conf0_") + std::to_string(un);
      sc_trace(tf, conf0[un], sign.c_str());
   }
   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      sign = sigb + std::string(".conf1_") + std::to_string(un);
      sc_trace(tf, conf1[un], sign.c_str());
   }
   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      sign = sigb + std::string(".hpoint_") + std::to_string(un);
      sc_trace(tf, hpoint[un], sign.c_str());
   }
   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      sign = sigb + std::string(".duty_") + std::to_string(un);
      sc_trace(tf, duty[un], sign.c_str());
   }
   for(un = 0; un < LEDC_CHANNELS; un = un + 1) {
      sign = sigb + std::string(".duty_r") + std::to_string(un);
      sc_trace(tf, duty_r[un], sign.c_str());
   }
   for(un = 0; un < LEDC_TIMERS; un = un + 1) {
      sign = sigb + std::string(".timer_cnt_") + std::to_string(un);
      sc_trace(tf, timer_cnt[un], sign.c_str());
   }
   for(un = 0; un < LEDC_TIMERS; un = un + 1) {
      sign = sigb + std::string(".timer_lim_") + std::to_string(un);
      sc_trace(tf, timer_lim[un], sign.c_str());
   }
}
