#include <systemc.h>

class gn_semaphore : public sc_semaphore {
   public:
   gn_semaphore(int iv): sc_semaphore(iv) {}
   gn_semaphore(const char *n, int iv): sc_semaphore(n, iv) { }
   virtual int wait() {
      return sc_semaphore::wait();
   }
   virtual int wait(const sc_time &timeout) {
      while( in_use() ) {
	sc_core::wait( timeout , m_free );
      }
      if (m_free.triggered()) {
         -- m_value;
         return 0;
      }
      else return -1;
   }
   virtual const char* kind() const
      { return "gn_semaphore"; }
};
