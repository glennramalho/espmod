/*******************************************************************************
 * webclient.cpp -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
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

#include <systemc.h>
#include <stdarg.h>
#include "webclient.h"
#include "libb64/cencode.h"
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"
#include "info.h"
#include <sys/types.h>
#include <regex.h>

handle::handle(String name, void (*_f)(void)): path(name) { f = _f; }
handle::handle(String name): path(name) { f = NULL; }
handle::handle(): path("") { f = NULL; }

const char *macstr = "55:32:77:aa:bb:38";

bool webclient::isclosed(int port) {
   if (getind(port) < 0) return true;
   else return false;
}
bool webclient::willclose(int port) {
   int ind = getind(port);
   if (ind < 0) return true;
   else if (_portlist[ind].closed) return true;
   else return false;
}
sc_event fifowrite_ev;

void webclient::send(int port, void *msg, int len, bool escape) {
   const unsigned char *c = (const unsigned char *)msg;
   static int lastport = -1;
   int pos;

   /* If a non-control port was requested, we then have to check if we are
    * already dealing with that port. If we are we do nothing. If we are dealing
    * with a new port, we need to send the port escape sequence.
    */
   if (port >= 0 && lastport != port) {
      char portmsg[10];
      char *ptr = portmsg;
      snprintf(portmsg, 10, "\xff%d:", port);
      while(*ptr != '\0') {
         i_uwifi.to.write((int)(*ptr));
         ptr = ptr + 1;
      }
      lastport = port;
   }
   /* If we are dealing with a control port we do not send the escape, but we
    * do need to set the lastport to something odd so that the next port send
    * will get the port escape.
    */
   else if (port < 0) {
      lastport = -1;
   }

   printf("Sending %d bytes to port %d:", len, port);
   for(pos = 0; pos < len; pos = pos + 1) {
      if (c[pos] == '\n' || c[pos] == '\r') ;
      else if (isprint(c[pos])) putc(c[pos], stdout);
      else printf("<%02x>", c[pos]);
   }
   printf("\n");
   for(pos = 0; pos < len; pos = pos + 1) {
      /* If we see a 0xff and we are escaping the message we need to send two
       * 0xff.
       */
      i_uwifi.to.write((int)(c[pos]));
      if (c[pos] == 0xff && escape) i_uwifi.to.write((int)(c[pos]));
   //   wait(1, SC_US);
   }
}

void webclient::send(int port, const char *msg, bool escape) {
   send(port, (void *)msg, strlen(msg), escape);
}

void webclient::sendaddress(int port, std::string msg, bool escape) {
   std::string patched("");
   const char *c = msg.c_str();

   /* First we need to replace any spaces with a %20. This is something
    * the browser usually does. We reserve space in the patched string, we
    * guess the width will be the same, and then expand it as needed.
    */
   patched.reserve(msg.length());
   while(*c != '\0') {
      if (*c == ' ') patched = patched + "%20";
      else patched = patched + *c;
      c = c + 1;
   }
   
   /* Now we send it to the port. */
   send(port, (void *)patched.c_str(), (int)patched.length(), escape);
}

int webclient::sendf(int port, bool escape, const char *fmt, ...) {
   char c[128];
   int resp;
   va_list ap;
   va_start(ap, fmt);
   resp = vsnprintf(c, 128, fmt, ap);
   va_end(ap);
   send(port, c, escape);
   return resp;
}

int webclient::sendifauth(int port, const char *auth, bool escape) {
   char *key;
   const int len = 5+7+1;
   char passphrase[len+1];
   int passphraselen;

   /* If auth is NULL, we do nothing. No authentication was requested. */
   if (auth == NULL) return 0;
   /* If it was requested, then we need to generate a response. */
   if (NULL == (key = new char[base64_encode_expected_len(len)+1])) {
      PRINTF_FATAL("WEBCLI", "Could not allocate encoded space");
      return -1;
   }
   passphraselen = snprintf(passphrase, len+1, "admin:%s", auth);
   if (base64_encode_chars(passphrase, passphraselen, key) <= 0) {
      PRINTF_FATAL("WEBCLI", "Could not encode passphrase");
      return -1;
   }
   return sendf(port, escape, "Authorization: Basic %s\r\n", key);
}

void webclient::flush(int port) {
   /* We need to find the index. The port might already be closed. If it is
    * we just quit.
    */
   int ind = getind(port);
   if (ind < 0) return;

   /* If it is not closed, we read as long as there is data in the buffer. */
   _portlist[ind].buffer.clear();

   /* When it runs out of data, if it was tagged to close, we then go ahead and
    * close it.
    */
   if (_portlist[ind].closed) {
      _portlist.erase(_portlist.begin()+ind);
      _portclosed = true;
   }
}

int webclient::read(int port) {
   return read(port, getind(port));
}

unsigned char webclient::readchar(int port) {
   int res = read(port);
   if (res < 0) {
      PRINTF_ERROR("WEBCLI", "Did not get data as expected");
   }
   return (unsigned char)res;
}

int webclient::read(int port, int ind) {
   int resp;
   unsigned char recv;
   /* This should not be called on an already closed port. */
   if (ind < 0) {
      PRINTF_ERROR("WEBCLI", "Reading from closed port %d", port);
      return -1;
   }

   /* We keep waiting until we get a char or we get a closed message. */
   while (_portlist[ind].buffer.size() == 0 && !_portlist[ind].closed) {
      wait(fifowrite_ev);
   }
   
   /* If we have no data, we then return -1. This means we were waiting and
    * got a close. We do not check the closed flag as we could have both.
    * As long as there is data, we have stuff to read.
    */
   if (_portlist[ind].buffer.size() == 0) { resp = -1; }
   /* We have data, so we take it. */
   else {
      recv = _portlist[ind].buffer.front();
      _portlist[ind].buffer.pop_front();
      resp = (int)recv;
   }

   /* Once a closed port goes empty we can delete it. */
   if (_portlist[ind].buffer.size() == 0 && _portlist[ind].closed) {
      _portlist.erase(_portlist.begin()+ind);
      _portclosed = true;
   }
   
   return resp;
}

std::string webclient::readln(int port) {
   int ret;
   char recv;
   std::string msg;
   bool crseen = false;
   int ind = getind(port);

   while(1) {
      ret = read(port, ind);

      /* If ret is negative, it got closed, so we quit here. We might have
       * a blank screen or perhaps nothing at all.
       */
      if (ret < 0) return msg;

      /* We got something so we analyze it. */
      recv = (int)ret;

      if (recv == '\n' && crseen) break;
      /* If we see a \r followed by something else, we put the \r in the msg. */
      else if (crseen) {
         msg = msg + '\r';
         crseen = false;
      }
      /* Now we look at the char that came in. If it is a \r,
       * crseen goes true. If not, we then just push it onto the list. */
      if (recv == '\r') crseen = true;
      else msg = msg + recv;
   } while(recv != '\n');

   return msg;
}

void webclient::expectconnect(IPAddress ip, unsigned int port) {
   std::string msg;
   char gotipstr[40];
   unsigned int gotport;
   IPAddress gotip;
   flush(port);
   msg = readln(-1);
   printf("To WiFi: %s\n", msg.c_str());
   if (msg.find("\xff""c ") != 0) {
      PRINTF_ERROR("WEBCLI", "Did not get expected connect request");
   }
   if (2 != sscanf(msg.c_str(), "\xff""c %39[^:]:%u", gotipstr, &gotport)) {
      PRINTF_ERROR("WEBCLI", "Did not get expected connect request pattern");
   }
   if (!gotip.fromString(gotipstr)) {
      PRINTF_ERROR("WEBCLI", "Did not get valid IP Address");
   }
   gotip.fromString(gotipstr);
   if (gotip != ip || gotport != port) {
      PRINTF_ERROR("WEBCLI", "Did not get the expected IP Address and port");
      sendf(-1, false, "\xff%dn\r\n", port);
   }
   else {
      /* We got a valid connect request. */
      SC_REPORT_INFO("WEBCLI", "Got connect request");
      
      /* We then allocate it and tag it as connected. */
      _portlist.push_back(wifiport_t(port));

      /* Now that the port and associated buffer has been created, we can
       * tell the other side that we are good to go. So we send a "y"
       * message with the MAC ID.
       */
      sendf(-1, false, "\xff%dy %s\r\n", port, macstr);
   }
}
void webclient::expectline(int port, std::string page) {
   std::string msg;
   msg = readln(port);
   printf("To WiFi: %s\n", msg.c_str());
   if (msg.find(page.c_str()) != 0) {
      PRINTF_ERROR("WEBCLI", "Did not get expected request");
   }
}
void webclient::expecttillregex(int port, const char *patt) {
   regex_t r;
   std::string msg;
   /* First we compile the pattern. */
   if (0 != regcomp(&r, patt, REG_NOSUB)) {
      PRINTF_FATAL("WEBCLI", "Regex pattern %s failed to compile", patt);
      return;
   }

   /* Assuming it worked, we can try to match the lines and we go until we hit
    * a pattern.
    */
   do {
      msg = readln(port);
      printf("To WiFi: %s\n", msg.c_str());
   } while(REG_NOMATCH == regexec(&r, msg.c_str(), (size_t)0, NULL, 0));
}

void webclient::expecttillline(int port, std::string line) {
   std::string msg;
   do {
      msg = readln(port);
      printf("To WiFi: %s\n", msg.c_str());
   } while (msg.find(line.c_str()) != 0);
}

bool findend(std::string msg, const char *cnst) {
   std::string patt = cnst;
   auto srch = std::search(msg.begin(), msg.end(), patt.begin(), patt.end(),
      [](char c1, char c2) { return std::toupper(c1) == std::toupper(c2); });
   return srch != msg.end();
}
void webclient::expectauthenticate(int port) {
   /* We are expecting the server to request authentication. So we make sure
    * it shows up. This format can change, but for our webserver we know what
    * it is supposed to be.
    */
   expectline(port, "HTTP/1.1 401 Unauthorized");
   expectline(port, "Content-Type: text/html");
   expectline(port, "WWW-Authenticate: Basic realm=\"Login Required\"");
   expectline(port, "Content-Length: 0");
   expectline(port, "Connection: close");
   expectline(port, "");

   /* And we dump any remaining chars in the stream. */
   wait(10, SC_MS);
   flush(port);
}
void webclient::openudpport(IPAddress ip, int port) {
   /* Ports need to be open before expecting data. Usually this is done via the
    * connect command, but UDP ports do not have a connect. Therefore, we need
    * an open command.
    */
   PRINTF_INFO("TEST", "Opening UDP port %d", port);
   _portlist.push_back(wifiport_t(port, true));
}

void webclient::printpage(int port) {
   std::string msg;
   do {
      msg = readln(port);
      printf("To WiFi: %s\n", msg.c_str());
      /* We keep on going until we get an EOT. */
   } while (!isclosed(port));

   /* And we dump any remaining chars in the stream. */
   wait(10, SC_MS);
}

void webclient::deleteArgs() { _arg.deleteArgs(); }
void webclient::regArg(const char *name, const char *value) {
   _arg.regArg(name, value);
}
bool webclient::setArg(const char *name, const char *value) {
   return _arg.setArg(name, value);
}
bool webclient::hasArg(const char *name) { return _arg.hasArg(name); }
String webclient::arg(const char *name) {
   return String(_arg.arg(name).c_str());
}
int webclient::args() { return _arg.args(); }
String webclient::argName(int i) {
   return String(_arg.argName(i).c_str());
}
String webclient::arg(int i) {
   return String(_arg.arg(i).c_str());
}

void webclient::connectclient(IPAddress toip, unsigned int toport) {
   std::string msg;
   unsigned int a[6];

   /* Get rid of any junk. */
   flush(-1);
   /* Send a connect request. */
   sendf(-1, false, "\xff""c %s:%u\r\n", toip.toString().c_str(), toport);
   /* We need to create the port or we will not get any answer. */
   _portlist.push_back(wifiport_t(toport));
   /* And we wait for the resposne. */
   msg = readln(toport);
   printf("To WiFi: %s\n", msg.c_str());
   if (msg.find("\xff""y") != 0) {
      PRINTF_ERROR("WEBCLI", "Expected Accept from the DUT");
      int ind = getind(toport);
      if (ind >= 0) {
         _portlist.erase(_portlist.begin()+ind);
         _portclosed = true;
      }
      return;
   }
   if (6 != sscanf(msg.c_str(), "\xff""y %x:%x:%x:%x:%x:%x",
      &a[0],&a[1],&a[2],&a[3],&a[4],&a[5])
         || a[0] > 0xff || a[1] > 0xff || a[2] > 0xff
         || a[3] > 0xff || a[4] > 0xff || a[5] > 0xff) {
      PRINTF_ERROR("WEBCLI", "Did not get a valid MAC back");
      int ind = getind(toport);
      if (ind < 0) {
         _portlist.erase(_portlist.begin()+ind);
         _portclosed = true;
      }
      return;
   }

   /* And we print a message and add the port to the connected list. */
   PRINTF_INFO("WEBCLI", "Connected to AP BSSID %02x:%02x:%02x:%02x:%02x:%02x",
      a[0], a[1], a[2], a[3], a[4], a[5]);
}

void webclient::requestpage(int port, std::string path, const char *auth) {
   /* Get rid of any junk. */
   flush(port);
   /* Send a connect request. */
   sendf(port, false, "GET %s HTTP/1.1\r\n", path.c_str());
   sendf(port, false, "Host: www.webclient.com.br\r\n", path.c_str());
   /* If authentication was requested, we send the string. */
   sendifauth(port, auth);
   /* And we need to send a blank line. This ends the header and begins the
    * content.
    */
   sendf(port, false, "\r\n", path.c_str());
}

void webclient::answerpage(int port, std::string path, const char *auth) {
   std::string msg;
   int handle;

   /* Get rid of any junk. */
   flush(port);

   /* We put together the POST request and send it. */
   send(port, "POST ", true);
   sendaddress(port, path.c_str(), true);
   sendaddress(port, "?", true);
   /* If authentication was requested, we send the string. */
   for(handle = 0; handle < _arg.args(); handle = handle + 1) {
      msg = _arg.argName(handle);
      msg = msg + "=";
      msg = msg + _arg.arg(handle);
      if (handle != _arg.args()-1) msg = msg + "&";
      sendaddress(port, msg, true);
   }
   send(port, " HTTP/1.1\r\n", true);
   send(port, "Host: www.webclient.com.br\r\n", true);
   sendifauth(port, auth, true);
   send(port, "\r\n", true);
}
void webclient::expectupgrade(int port) {
   std::string msg;
   bool found = false;
   char clientkey[32+36+1];
   unsigned char encodedresp[20];
   unsigned char respkey[32];
   size_t olen;
   const char *cpos;
   char *pos;

   clientkey[0] = '\0';
   do {
      msg = readln(port);
      if (msg.find("Upgrade: websocket") == 0) found = true;
      /* We need the key */
      if (msg.find("Sec-WebSocket-Key:") == 0) {
         cpos = strchr(msg.c_str(), ':');
         cpos = cpos + 1;
         while (*cpos == ' ') cpos = cpos + 1;
         strncpy(clientkey, cpos, 32);
         /* In case we passed the 32 chars we cap it. */
         clientkey[31] = '\0';
         /* Now we look for the end char and cap it again. */
         pos = strchr(clientkey, '\r');
         /* If we found it we cap it. If we don't we probably got a very
          * very long key. We then just keep it as it is and hope for the best.
          */
         if (pos != NULL) *pos = '\0';
      }
      printf("To WiFi: %s\n", msg.c_str());
      /* We keep on going until we get a blank line. */
   } while (msg != "");

   if (!found) {
      PRINTF_ERROR("WEBCLI", "Did not get expected websocket upgrade.");
      return;
   }

   if (clientkey[0] == '\0') {
      PRINTF_ERROR("WEBCLI", "Did not get the client key");
      return;
   }

   char buf[64];
   snprintf(buf, 64, "Received key: %s", clientkey);
   SC_REPORT_INFO("WEBCLI", buf);

   /* We now concatenate the web key with the magic string. */
   strcat(clientkey, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
   snprintf(buf, 64, "Concat: %s", clientkey);
   SC_REPORT_INFO("WEBCLI", buf);
   /* We can now re-encode it. */
   mbedtls_sha1((const unsigned char *)clientkey, strlen(clientkey),
      encodedresp);
   mbedtls_base64_encode(respkey, 32, &olen, encodedresp, 20);
   snprintf(buf, 64, "Resp: %s", respkey);
   SC_REPORT_INFO("WEBCLI", buf);
   /* And we send it back to the client. */
   send(port, "HTTP/1.1 101 Switching Protocols\r\n", true);
   send(port, "Upgrade: websocket\r\n", true);
   send(port, "Connection: Upgrade\r\n", true);
   sendf(port, true, "Sec-WebSocket-Accept: %s\r\n", respkey);
   send(port, "\r\n", true);
}

std::string webclient::packetname(mqtt_type_t type, const unsigned char *str) {
   return packetname(type, (const char *)str);
}

std::string webclient::packetname(mqtt_type_t type, const char *str) {
   switch (type) {
      case MQTT_TYPE_CONNECT:
         return std::string("MQTT Connect");
      case MQTT_TYPE_CONNACK:
         return std::string("MQTT CONNACK");
      case MQTT_TYPE_PUBLISH:
         return std::string("MQTT Publish: ") + std::string(str);
      case MQTT_TYPE_PUBACK:
         return std::string("MQTT PUBACK");
      case MQTT_TYPE_PUBREC:
         return std::string("MQTT PUBREC");
      case MQTT_TYPE_PUBREL:
         return std::string("MQTT PUBREL");
      case MQTT_TYPE_PUBCOMP:
         return std::string("MQTT PUBCOMP");
      case MQTT_TYPE_SUBSCRIBE:
         return std::string("MQTT Subscribe");
      default:
         return std::to_string(type);
   }
}
int webclient::expectmqtt(int port, mqtt_type_t exptype) {
   unsigned char recv;
   mqtt_type_t packettype;
   int flags;
   int length;
   int packetid;
   int cnt;
   unsigned char *topic = NULL;
   std::string pub("");

   /* We first read the fixed header. */
   recv = readchar(port);
   packettype = (mqtt_type_t)((recv & 0xf0)>>4);
   flags = recv & 0x0f;
   /* The length is a bit complicated. */
   recv = readchar(port); /* Byte 0 */
   length = recv & 0x7f;
   if (recv > 127) {
      recv = readchar(port); /* Byte 1 */
      length = length + (recv & 0x7f) * 128;
      if (recv > 127) {
         recv = readchar(port); /* Byte 2 */
         length = length + (recv & 0x7f) * 128 * 128;
         if (recv > 127) {
            recv = readchar(port); /* Byte 3 */
            length = length + (recv & 0x7f) * 128 * 128 * 128;
            if (recv > 127) {
               PRINTF_ERROR("WEBCLI", "MQTT packet larger than supported.");
            }
         }
      }
   }

   /* If we have enough space for a variable ID, we take it. */
   if (length > 1) {
      /* Depending on the command we might need to grab a few fields. */
      switch(packettype) {
         case MQTT_TYPE_PUBLISH: {
            /* First we get the length of the topic string; */
            int topiclength = (int)readchar(port) << 8;
            topiclength = topiclength | (int)readchar(port);
            topic = new unsigned char[topiclength+1];
            if (topic == NULL) PRINTF_FATAL("WEBCLI", "Out of memory");
            /* Now we get the topic string. */
            for(cnt = 2; cnt < topiclength+2; cnt = cnt + 1)
               topic[cnt-2] = (int)readchar(port);
            topic[cnt-2] = '\0';

            /* Now we get the packet ID. */
            packetid = (int)readchar(port) << 8;
            packetid = packetid | (int)readchar(port);
            cnt = cnt + 2;

            /* We collect the payload into a string to print */
            pub.reserve(256);
            bool done = false;
            char onech;
            for(; cnt < length; cnt = cnt + 1) {
               onech = (char)readchar(port);
               if (done || onech == '\0') done = true;
               else pub = pub + onech;
            }
            break;
         }
         /* Some packets have a packet ID. The rest we junk. */
         case MQTT_TYPE_PUBREL:
         case MQTT_TYPE_SUBSCRIBE: {
            if (length < 2)
               PRINTF_ERROR("WEBCLI", "Recevied illegal %s length of %d",
                  packetname(packettype, "").c_str(), length);
            packetid = (int)readchar(port) << 8;
            packetid = packetid | (int)readchar(port);
            for(cnt = 2; cnt < length; cnt = cnt + 1) readchar(port);
            break;
         }
         /* The other packets we simply dump what comes in. */
         default: {
            for(cnt = 0; cnt < length; cnt = cnt + 1) readchar(port);
            packetid = -1;
            break;
         }
      }
   }
   else packetid = -1; /* If there was no ID we set it to zero. */

   /* We check the type. */
   if (packettype == exptype && packetid == -1) {
      PRINTF_INFO("WEBCLI",
        "Received WiFi: %s, DUP=%d,QoS=%d,Retain=%d, len = %d, packetID = NONE",
         packetname(packettype, topic).c_str(), flags>>3, (flags&6)>>1,
         flags & 1, length);
   }
   else if (packettype == exptype) {
      PRINTF_INFO("WEBCLI",
        "Received WiFi: %s, DUP=%d,QoS=%d,Retain=%d, len = %d, packetID = %xh",
         packetname(packettype, topic).c_str(), flags>>3, (flags&6)>>1,
         flags & 1, length, packetid);
   }
   else {
      PRINTF_ERROR("WEBCLI", "Expected packet %s but got %s",
         packetname(exptype, "").c_str(),
         packetname(packettype, topic).c_str());
      return -1;
   }
   if (topic != NULL) delete topic;
   if (pub.length() > 0) printf("Published: %s\n", pub.c_str());

   /* Now we flush out what is left in the packet as we are not checking this
    * yet. Maybe later.
    */
   flush(port);

   /* And we return the packet ID */
   return packetid;
}

void webclient::sendmqtt(int port, mqtt_type_t packettype, int packetid,
      int c0) {
   int cnt;
   unsigned char *msg;
   switch (packettype) {
      case MQTT_TYPE_CONNACK:
         /* Send fixed header and a blank variable header. There is no payload.
          */
         msg = new unsigned char[4];
         msg[0] = packettype<<4;
         msg[1] = 2; /* The length is fixed. */
         /* We send whatever response we got from the TB. */
         msg[2] = (c0 & 0xff00) >> 8;
         msg[3] = c0 & 0x0ff;
         send(port, (void *)msg, 4);
         delete msg;
         break;
      case MQTT_TYPE_SUBACK:
         /* SUBACK has reserved flags. */
         msg = new unsigned char[4+c0];
         msg[0] = packettype<<4;
         msg[1] = 2+c0; /* Length: variable header + QoS responses */
         msg[2] = (packetid & 0xff00) >> 8;
         msg[3] = packetid & 0x0ff;
         for (cnt = 0; cnt < c0; cnt = cnt + 1) {
            msg[4+c0] = 0; /* Write success QoS0 for all. */
         }
         send(port, (void *)msg, 4+c0);
         delete msg;
         break;
      case MQTT_TYPE_PUBACK:
      case MQTT_TYPE_PUBREC:
      case MQTT_TYPE_PUBCOMP:
         msg = new unsigned char[4];
         msg[0] = packettype<<4;
         msg[1] = 2; /* Length: variable header */
         msg[2] = (packetid & 0xff00) >> 8;
         msg[3] = packetid & 0x0ff;
         send(port, (void *)msg, 4);
         delete msg;
         break;
      default:
         PRINTF_ERROR("WEBCLI", "Requested to send unsupported response");
         break;
   }
}

void webclient::publishmqtt(int port, const char *topic, int packetid,
      void *payload, int payloadlen) {
   unsigned char *msg;
   int len;
   int pos;
   unsigned char *payloadptr;
   len = strlen(topic);
   int packlen = 2+2+len+payloadlen;

   msg = new unsigned char[packlen];
   if (msg == NULL) SC_REPORT_FATAL("WEBCLI", "Out of memory");
   msg[0] = ((int)MQTT_TYPE_PUBLISH)<<4;
   msg[1] = packlen - 2;
   /* Topic Length */
   msg[2] = (len & 0xff00) >> 8;
   msg[3] = len & 0x0ff;
   /* Topic */
   for (pos = 0; pos < len; pos = pos + 1) msg[4+pos] = topic[pos];
   /* Packet ID */
   msg[4+len] = (packetid & 0xff00) >> 8;
   msg[4+len+1] = packetid & 0x0ff;
   payloadptr = (unsigned char *)payload;
   for (pos = 0; pos < payloadlen; pos = pos + 1)
      msg[4+len+2+pos] = payloadptr[pos];

   send(port, (void *)msg, packlen);
}

void webclient::respondntp(int port, time_t trec, time_t tsend) {
   int cnt;
   unsigned char msg[12*4];

   /* We first collect the packet. */
   for (cnt = 0; cnt < 12*4; cnt = cnt + 1) {
      msg[cnt] = readchar(port);
   }
   flush(port);

   /* Now we put trec and tsend. We use little-endian. */
   msg[8*4] = (trec>>24) & 0xff;
   msg[8*4+1] = (trec>>16) & 0xff;
   msg[8*4+2] = (trec>>8) & 0xff;
   msg[8*4+3] = trec & 0xff;
   msg[9*4] = 0; /* For now we are not recording the fractional part. */
   msg[9*4+1] = 0;
   msg[9*4+2] = 0;
   msg[9*4+3] = 0;
   msg[10*4] = (tsend>>24) & 0xff;
   msg[10*4+1] = (tsend>>16) & 0xff;
   msg[10*4+2] = (tsend>>8) & 0xff;
   msg[10*4+3] = tsend & 0xff;
   msg[11*4] = 0;
   msg[11*4+1] = 0;
   msg[11*4+2] = 0;
   msg[11*4+3] = 0;

   /* Now we send the packet back to the client with the new times. */
   send(port, (void *)msg, sizeof(msg));
}
void webclient::expectws(int port) {
   unsigned int v;
   unsigned char recv;
   recv = readchar(port); v = (unsigned int)recv;
   recv = readchar(port); v = ((unsigned int)recv << 8) | recv;
   recv = readchar(port); v = ((unsigned int)recv << 8) | recv;
   recv = readchar(port); v = ((unsigned int)recv << 8) | recv;

   printf("FIN: %d\n", (v & 0x1)>0?1:0);
   printf("RSV1: %d\n", (v & 0x2)>0?1:0);
   printf("RSV2: %d\n", (v & 0x4)>0?1:0);
   printf("OPCODE: %d\n", (v>>4) & 0xf);
   printf("MASK: %d\n", (v & 0x10)>0?1:0);
   printf("LEN: %d\n", (v>>8) & 0x7f);
   exit(0);
}

bool webclient::pending(int port) {
   int ind = getind(port);
   if (_portlist[ind].buffer.size() > 0) return true;
   else return false;
}

void webclient::fillbuffers() {
   int port = -1;
   int ind = -1;
   char token, escaped;

   /* We begin initializing the buffers. We need the -1 port. */
   _portlist.push_back(wifiport_t(-1));

   while(1) {
      /* We start by reading the next char. We do a blocking read as this is
       * an internal function, so we can sleep until something comes in.
       */
      token = i_uwifi.from.read();

      /* If an escape comes in we do a second read. */
      if (token == '\xff') escaped = i_uwifi.from.read();
      else escaped = '\0';

      /* If we are in the middle of a message (ind > 0) and data came in,
       * we simply forwarded it to the corresponding stream. Escaped FF we
       * treat the same way.
       */
      if (token == '\xff' && escaped == '\xff' || token != '\xff') {
         if (_portclosed) {
            /* A port was closed since last time we looked. So we need to
             * find out the index again. And we also clear the flag.
             */
            ind = getind(port);
            _portclosed = false;
         }
         /* If the index is not negative, then it is valid. We can write to
          * it. We also raise an event so that if a reader is waiting it can
          * grab the result.
          */
         if (ind >= 0) {
            _portlist[ind].buffer.push_back(token);
            fifowrite_ev.notify();
         }
         continue;
      }

      /* If we have a port associated, we send the message to the correct
       * port.
       */
      if (escaped >= '0' && escaped <= '9') {
         port = 0;
         while(escaped >= '0' && escaped <= '9') {
            port = port * 10 + (int)(escaped - '0');
            escaped = i_uwifi.from.read();
         }
         /* Now we know the port, we need to find out what is the index.
          * Note that listening sockets and accepting sockets have the same
          * port number. The distinction is if it was bound or not.
          */
         ind = getnotclosed(port);

         /* If we did not find the port, we set the index to -1. */
         if (ind < 0) {
            PRINTF_ERROR("WEBCLI", "Got request on closed port %d", port);
            port = -1;
         }

         /* We should look at the control charactyer to know what to do with it.
          */
         if (escaped == '!') {
            /* If this was a close, we then flush out until the next newline
             * and then close the port.
             */
            while (i_uwifi.from.read() != '\n') {}

            /* We close the fifo. We also need to notify any waiting threads. */
            PRINTF_INFO("WEBCLI", "Closed port %d", _portlist[ind].port);
            fifowrite_ev.notify();
            _portlist[ind].closed = true;
         }
         else if (escaped == 'y' || escaped == 'n') {
            /* We got a y or n. We then put it in the correct port and flush
             * the whole message until we see the newline.
             */
            _portlist[ind].buffer.push_back('\xff');
            _portlist[ind].buffer.push_back(escaped);
            do {
               escaped = i_uwifi.from.read();
               _portlist[ind].buffer.push_back(escaped);
            } while(escaped != '\n');
            fifowrite_ev.notify();
         }
         /* If it was a colon, we just discard it.  The rest should go to the
          * port. */
      }
      /* If this is not a port number, we check to see if this is a UDP send. */
      else if (escaped == 's') {
         /* We now parse the message to get the IP and port. */
         String msg = "";
         int newind;
         int a1, a2, a3, a4, port;
         bool firstcolon;
         /* We go until we get the second colon. */
         firstcolon = false;
         do {
            if (escaped == ':') firstcolon = true;
            escaped = i_uwifi.from.read();
            msg = msg + escaped;
         } while (!(escaped == ':' && firstcolon == true));
         /* We check the address and port to see if it is ok. */
         if (5 != sscanf(msg.c_str(), "%d.%d.%d.%d:%d",&a1,&a2,&a3,&a4,&port)) {
            /* If we do not match the format, we warn and dump the rest of the
             * line.
             */
            PRINTF_ERROR("WEBCLI", "Got an illegal send command");
            do escaped = i_uwifi.from.read();
            while (escaped != '\n');
         }
         /* We now try to find an open matching port. Note that we do not yet
          * check the IP. This is UDP, so we just discard messages to non-open
          * ports.
          */
         else {
            newind = getnotclosed(port);
            if (newind < 0) {
               /* We did not find a matching port, so we issue an ERROR and dump it.
                */
               PRINTF_ERROR("WEBCLI", "Got a SEND command to port %d", port);
               do escaped = i_uwifi.from.read();
               while (escaped != '\n');
            }
            /* If we found a matching port, we set the ind to it so that the
             * data goes into the correct port.
             */
            else ind = newind; 
         }
         /* Other commands like connect, we simply let them go to the control
          * fifo.
          */
      }
      else {
         /* We take all chars until the end of the command. Then we send the
          * notification. Note that we do not need to send the notification
          * for every character as we know the command has to have an end.
          */
         _portlist[0].buffer.push_back(token);
         do {
            _portlist[0].buffer.push_back(escaped);
            escaped = i_uwifi.from.read();
         } while (escaped != '\n');
         _portlist[0].buffer.push_back(escaped);
         fifowrite_ev.notify();
      }
   }
}

int webclient::getind(int port) {
   int it;
   for(it = 0; it < (int)_portlist.size(); it = it + 1) {
      if (_portlist[it].port == port) return it;
   }
   return -1;
}

int webclient::getnotclosed(int port) {
   int it;
   for(it = 0; it < (int)_portlist.size(); it = it + 1) {
      if (!_portlist[it].closed && _portlist[it].port == port) return it;
   }
   return -1;
}
