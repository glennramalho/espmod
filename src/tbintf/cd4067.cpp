/*******************************************************************************
 * cd4067.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a testbench module to emulate the CD4067 display controller.
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

void cd4067::process_th() {
   int line;
   x.write(GN_LOGIC_Z);
   for(line = 0; line < channel.size(); line = line + 1)
      channel[line]->write(GN_LOGIC_Z);
   sel = (((c.read())?1:0) << 2) | (((b.read())?1:0) << 1) |
      ((a.read())?1:0);

   while(true) {
      if (sel >= channel.size()) wait(a | b | c | inh);
      else wait(channel[sel]->default_event()
            | a->default_event() | b->default_event() | c->default_event()
            | d->default_event() | inh->default_event() | x->default_event());
      
      if (debug) printf(
         "sel: %d - channel -> %c/ a %c/ b %c/ c %c/ d %c/ inh %c/ x %c\n", sel,
            (channel[sel]->value_changed_event().triggered())?'t':'f',
            (a->value_changed_event().triggered())?'t':'f',
            (b->value_changed_event().triggered())?'t':'f',
            (c->value_changed_event().triggered())?'t':'f',
            (d->value_changed_event().triggered())?'t':'f',
            (inh->value_changed_event().triggered())?'t':'f',
            (x->value_changed_event().triggered())?'t':'f');

      /* If inh went high we shut it all off. */
      if (inh.read() == GN_LOGIC_1) {
         x.write(GN_LOGIC_Z);
         channel[sel]->write(GN_LOGIC_Z);
      }

      /* We have an analog mux. In a real mux system, we would do a fight
       * between the sides and eventually settle to something. This is a
       * digital simulator though, so we need a simplification. If I get a
       * driver on one side, we drive that value on the other.
       */

      /* If there was a change in the selector, we adjust that first. */
      if (a.value_changed_event().triggered() ||
            b.value_changed_event().triggered() ||
            c.value_changed_event().triggered() ||
            d.value_changed_event().triggered()) {
         int newsel =
            (((d.read())?1:0) << 3) |
            (((c.read())?1:0) << 2) | (((b.read())?1:0) << 1) |
            ((a.read())?1:0);

         /* If the driver changed, we then undrive the old one. */
         if (sel != newsel) {
            channel[sel]->write(GN_LOGIC_Z);
         }

         /* We only drive the paths if the sel is valid. */
         sel = newsel;
         if (sel >= channel.size()) x.write(GN_LOGIC_Z);
         else {
            x.write(channel[sel]->read());
            channel[sel]->write(x.read());
         }
      }

      /* Now we drive the new one. */
      else if (channel[sel]->value_changed_event().triggered())
         x.write(channel[sel]->read());
      else if (x.default_event().triggered())
         channel[sel]->write(x.read());
   }
}

void cd4067::trace(sc_trace_file *tf) {
   int un;
   std::string sigb = name();
   std::string sign;

   sc_trace(tf, x, x.name());
   sc_trace(tf, a, a.name());
   sc_trace(tf, b, b.name());
   sc_trace(tf, c, c.name());
   sc_trace(tf, d, d.name());
   sc_trace(tf, inh, inh.name());

   /* The channels we can only display the ones that are used. */
   for(un = 0; un < channel.size(); un = un + 1) {
      sign = sigb + std::string(".channel_") + std::to_string(un);
      sc_trace(tf, channel[un], sign);
   }
}
