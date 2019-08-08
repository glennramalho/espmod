/*******************************************************************************
 * Description:
 *   This is a derived class from sc_signal_resolved which resolves to 1
 *   instead of Z.
 *
 *   It was based off the example ea_signal_pullup in figure 9-14 of
 *   David C. Black and Jack Donovan (2004) "SystemC: From the Ground Up"
 *      Boston: Kluwer Academic Publishers
 *   Also by Davif C. Black (2005) SystemC Forum from 2005
 *      "systemc-forum - Re: [Systemc-forum] OC with Pullup"
 *
 *   This is intended mainly as a way to simplify more complex communication
 *   protocols and does not represent any existing protocol.
 */

#include <systemc.h>
#include "ea_pullup.h"
#include "info.h"

ea_pullup::ea_pullup() {}

void ea_pullup::update() {
   int bit;
   sc_logic resolved_val;
   int size = m_val_vec.size();

   /* If the size is zero, we quit. */
   if (size == 0) {
      PRINTF_FATAL("PULLUP", "The size of the vector is zero!");
   }

   /* We get the value of the first vector. */
   resolved_val = m_val_vec[0].value();

   /* If the size of the vector was one, we are done. */
   if (size > 1) m_new_val = m_val_vec[0];

   /* If not, we then look at the rest of the vector. */
   else {
      resolved_val = m_val_vec[0].value();
      /* We scan all bits. If we find X we quit as the resolution can no
       * longer change. */
      for(bit = 1; bit < size && resolved_val != SC_LOGIC_X; bit = bit + 1) {
         /* Each time we accumulate the resolution from the table. */
         resolved_val =
         sc_logic_resolution_tbl[resolved_val.value()][m_val_vec[bit].value()];
      }
      m_new_val = resolved_val;
   }

   /* And we check the result. If the resolution was Z, then we report it was
    * a logic 1. */
   if (m_new_val == SC_LOGIC_Z) {
      m_new_val = SC_LOGIC_1;
   }
   base_type::update();
}

template <int W> ea_pullup_pack<W>::ea_pullup_pack() {}

template <int W> inline void ea_pullup_pack<W>::update() {
   sc_lv_resolve<W>::resolve(this->m_new_val, this->m_val_vec);
   for (unsigned i=0;i!=W;i++) {
      if (this->m_new_val[i] == SC_LOGIC_Z) {
         this->m_new_val[i]=SC_LOGIC_1;
      }
   }
   sc_signal_rv<W>::base_type::update();
}
