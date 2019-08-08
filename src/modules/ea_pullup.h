/*******************************************************************************
 * Description:
 *   This is a derived class from sc_signal_resolved which resolves to 1
 *   instead of Z.
 *
 *   It was based off the example ea_signal_pullup in figure 9-14 of
 *   David C. Black and Jack Donovan (2004) "SystemC: From the Ground Up"
 *      Boston: Kluwer Academic Publishers
 *
 *   This is intended mainly as a way to simplify more complex communication
 *   protocols and does not represent any existing protocol.
 */
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~
// DESCRIPTION
// This derived channel, ea_pullup, behaves like an sc_signal_resolved
// with an implicit weak pull-up resistor. If all sources write
// sc_logic_Z to it, then the default sc_logic_1 will result.
// Otherwise works like a sc_signal_resolved. The derived channel
// ea_pullup_pack<W> is used when you need something more than a single
// wire.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~

#ifndef EA_PULLUP_H
#define EA_PULLUP_H
#include <systemc.h>

class ea_pullup: public sc_signal_resolved {
   public:
   ea_pullup();
   explicit ea_pullup(const char* name_ ) :sc_signal_resolved(name_) {}
   // the fix
   virtual void update();
};

template <int W>
class ea_pullup_pack: public sc_signal_rv<W> {
   public:
      // constructors
      ea_pullup_pack();
      explicit ea_pullup_pack(const char* name_ ) :sc_signal_rv<W>(name_) {}
   protected:
      // the fix
      virtual void update();
};

#endif
