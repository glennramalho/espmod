/*******************************************************************************
 * ledcmod.cpp -- Copyright 2020 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 * Implements a SystemC module for the ESP32 PWM High-speed channel
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
   /* We wait for one of the responses to get triggered, and then we copy the
    * R/O register back to the struct.
    */
   wait() {
      int unint;
      LEDC.int_raw.val = 0;
      for(unint = 0; unint < 24; unint = unint + 1)
         LEDC.int_raw.val = 1 << (int_raw[unint].read());




         /* If we hit the maximum number of times, we clear the duty_start. */
         LEDC.channel_group[x].channel[ch].conf1.duty_start = 0;
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
      sc_spawn(sc_bind(&ledcmod::capture, this, un));
      sc_spawn(sc_bind(&ledcmod::count, this, un));
   }
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
   /* Now we calculate the timing for the lpoint. */
   lpoint = lp * timestep[un].read();
   /* We put the lp value in the read/only register. */
   LEDC.channel_group[x].channel[ch].duty_rd.duty_read = lp;
   /* The hpoint we simply take. */
   hpoint = hpoint_i.read() * timestep[un].read();
}

void ledcmod::channel(int ch) {
   enum {OVERFLOW, HPOINT, LPOINT} state = OVERFLOW;
   bool outen;
   while(1) {
      /* We go ahead and grab the output enable as we use it quite often. */
      if (un < 8) outen =
         RDFIELD(conf0[un], LEDC_SIG_OUT_EN_HSCH0_M, LEDC_SIG_OUT_EN_HSCH0_S);
      else outen =
         RDFIELD(conf0[un], LEDC_SIG_OUT_EN_LSCH0_M, LEDC_SIG_OUT_EN_LSCH0_S);

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

      /* If we see a change to one of the timer parameters, we need to
       * recalculate where we are.
       */
      if (hpoint[un].event() || duty[un].event()) {
         /* We restart the cycle calculation. */
         thiscyc[un] = 0;
         /* Recalculate the hpoint and lpoint. */
         calc_points(un, true);

         /* We need to find out where we are first. */
         current_pos = sc_time_stamp() - starttime;
         if (current_pos < hpoint[un]) {
            if (outen) sig_out[un].write(false);
            chev[un].notify(hpoint[un] - current_pos);
            state = LPOINT;
         }
         else if (current_pos < hpoint + lpoint) {
            if (outen) sig_out[un].write(true);
            chev[un].notify(lpoint[un] + hpoint[un] - current_pos);
            state = LPOINT;
         }
         else {
            /* We are post lpoint, so all we do is take the line low and
             * wait for the overlap.
             */
            if (outen) sig_out[un].write(false);
            state = OVERLAP;
         }
      }
      else if (overlap.triggered()) {
         /* We start adjusting the cycle number for the dither. */
         thiscyc[un] = thiscyc[un] + 1;
         if (thiscyc[un] == 16) thiscyc[un] = 0;
         /* and we calculate the hpoint and lpoint. */
         calc_points(un, false);
         /* Anytime the timer overlaps, we restart to make sure all timers
          * are synchronized.
          */
         if (hpoint[un] == sc_time(0, SC_NS)) {
            /* If hpoit is zero, we are in the |"|_.
             * We start with the ramp going high and wait until the lpoint.
             */
            if (outen) sig_out[un].write(true);
            chev.notify(lpoint[un]);
            state = LPOINT;
         }
         else {
            /* Can be: _|"| or _|"|_ 
             * Here, we are waiting for the hpoint. Therefore we take the
             * output low (or high if it is inverted) and wait for the
             * hpoint.
             */
            sig_out[un].write(false);
            chev.notify(hpoint[un]);
            state = HPOINT;
         }
      }
      else if (state == HPOINT) {
         /* If we are at the hpoint, we take the wave high and wait for the
          * lpoint.
          */
         sig_out[un].write(true);
         /* We only trigger the event if we do not have the case where the
          * wave ends on the lowpoint.
          */
         if (lpoint[un] + hpoint[un] != period[un]) state = LPOINT;
         chev.notify(lpoint);
      }
      else if (state == LPOINT) {
         /* If we are in the lpoint state all we do is take the wave low. We
          * do not need to trigger the event as we know we will keep it low
          * until the next overlap.
          */
         sig_out[un].write(false);
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
         /* We now increment the timer value. */
         if (timer_cnt[tim].read() != timer_lim[tim].read() - 1) {
            timer_cnt[tim].write(timer_cnt[tim].read() + 1);
         }
         else {
            /* We hit a overlap. We need to notify the channels. */
            timer_cnt.notify(timeinc[tim]);
            /* And raise the interrupt. */
            int_ovl[tim+8].write(true);
            /* We also refresh the timer steps. */
            timestep[tim].write(timeinc[tim].read());
         }
         timer_cnt.notify(timeinc[tim]);
      }
   }
}
