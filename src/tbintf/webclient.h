/*******************************************************************************
 * webclient.h -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is a model for a net client. It can behave as a web client or server,
 *   a NTP server or a MQTT Broker. This file does not implement all the
 *   details of these protocols, it simply provides a way for a testbench to
 *   generate stimulus to emulate these protocols.
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

#ifndef _WEBCLIENT_H
#define _WEBCLIENT_H
#include <systemc.h>
#include "hfieldlist.h"
#include "WiFi.h"
#include "cchan.h"
/* This should never have happened, but it did. The TFT_eSPI header defines
 * min and max macros and deque goes crazy with them. The ideal would be to
 * fix it inside TFT_eSPI, but for now this will work.
 */
#undef min
#undef max
#include <deque>

typedef enum {HTTP_GET, HTTP_POST} httpenum_t;
typedef enum {
   MQTT_TYPE_CONNECT = 1,
   MQTT_TYPE_CONNACK = 2,
   MQTT_TYPE_PUBLISH = 3,
   MQTT_TYPE_PUBACK = 4,
   MQTT_TYPE_PUBREC = 5,
   MQTT_TYPE_PUBREL = 6,
   MQTT_TYPE_PUBCOMP = 7,
   MQTT_TYPE_SUBSCRIBE = 8,
   MQTT_TYPE_SUBACK = 9
} mqtt_type_t;
class handle {
    public:
       handle(String name, void (*_f)(void));
       handle(String name);
       handle();
       String path;
       void (*f)(void);
};

struct wifiport_t {
   int port;
   bool closed;
   std::deque<unsigned char> buffer;
   bool connectionless;
   wifiport_t() {
      port = 0;
      closed = false;
      connectionless = false;
   }
   wifiport_t(int _p) {
      port = _p;
      closed = false;
      connectionless = false;
   }
   wifiport_t(int _p, bool _connectionless) {
      port = _p;
      closed = false;
      connectionless = _connectionless;
   }

};

SC_MODULE(webclient) {
   sc_in<unsigned int> rx {"rx"};
   sc_out<unsigned int> tx {"tx"};
   cchan i_uwifi{"i_uwifi", 256, 256};

   /* Arg management, for answering to forms. */
   void deleteArgs();
   void regArg(const char *name, const char *value);
   bool setArg(const char *name, const char *value);
   bool hasArg(const char *name);
   String arg(const char *name);
   int args();
   String argName(int i);
   String arg(int i);

   /* Server stuff */
   void expectconnect(IPAddress ip, unsigned int port);
   void expectline(int port, std::string page);
   void expecttillregex(int port, const char *patt);
   void expecttillline(int port, std::string line);
   void expectupgrade(int port);
   void openudpport(IPAddress ip, int port);
   std::string packetname(mqtt_type_t type, const char *str);
   std::string packetname(mqtt_type_t type, const unsigned char *str);
   int receivemqtt(int port, unsigned char *&topic, mqtt_type_t &packettype,
      int &length, int &flags, std::string &pub);
   void sendmqtt(int port, mqtt_type_t packettype, int packetid = 0,
      int c0 = 0);
   void publishmqtt(int port, const char *topic, int packetid,
      void *payload, int payloadlen);
   int expectmqtt(int port, mqtt_type_t exptype);
   int autoanswermqttpub(int port, mqtt_type_t &packettype);
   void expectws(int port);
   void printpage(int port);
   bool pending(int port);
   bool isclosed(int port);
   bool willclose(int port);
   void respondntp(int port, time_t trec, time_t tsend);

   /* Tasks */
   void fillbuffers();

   /* Activators */
   void connectclient(IPAddress toip, unsigned int toport);
   void requestpage(int port, std::string path, const char *auth = NULL);
   void expectauthenticate(int port);
   void answerpage(int port, std::string path, const char *auth = NULL);

   SC_CTOR(webclient) {
      i_uwifi.tx(tx);
      i_uwifi.rx(rx);

      _portclosed = false;

      SC_THREAD(fillbuffers);
   }

   private:
   /* Channel send */
   void send(int port, void *msg, int len, bool escape = false);
   void send(int port, const char *msg, bool escape = false);
   void sendaddress(int port, std::string msg, bool escape = false);
   int sendf(int port, bool escape, const char *fmt, ...);
   int sendifauth(int port, const char *auth, bool escape = false);
   void flush(int port);
   int read(int port);
   unsigned char readchar(int port);
   std::string readln(int port);
   /* Server pages */
   std::vector<handle> _opt;
   /* Args for forms */
   hfieldlist _arg;

   private:
   bool _portclosed;
   int read(int port, int ind);

   /* Port list and methods */
   std::vector<wifiport_t> _portlist;
   int getind(int port);
   int getnotclosed(int port);
};

#endif
