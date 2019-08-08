#include "uartclient.h"
#include "info.h"

void uartclient::send(const char *string) {
   while(*string != '\0') i_uart.to.write(*string++);
}

std::string uartclient::get() {
   std::string msg("");
   char recv;

   do {
      recv = i_uart.from.read();
      if (recv != '\r' && recv != '\n') msg += recv;
   } while(recv != '\n');

   return msg;
}

void uartclient::expect(const char *string) {
   std::string msg;
   msg = get();
   PRINTF_INFO("UARTCLI", "received: \"%s\"", msg.c_str());
   if (0 != msg.find(string)) {
      PRINTF_ERROR("UARTCLI", "Did not get expected \"%s\"", string);
   }
}

void uartclient::dump() {
   std::string msg("");
   char recv;
   while(1) {
      recv = i_uart.from.read();
      if (recv == '\r') continue;
      else if (recv == '\n') {
         SC_REPORT_INFO("SERIAL", msg.c_str());
         msg = "";
         continue;
      }
      else msg = msg + recv;
   }
}
