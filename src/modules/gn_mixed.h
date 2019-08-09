/*******************************************************************************
 * gn_mixed.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#ifndef GN_MIXSIG_H
#define GN_MIXSIG_H

#include <systemc.h>

#define GN_LOGIC_0 gn_mixed::LOGIC_0
#define GN_LOGIC_1 gn_mixed::LOGIC_1
#define GN_LOGIC_W0 gn_mixed::LOGIC_W0
#define GN_LOGIC_W1 gn_mixed::LOGIC_W1
#define GN_LOGIC_Z gn_mixed::LOGIC_Z
#define GN_LOGIC_A gn_mixed::LOGIC_A
#define GN_LOGIC_X gn_mixed::LOGIC_X

struct gn_mixed {
   typedef enum {LOGIC_0, LOGIC_1, LOGIC_W0, LOGIC_W1, LOGIC_Z, LOGIC_A,
      LOGIC_X} gn_logic_t;
   typedef enum {GN_TYPE_STRONG, GN_TYPE_WEAK, GN_TYPE_Z, GN_TYPE_ANALOG}
      gn_param_t;
   sc_logic logic;
   gn_param_t param;
   float lvl;

   /* These are globals that set the analog levels to be used for the different
    * digital levels. There is no resolution or feedback calculated in them.
    * It is just a convention to make it more easily vieable in a digital
    * waveform viewer. vdd_lvl is the level for logic 1, vss_lvl is for logic 0
    * and undef_lvl is for Z or X.
    */
   static float vdd_lvl;
   static float vss_lvl;
   static float undef_lvl;

   gn_mixed();
   gn_mixed(const gn_mixed &n_);
   gn_mixed(const gn_logic_t n_);
   explicit gn_mixed(const sc_logic &n_);
   explicit gn_mixed(const float &n_);
   explicit gn_mixed(const char c_);
   explicit gn_mixed(const bool b_);

   gn_mixed &operator=(const gn_mixed &n_);
   gn_mixed &operator=(const gn_logic_t n_);
   gn_mixed &operator=(const sc_logic &n_);
   gn_mixed &operator=(const float &n_);
   gn_mixed &operator=(const char c_);
   gn_mixed &operator=(const bool b_);

   char to_char() const;
   gn_logic_t value();

   void print(std::ostream &os = std::cout) const {
      if (logic != LOGIC_A) os << to_char();
      else os << lvl;
   }

   private:
   float guess_lvl(sc_logic &l);
   gn_param_t guess_param(gn_logic_t g);
};
bool operator==(const gn_mixed &a, const gn_mixed &b);
bool operator==(const gn_mixed &a, const sc_logic &b);
inline bool operator!=(const gn_mixed &a, const gn_mixed &b) {
   return !operator==(a, b);
}
inline bool operator!=(const gn_mixed &a, const sc_logic &b) {
   return !operator==(a, b);
}
inline std::ostream &operator<<(std::ostream &os, const gn_mixed &a) {
   a.print(os);
   return os;
}
class gn_signal_mix : public sc_signal<gn_mixed,SC_MANY_WRITERS> {
public:

    // typedefs

    typedef gn_signal_mix                       this_type;
    typedef sc_signal<gn_mixed,SC_MANY_WRITERS> base_type;
    typedef gn_mixed                            value_type;

public:

    // constructors

    gn_signal_mix()
      : base_type( sc_gen_unique_name( "signal_mix" ) ), m_proc_vec(),
         m_val_vec()
    {}

    explicit gn_signal_mix( const char* name_ )
      : base_type( name_ ) , m_proc_vec(), m_val_vec() {}

    gn_signal_mix( const char* name_, const value_type& initial_value_ )
      : base_type( name_, initial_value_ ) , m_proc_vec() , m_val_vec() {}

    // interface methods

    virtual void register_port( sc_port_base&, const char* ) {}

    /* write the new value, we take the mixed value or either a digital or
     * analog value.
     */
    virtual void write(const value_type& value_);
    virtual void write(const gn_mixed::gn_logic_t value_);
    virtual void write(const sc_logic& value_);
    virtual void write(const float& value_);
    virtual void write(const char value_);
    virtual void write(const bool value_);

    /* For reading, we have the inherited read() plus these two. */
    sc_logic read_logic( );
    float read_lvl( );

    // other methods
    virtual const char* kind() const { return "gn_signal_mix"; }

    // assignment
    this_type& operator = ( const value_type& a )
      { base_type::operator=(a); return *this; }

    this_type& operator = ( const sc_signal_resolved& a )
      { base_type::operator=(gn_mixed(a.read())); return *this; }

    this_type& operator = ( const sc_signal_in_if<value_type>& a )
      { base_type::operator=(a.read()); return *this; }

    this_type& operator = ( const sc_signal_in_if<sc_logic>& a )
      { base_type::operator=(gn_mixed(a.read())); return *this; }

    this_type& operator = ( const sc_signal_in_if<float>& a )
      { base_type::operator=(gn_mixed(a.read())); return *this; }

    this_type& operator = ( const this_type& a )
      { base_type::operator=(a); return *this; }

protected:
    virtual void update();

protected:
    std::vector<sc_process_b*> m_proc_vec; // processes writing this signal
    std::vector<value_type>    m_val_vec;  // new values written this signal

private:

    // disabled
    gn_signal_mix( const this_type& );
};

inline void sc_trace(sc_trace_file *tf, const gn_mixed &object,
      const std::string &name) {
   sc_trace(tf, object.logic, name + "_d");
   sc_trace(tf, object.lvl, name + "_a");
}

#endif
