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

#include <systemc.h>
#include "ledcmod.h"
#include "soc/ledc_struct.h"
#include "soc/ledc_reg.h"

void ledcmod::updateth() {
   int ch;
   int tim;

   while(true) {
      wait();
      for(ch = 0; ch < 8; ch = ch + 1; {
         conf0[un].write(LEDC.channel_group[0].channel[ch].conf0.val);
         conf1[un].write(LEDC.channel_group[0].channel[ch].conf1.val);
         hpoint[un].write(LEDC.channel_group[0].channel[ch].hpoint.val);
         duty[un].write(LEDC.channel_group[0].channel[ch].duty.val);
         conf0[un+8].write(LEDC.channel_group[1].channel[ch].conf0.val);
         conf1[un+8].write(LEDC.channel_group[1].channel[ch].conf1.val);
         hpoint[un+8].write(LEDC.channel_group[1].channel[ch].hpoint.val);
         duty[un+8].write(LEDC.channel_group[1].channel[ch].duty.val);
      }

      /* we do not actually build the timers, so we need to do some
       * precalculations for the channels.
       */
      for(tim = 0; tim < 8; tim = tim + 1) {
         /* HSTIMER */
         if (LEDC.timer_group[0].timer[tim].tick_sel == 0) {
            PRINTF_WARNING("LEDC", "REF_TICK has not yet been implemented");
         }
         /* TODO -- do the decimal part */
         timeinc[tim].write(
            apb_clk * (LEDC.timer_group[0].timer[tim].clock_divider>>8));
         timestep[tim].write(
            apb_clk * (LEDC.timer_group[0].timer[tim].clock_divider>>8)
            * (1<<LEDC.timer_group[0].timer[tim].duty_resolution));
         period[tim].write(1<<LEDC.timer_group[0].timer[tim].duty_resolution);

         if (LEDC.timer_group[0].timer[tim].rst
               || LEDC.timer_group[0].timer[tim].pause)
            timer_ev[tim].notify();

         /* LSTIMER */
         /* TODO -- SLOW_CLK */
         if (LEDC.timer_group[1].timer[tim].tick_sel == 0) {
            PRINTF_WARNING("LEDC", "REF_TICK has not yet been implemented");
         }
         /* TODO -- do the decimal part */
         /* TODO -- do the pause */
         if (LEDC.timer_group[1].timer[tim].rst
               || LEDC.timer_group[1].timer[tim].pause)
            timer_ev[tim].notify();
         if (LEDC.timer_group[1].timer[tim].low_speed_update) {
            timeinc[tim].write(
               apb_clk * (LEDC.timer_group[1].timer[tim].clock_divider>>8));
            timestep[tim].write(
               apb_clk * (LEDC.timer_group[1].timer[tim].clock_divider>>8)
               * (1<<LEDC.timer_group[1].timer[tim].duty_resolution));
         }
      }
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
         int_raw[0].value_changed_event() | int_raw[1].value_changed_event() |
         int_raw[2].value_changed_event() | int_raw[3].value_changed_event() |
         int_raw[4].value_changed_event() | int_raw[5].value_changed_event() |
         int_raw[6].value_changed_event() | int_raw[7].value_changed_event() |
         int_raw[8].value_changed_event() | int_raw[9].value_changed_event() |
         int_raw[10].value_changed_event()| int_raw[11].value_changed_event() |
         int_raw[12].value_changed_event()| int_raw[13].value_changed_event() |
         int_raw[14].value_changed_event()| int_raw[15].value_changed_event() |
         int_raw[16].value_changed_event()| int_raw[17].value_changed_event() |
         int_raw[18].value_changed_event()| int_raw[19].value_changed_event() |
         int_raw[20].value_changed_event()| int_raw[21].value_changed_event() |
         int_raw[22].value_changed_event()| int_raw[23].value_changed_event());

      for (un = 0; un < 16; un = un + 1) {
         LEDC.int_raw.val = 0x0;
         /* If we hit the maximum dither times, we have an interrupt and
          * clear the duty_start. */
         if (int_raw[un+8].read()) {
            LEDC.int_raw[un+8].val = LEDC.int_raw[un+8].val | (1<<un);
            if (un < 8)
               LEDC.channel_group[0].channel[un].conf1.duty_start = false;
            else LEDC.channel_group[1].channel[un-8].conf1.duty_start = false;
         }
         /* We copy the duty values */
         if (un < 8) LEDC.channel_group[0].channel[un].duty_rd.duty_read =
            duty_r[un].read();
         else LEDC.channel_group[1].channel[un-8].duty_rd.duty_read =
            duty_r[un-8].read();
      }
      /* We also copy over the timer values and interrupts. */
      for(un = 0; un < 4; un = un + 1) {
         LEDC.int_raw[un].val = LEDC.int_raw[un].val | int_raw[un].read();
         LEDC.timer_group[0].timer[un].value.timer_cnt[un]
            = timer_cnt[un].read();

      }
      for(un = 0; un < 4; un = un + 1) {
         LEDC.int_raw[un+4].val = LEDC.int_raw[un+4].val | int_raw[un+4].read();
         LEDC.timer_group[1].timer[un].value.timer_cnt[un+8]
            = timer_cnt[un].read();
      }
   }
}

void ledcmod::update() {
   update_ev.notify();
   wait_next_apb_clock();
}

void ledcmod::initstruct() {
   memset(&LEDC, 0, sizeof(ledc_dev_t));
}

void ledcmod::start_of_simulation() {
   /* We spawn a thread for each channel and timer. */
   int un;
   for(un = 0; un < 16; un = un + 1) {
      sc_spawn(sc_bind(&ledcmod::channel, this, un));
      sig_out[un].write(false);
   }
   for(un = 0; un < 8; un = un + 1)
      sc_spawn(sc_bind(&ledcmod::timer, this, un));

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
   if (un < 8) {
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
         dithcycles = dithcycles - 1;

         /* If we hit the target, we are done. */
         if (dithcycles == 0 && duty_inc)
            lp = lp + duty_scale;
            dithcycles = duty_cycle;
            dithtimes = dithtimes - 1;
         }
         else if (dithcycles == 0) {
            lp = lp - duty_scale;
            dithcycles = duty_cycle;
            dithtimes = dithtimes - 1;
         }

         int_raw[ch+8].write(1, SC_NS);
      }
   }

   /* We put the lp value in the read/only register. */
   duty_r[un].write(lp & LEDC_DUTY_HSCH0);
   thislp[un] = lp & LEDC_DUTY_HSCH0;
}

void ledcmod::channel(int ch) {
   bool outen;
   bool recheckpoints;
   int sel;
   while(1) {
      /* We wait for a timer to tick or a change in the configuration. */
      wait();

      /* We go ahead and grab the output enable as we use it quite often. */
      if (un < 8) {
         outen =
            RDFIELD(conf0[un], LEDC_SIG_OUT_EN_HSCH0_M, LEDC_SIG_OUT_EN_HSCH0_S);
         sel= RDFIELD(conf0[un], LEDC_TIMER_SEL_HSCH0_M, LEDC_TIMER_SEL_HSCH0_S);
      }
      else {
         outen =
            RDFIELD(conf0[un], LEDC_SIG_OUT_EN_LSCH0_M, LEDC_SIG_OUT_EN_LSCH0_S);
         sel= RDFIELD(conf0[un], LEDC_TIMER_SEL_LSCH0_M, LEDC_TIMER_SEL_LSCH0_S);
      }

      /* First we process changes in the PWM. These can affect the rest. */
      /* If we got a trigger on the output and the channel is stopped, we
       * update the value.
       * The timer we ignore as the timer mux is done outside the channel.
       */
      if (conf0[un].event() && !outen) {
         if (un < 8) sig_out[un].write(
            RDFIELD(conf0[un], LEDC_IDLE_LV_HSCH0_M, LEDC_IDLE_LV_HSCH0_S))
         else sig_out[un].write(
            RDFIELD(conf0[un], LEDC_IDLE_LV_LSCH0_M, LEDC_IDLE_LV_LSCH0_S))

      }

      /* If we see a change in the hpoint or the duty we need to recalculate
       * the lpoint. We also need to do this when the timer is switched or when
       * we start a new cycle.
       */
      if (duty[un].event() || timer_sel[un].event()) {
         calc_points(un, true);
         /* We restart the cycle calculation. */
         thiscyc[un] = 0;
         /* And we will need to recheck the hpoint and lpoint. */
         recheckpoints = true;
      }
      /* Anytime the cycle restarts (timer returns to zero) we need to
       * increment the cycle counter and adjust the jitter, if any.
       */
      else if (timer_cnt[sel].read() == 0) {
         /* We start adjusting the cycle number for the dither. */
         thiscyc[un] = thiscyc[un] + 1;
         if (thiscyc[un] == 16) thiscyc[un] = 0;
         /* We also calculate the lpoint. */
         calc_points();
         /* And we will need to recheck the hpoint and lpoint. */
         recheckpoints = true;
      }
      else recheckpoints = false;

      /* If it was a timer tick, we need to see if it affects us. */
      if (outen && (timer_cnt[sel].event() || recheckpoints)) {
         /* And we check where we are in the flow. */
         if (timer_cnt[sel].read() >= thislp[un] + hpoint[un].read())
            sig_out[un].write(false);
         else if (timer_cnt[sel].read() >= hpoint[un]) sig_out[un].write(true);
         else sig_out[un].write(false);
      }
   }
}

void ledcmod::timer(int tim) {
   int ch;
   while(1) {
      /* If we got a reset, we then restart the timer. */
      if (rst) timer_cnt[tim].write(0);
      /* We only count if we are not paused. */
      if (!pause) {
         /* If we are not paused, we increment. */
         if (timer_cnt[tim].read() != timer_lim[tim].read() - 1) {
            timer_cnt[tim].write(timer_cnt[tim].read() + 1);
         }
         else {
            /* We hit the end we write a zero and raise the interrupt. */
            timer_cnt[tim].write(0);
            int_raw[tim+8].write(true);
         }
      }
   }
}

void pcntmod::trace(sc_trace_file *tf) {
   int un;
   std::string sigb = name();
   std::string sign = sigb + ".conf0_0";
   int digit;

   digit = sign.length() - 1;
   for(un = 0; un < 16; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf0[un], sign.c_str());
   }
   sign = sigb + ".conf1_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf0[un], sign.c_str());
   }
   sign = sigb + ".hpoint_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, conf1[un], sign.c_str());
   }
   sign = sigb + ".duty_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, duty[un], sign.c_str());
   }
   sign = sigb + ".duty_r_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, duty_r[un], sign.c_str());
   }
   sign = sigb + ".timer_cnt_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, timer_cnt[un], sign.c_str());
   }
   sign = sigb + ".timer_lim_0";
   digit = sign.length() - 1;
   for(un = 0; un < 8; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, timer_lim[un], sign.c_str());
   }
   sign = sigb + ".int_raw_0";
   digit = sign.length() - 1;
   for(un = 0; un < 24; un = un + 1) {
      sign[digit] = '0' + un; sc_trace(tf, int_raw[un], sign.c_str());
   }
}
