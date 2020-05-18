/*******************************************************************************
 * gn_mixed.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a derivative of the sc_signal_resolved class from SystemC
 * which was extended to support a third, analog level, data state. It should
 * be noted that this is not a real AMS style signal, for that we need
 * SystemC-AMS. This is just a shortcut to make it possible to express
 * a signal which can be either analog or 4-state without having to
 * use a AMS style simulator.
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
 *
 * This was based off the work licensed as:
 *
 *    Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
 *    more contributor license agreements.  See the NOTICE file distributed
 *    with this work for additional information regarding copyright ownership.
 *    Accellera licenses this file to you under the Apache License, Version 2.0
 *    (the "License"); you may not use this file except in compliance with the
 *    License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied.  See the License for the specific language governing
 *    permissions and limitations under the License.
 *
 *    Original Author: Martin Janssen, Synopsys, Inc., 2001-05-21
 *******************************************************************************
 */

#include <systemc.h>
#include "gn_mixed.h"

/* The table for resolving GN_LOGIC.
 * Some notes:
 *    -weak always looses from strong, 
 *    -there is no weak undefined as we do not know if a weak undefined would
 *        loose.
 *    -Analog only wins from Z as a short between analog requires us to know
 *        the currents.
 *    -Conflicts with Undefined always produce an Undefined.
 */
gn_mixed::gn_logic_t
   gn_logic_resolution_tbl[7][7] =
            /* 0 */    /* 1 */    /* W0 */   /* W1 */   /* Z */     /* A */     /* X */
/* 0 */{{GN_LOGIC_0,GN_LOGIC_X,GN_LOGIC_0 ,GN_LOGIC_0 ,GN_LOGIC_0 ,GN_LOGIC_X,GN_LOGIC_X},
/* 1 */ {GN_LOGIC_X,GN_LOGIC_1,GN_LOGIC_1 ,GN_LOGIC_1 ,GN_LOGIC_1 ,GN_LOGIC_X,GN_LOGIC_X},
/*W0 */ {GN_LOGIC_0,GN_LOGIC_1,GN_LOGIC_W0,GN_LOGIC_X ,GN_LOGIC_W0,GN_LOGIC_X,GN_LOGIC_X},
/*W1 */ {GN_LOGIC_0,GN_LOGIC_1,GN_LOGIC_X ,GN_LOGIC_W1,GN_LOGIC_W1,GN_LOGIC_X,GN_LOGIC_X},
/* Z */ {GN_LOGIC_0,GN_LOGIC_1,GN_LOGIC_W0,GN_LOGIC_W1,GN_LOGIC_Z ,GN_LOGIC_A,GN_LOGIC_X},
/* A */ {GN_LOGIC_X,GN_LOGIC_X,GN_LOGIC_X ,GN_LOGIC_X ,GN_LOGIC_A ,GN_LOGIC_X,GN_LOGIC_X},
/* X */ {GN_LOGIC_X,GN_LOGIC_X,GN_LOGIC_X ,GN_LOGIC_X ,GN_LOGIC_X ,GN_LOGIC_X,GN_LOGIC_X}};

/* These are globals that set the analog levels to be used for the different
 * digital levels. There is no resolution or feedback calculated in them.
 * It is just a convention to make it more easily vieable in a digital
 * waveform viewer. vdd_lvl is the level for logic 1, vss_lvl is for logic 0
 * and undef_lvl is for Z or X.
 */
float gn_mixed::vdd_lvl = 3.3f;
float gn_mixed::vss_lvl = 0.0f;
float gn_mixed::undef_lvl = 1.5f;

sc_logic gn_to_sc_logic(gn_mixed::gn_logic_t g) {
   switch(g) {
      case GN_LOGIC_0:
      case GN_LOGIC_W0: return SC_LOGIC_0;
      case GN_LOGIC_1:
      case GN_LOGIC_W1: return SC_LOGIC_1;
      case GN_LOGIC_Z: return SC_LOGIC_Z;
      default: return SC_LOGIC_X;
   }
}
float gn_mixed::guess_lvl(sc_logic &l) {
   if (l == SC_LOGIC_0) return vss_lvl;
   else if (l == SC_LOGIC_1) return vdd_lvl;
   else return undef_lvl;
}
gn_mixed::gn_param_t gn_mixed::guess_param(gn_logic_t g) {
   switch(g) {
      case GN_LOGIC_W0: return GN_TYPE_WEAK;
      case GN_LOGIC_W1: return GN_TYPE_WEAK;
      case GN_LOGIC_A: return GN_TYPE_ANALOG;
      case GN_LOGIC_Z: return GN_TYPE_Z;
      default: return GN_TYPE_STRONG;
   }
}
gn_mixed::gn_logic_t char_to_gn(const char c_) {
   switch(c_) {
      case '0': return GN_LOGIC_0;
      case '1': return GN_LOGIC_1;
      case 'l': return GN_LOGIC_W0;
      case 'h': return GN_LOGIC_W1;
      case 'Z': return GN_LOGIC_Z;
      case 'A': return GN_LOGIC_A;
      default: return GN_LOGIC_X;
   }
}
gn_mixed::gn_mixed() {
   logic = SC_LOGIC_X;
   lvl = undef_lvl;
   param = GN_TYPE_STRONG;
}
gn_mixed::gn_mixed(const gn_mixed &n) {
   logic = n.logic;
   lvl = n.lvl;
   param = n.param;
}
gn_mixed::gn_mixed(const gn_logic_t n_) {
   logic = gn_to_sc_logic(n_);
   lvl = guess_lvl(logic);
   param = guess_param(n_);
}
gn_mixed::gn_mixed(const sc_logic &n) {
   logic = n;
   lvl = guess_lvl(logic);
   param = (n == SC_LOGIC_Z)?GN_TYPE_Z:GN_TYPE_STRONG;
}
gn_mixed::gn_mixed(const float &n) {
   logic = SC_LOGIC_X;
   lvl = n;
   param = GN_TYPE_ANALOG;
}
gn_mixed::gn_mixed(const char c_) {
   gn_logic_t g = char_to_gn(c_);
   logic = gn_to_sc_logic(g);
   lvl = guess_lvl(logic);
   param = guess_param(g);
}
gn_mixed::gn_mixed(const bool b_) {
   logic = sc_logic(b_);
   lvl = guess_lvl(logic);
   param = GN_TYPE_STRONG;
}
gn_mixed &gn_mixed::operator=(const gn_mixed &n) {
   logic = n.logic;
   lvl = n.lvl;
   param = n.param;
   return *this;
}
gn_mixed &gn_mixed::operator=(const gn_logic_t n_) {
   logic = gn_to_sc_logic(n_);
   lvl = guess_lvl(logic);
   param = guess_param(n_);
   return *this;
}
gn_mixed &gn_mixed::operator=(const sc_logic &n) {
   logic = n;
   lvl = guess_lvl(logic);
   param = (n == SC_LOGIC_Z)?GN_TYPE_Z:GN_TYPE_STRONG;
   return *this;
}
gn_mixed &gn_mixed::operator=(const float &n) {
   logic = LOGIC_A;
   lvl = n;
   param = GN_TYPE_ANALOG;
   return *this;
}
gn_mixed &gn_mixed::operator=(const char c_) {
   gn_logic_t g = char_to_gn(c_);
   logic = gn_to_sc_logic(g);
   lvl = guess_lvl(logic);
   param = guess_param(g);
   return *this;
}
gn_mixed &gn_mixed::operator=(const bool b_) {
   logic = sc_logic(b_);
   lvl = guess_lvl(logic);
   param = GN_TYPE_STRONG;
   return *this;
}
/* When comparing GN_MIXED, we compare all fields. When comparing GN_MIXED to
 * SC_LOGIC we have to treat the weak and strong signals as the same. We also
 * have to treat analog signals as simply undefined.
 */
bool operator==(const gn_mixed &a, const gn_mixed &b) {
   if (a.logic != b.logic) return false;
   if (a.param != b.param) return false;
   if (a.param != gn_mixed::GN_TYPE_ANALOG) return true;
   return (a.lvl > b.lvl - 0.01 && a.lvl < b.lvl + 0.01);
}
bool operator==(const gn_mixed &a, const sc_logic &b) {
   return a.logic == b;
}
char gn_mixed::to_char() const {
   if (param == GN_TYPE_ANALOG) return 'A';
   else if (logic == SC_LOGIC_X) return 'X';
   else if (logic == SC_LOGIC_Z) return 'Z';
   else if (logic == SC_LOGIC_0) return (param == GN_TYPE_WEAK)?'l':'0';
   else if (logic == SC_LOGIC_1) return (param == GN_TYPE_WEAK)?'h':'1';
   else return GN_LOGIC_X;
}
gn_mixed::gn_logic_t gn_mixed::value() {
   if (param == GN_TYPE_ANALOG) return GN_LOGIC_A;
   else if (logic == SC_LOGIC_X) return GN_LOGIC_X;
   else if (logic == SC_LOGIC_Z) return GN_LOGIC_Z;
   else if (logic == SC_LOGIC_0)
      return (param == GN_TYPE_WEAK)?GN_LOGIC_W0:GN_LOGIC_0;
   else if (logic == SC_LOGIC_1)
      return (param == GN_TYPE_WEAK)?GN_LOGIC_W1:GN_LOGIC_1;
   else return GN_LOGIC_X;
}

static void gn_mixed_resolve(gn_mixed& result_,
      const std::vector<gn_mixed>& values_ )
{
   int sz = values_.size();
   sc_assert( sz != 0 );

   if( sz == 1 ) {
      result_ = values_[0];
      return;
   }

   gn_mixed current;
   gn_mixed res = values_[0];
   int i;
   /* We now scan the list of driven values so we can resolve it. If we find
    * a X it all goes X, so we can stop looking.
    */
   for(i = sz - 1; i>0 && res.logic != SC_LOGIC_X; --i) {
      /* If the resolved is high-Z, we resolve to the next one. */
      if (res.value() == GN_LOGIC_Z) {
         res = values_[i];
         continue;
      }

      /* In other cases we use the resolution table. */
      current = values_[i];

      /* This case is simple. */
      if (current.value() == GN_LOGIC_Z) continue;

      /* We let the resolution table handle the rest. */
      res = gn_logic_resolution_tbl[res.value()][current.value()];
   }

   result_ = res;
}

void gn_signal_mix::write(const value_type& value_) {
   sc_process_b* cur_proc = sc_get_current_process_b();

   bool value_changed = false;
   bool found = false;
    
   for( int i = m_proc_vec.size() - 1; i >= 0; -- i ) {
      if( cur_proc == m_proc_vec[i] ) {
	 if( value_ != m_val_vec[i] ) {
            m_val_vec[i] = value_;
            value_changed = true;
	 }
	 found = true;
	 break;
      }
   }
    
    if( ! found ) {
	m_proc_vec.push_back( cur_proc );
	m_val_vec.push_back( value_ );
	value_changed = true;
    }
    
    if( value_changed ) {
	request_update();
    }
}

void gn_signal_mix::write( const gn_mixed::gn_logic_t value_ )
   { write(gn_mixed(value_));}
void gn_signal_mix::write( const sc_logic& value_ ) { write(gn_mixed(value_)); }
void gn_signal_mix::write( const float& value_ ) { write(gn_mixed(value_)); }
void gn_signal_mix::write( const char value_ ) { write(gn_mixed(value_)); }
void gn_signal_mix::write( const bool value_ ) { write(gn_mixed(value_)); }

sc_logic gn_signal_mix::read_logic( ) {
   return read().logic;
}
float gn_signal_mix::read_lvl( ) {
   return read().lvl;
}

void gn_signal_mix::update()
{
   gn_mixed_resolve( m_new_val, m_val_vec );
   base_type::update();
}

void gn_tie_mix::update()
{
   base_type::update();
}
