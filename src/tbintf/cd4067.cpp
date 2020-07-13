/*******************************************************************************
 * cd4067.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *  This is a simple model for the CD4067 Analog Mux.
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
#include "info.h"
#include "cd4067.h"

int cd4067::get_selector() {
   return
      (((d.read())?1:0) << 3) |
      (((c.read())?1:0) << 2) |
      (((b.read())?1:0) << 1) |
      ((a.read())?1:0);
}

void cd4067::process_th() {
   int line;
   bool channel_is_driver = false;
   x.write(GN_LOGIC_Z);
   for(line = 0; line < channel.size(); line = line + 1)
      channel[line]->write(GN_LOGIC_Z);
   sel = get_selector();
   bool recheck = false;

   while(true) {
      /* If the recheck is high, this means the selector changed and we need
       * to refigure out who is driving.
       */
      if (recheck) {
         /* We wait a time stamp and redo the driving. */
         wait(1, SC_NS);
         recheck = false;

         /* In these cases, we just keep everyone at Z. */
         if (inh.read() || sel >= channel.size()) continue;

         /* We check who is the driver and drive the other. If they are both
          * being driven, we drive X everywhere.
          */
         if (x.read() == GN_LOGIC_Z) {
            channel_is_driver = true;
            x.write(channel[sel]->read());
         }
         else if (channel[sel]->read()==GN_LOGIC_Z) {
            channel_is_driver = false;
            channel[sel]->write(x.read());
         }
         else {
            PRINTF_WARN("TEST", "Two drivers on MUX");
            channel[sel]->write(GN_LOGIC_X);
            x.write(GN_LOGIC_X);
         }

         continue;
      }

      /* We wait for one either a change  in the selector or a change in
       * either side of the mux. If the selector is pointing to an illegal
       * value, we then just wait for a change in the selector.
       */
      if (sel >= channel.size())
         wait(a->default_event() | b->default_event() | c->default_event()
            | d->default_event() | inh->default_event());
      else wait(channel[sel]->default_event()
            | a->default_event() | b->default_event() | c->default_event()
            | d->default_event() | inh->default_event() | x->default_event());

      bool sel_triggered = 
         a->value_changed_event().triggered() ||
         b->value_changed_event().triggered() ||
         c->value_changed_event().triggered() ||
         d->value_changed_event().triggered();

      if (debug) {
         /* If the sel is illegal, we only can process the selector. */
         if (sel >= channel.size())
            printf("%s: sel = %d, triggers: sel %c, inh %c, x %c\n",
            name(), sel, (sel_triggered)?'t':'f',
            (inh->value_changed_event().triggered())?'t':'f',
            (x->value_changed_event().triggered())?'t':'f');
         else printf(
            "%s: sel = %d, triggers: channel %c, sel %c, inh %c, x %c\n",
            name(), sel,
            (channel[sel]->value_changed_event().triggered())?'t':'f',
            (sel_triggered)?'t':'f',
            (inh->value_changed_event().triggered())?'t':'f',
            (x->value_changed_event().triggered())?'t':'f');
      }

      /* If inh went high we shut it all off, it does not matter who triggered
       * what.
       */
      if (inh.read() == GN_LOGIC_1) {
         x.write(GN_LOGIC_Z);
         if (sel < channel.size()) channel[sel]->write(GN_LOGIC_Z);
         channel_is_driver = false;
      }
      /* If inh came back, we then need to resolve. */
      else if (inh.value_changed_event().triggered()) {
         /* No need to drive Z if it is already. */
         recheck = true;
         channel_is_driver = false;
      }
      /* Now we check the selector. If it changed, we need to drive Z and
       * recheck. This is because  it is difficult to check every single
       * driver in a signal to find out what is going on.
       */
      else if (sel_triggered) {
         int newsel = get_selector();

         /* We now drive Z everywhere. */
         if (sel < channel.size()) channel[sel]->write(GN_LOGIC_Z);
         x.write(GN_LOGIC_Z);
         channel_is_driver = false;

         /* And we update the selector variable. */
         sel = newsel;

         /* And if the new selector is valid, we do a recheck. */
         if (sel < channel.size()) recheck = true;
      }

      /* If the selector is not valid, we just ignore any activity on either
       * side as there can't be any change. We simply drive Z everywhere.
       * Note: this path is a safety one, it is probably never called.
       */
      else if (sel >= channel.size()) {
         channel_is_driver = false;
         x.write(GN_LOGIC_Z);
      }

      /* When we update a output pin, there is always a reflection. If both
       * sides have the same value, this probably was a reflection, so we
       * can dump it.
       */
      else if (x.read() == channel[sel]->read()) continue;

      /* We have an analog mux. In a real mux system, we would do a fight
       * between the sides and eventually settle to something. This is a
       * digital simulator though, so we need a simplification. If I get a
       * driver on one side, we drive that value on the other.
       */
      else if (channel[sel]->value_changed_event().triggered()
            && (channel_is_driver || x.read() == GN_LOGIC_Z)) {
         channel_is_driver = true;
         x.write(channel[sel]->read());
      }
      else if (x.default_event().triggered()
            && (!channel_is_driver || channel[sel]->read() == GN_LOGIC_Z)) {
         channel_is_driver = false;
         channel[sel]->write(x.read());
      }
      /* If there are drivers on both sides, we need to put X on both sides.
       * Note: the current model will not leave this state unless there is
       * an inh or sel change.
       */
      else {
         PRINTF_WARN("TEST", "Two drivers on MUX");
         channel[sel]->write(GN_LOGIC_X);
         x.write(GN_LOGIC_X);
      }
   }
}

void cd4067::trace(sc_trace_file *tf) {
   sc_trace(tf, x, x.name());
   sc_trace(tf, a, a.name());
   sc_trace(tf, b, b.name());
   sc_trace(tf, c, c.name());
   sc_trace(tf, d, d.name());
   sc_trace(tf, inh, inh.name());
}
