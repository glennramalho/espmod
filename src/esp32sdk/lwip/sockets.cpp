/*******************************************************************************
 * sockets.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This file ports several socket functions of the LwIP TCP/IP stack to the
 *   ESPMOD SystemC model. It was based off the functions from the LwIP
 *   TCP/IP stack from the Swedish Institure of Computer Science.
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
 * This file was based off the work covered by the license below:
 *
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc.h>
#include "Arduino.h"
#include "IPAddress.h"
#include "tcpip_adapter.h"
#include "sockets.h"
#include "WiFi.h"
#include <vector>
#include "info.h"
#include <deque>
#include "errno.h"
#include "freertos/task.h"

#define ESCAPENONE 0
#define ESCAPEALL 1
#define ESCAPEFROMCOLON 2

struct fd_t {
   int fdn;             /* File Descriptor Number */
   IPAddress ip;        /* Associated IP address */
   int port;            /* Associated port */
   int owner;           /* Associated bound/listening fdn (if any) */
   unsigned long tmout; /* Timeout or 0 if none */
   int flags;           /* Flags */
   int type;            /* Protocol type: STREAM, DGRAM, etc. */
   bool connected;      /* Is connected */
   bool bound;          /* Is bound/listentimg */
   bool closed;         /* Tagged to close when the buffer goes empty. */
   int maxconnect;      /* Maximum connections (only if listening) */
   int connections;     /* Current connections (only if listening) */
   int tcpnodelay;      /* TCPNODELAY flag */
   int keepalive;       /* Keep alive flag */
   std::deque<unsigned char> buffer; /* Data in buffer */
   bool islistening() {
      return (type == SOCK_STREAM || type == SOCK_SEQPACKET)
         && bound == true && maxconnect >= 0;
   }
   fd_t(IPAddress _ip, int _port, int _owner): buffer() {
      fdn = 0;
      ip = _ip;
      port = _port;
      owner = _owner;
      tmout = 0;
      type = SOCK_STREAM; /* This is the most common one. */
      connected = false;
      bound = false;
      closed = false;
      maxconnect = -1;
      connections = 0;
      flags = 0;
      tcpnodelay = 0;
      keepalive = 0;
   }
   fd_t(IPAddress _ip, int _port): buffer() {
      fdn = 0;
      ip = _ip;
      port = _port;
      owner = -1;
      tmout = 0;
      type = SOCK_STREAM; /* This is the most common one. */
      connected = false;
      bound = false;
      closed = false;
      maxconnect = -1;
      connections = 0;
      flags = 0;
      tcpnodelay = 0;
      keepalive = 0;
   }
   fd_t(): buffer() {
      fdn = 0;
      ip = IPAddress(0,0,0,0);
      port = -1;
      owner = -1;
      tmout = 0;
      type = SOCK_STREAM; /* This is the most common one. */
      connected = false;
      bound = false;
      closed = false;
      maxconnect = -1;
      connections = 0;
      flags = 0;
      tcpnodelay = 0;
      keepalive = 0;
   }
   fd_t(int _owner): buffer() {
      fdn = 0;
      ip = IPAddress(0,0,0,0);
      port = -1;
      owner = _owner;
      tmout = 0;
      type = SOCK_STREAM; /* This is the most common one. */
      connected = false;
      bound = false;
      closed = false;
      maxconnect = -1;
      connections = 0;
      flags = 0;
      tcpnodelay = 0;
      keepalive = 0;
   }
};

std::string fdstring(fd_t *f) {
   char buffer[128];
   snprintf(buffer, 128,
      "fd: %d IP: %s:%d owner: %d: tmout: %ld conn:%c bound:%c\n"
      "closed: %c maxconn: %d connections: %d flags: %x tcpnodelay: %x\n"
      "keepalive: %x",
      f->fdn, f->ip.toString().c_str(), f->port, f->owner,
      f->tmout, (f->connected)?'y':'n', (f->bound)?'y':'n', (f->closed)?'y':'n',
      f->maxconnect, f->connections, f->flags, f->tcpnodelay, f->keepalive);
   return std::string(buffer);
}

std::vector<fd_t> _fdlist;

/* We use these semaphores to make sure we do not have the threads running into
 * each other.
 */
static sc_semaphore __onewrite("__onewrite", 1);
static sc_semaphore __oneread("__oneread", 1);

static sc_event __fifowrite_ev;

int alloc(int owner = -1); /* -1 means no owner. */
bool isopen(int fd);
static int __espm_sendmsg(int port, const char *msg, int size = -1,
   int escape = ESCAPENONE, bool wait = true);
static int __espm_receive(int s, void *mem, size_t len, int flags);
static int __espm_readline(int s, String *msg, bool usetimeout = false);
int espm_getind(int fd);
void fillbuffers();
void takerequest(int *ind);
static int _controlfd;
static int _controlport;
static bool _portclosed;
int getnotlisten(int port);

void espm_socket_init() {
   sc_spawn(fillbuffers);
   _portclosed = false;
}

/* The select function gets a list of file descriptors and returns when one
 * is ready to be used.
 */
int espm_select(int maxfdp1, fd_set *readset, fd_set *writeset,
      fd_set *exceptset, struct timeval *timeout) {
   int rets;
   int it;
   unsigned long starttime;
   errno = 0;

   del1cycle();
   starttime = millis();

   /* If no set was given, we just wait for the time specified. */
   if (readset == NULL && writeset == NULL && timeout != NULL) {
      wait(timeout->tv_sec * 1000000 + timeout->tv_usec, SC_US);
      return 0;
   }
   else if (readset == NULL && writeset == NULL && timeout == NULL) {
      /* This is not quite correct, we are supposed to wait until a signal
       * came in, but being we do not have any signal system, we then just
       * report it as an error.
       */
      errno = -1;
      return EINVAL;
   }
   /* maxfdp1 needs to be checked too. */
   if (maxfdp1 < 0 || maxfdp1 > FD_SETSIZE) {
      errno = -1;
      return EINVAL;
   }
   /* This case does not make much sense, but the manpage says nothing about
    * how to handle this, so we'll leave it this way.
    */
   if (maxfdp1 == 0) {
      return 0;
   }

   /* This scanning part could have been done better using SC events. For now
    * this will work though.
    */
   while(timeout == NULL
      || millis() < starttime + timeout->tv_sec*1000 + timeout->tv_usec/1000) {

      del1cycle();
      /* This will probably change later, but for now we do a quite simple test
       * as we currently have only one client for the file descriptors, this one
       * is the WiFi and it has only one buffer.
       */

      rets = 0;

      /* We are going to loop through all file descriptors. */
      for(it = 0; it < (int)_fdlist.size(); it = it + 1) {

         /* Once we hit the maximum descriptor to checkm we stop. */
         if (_fdlist[it].fdn >= maxfdp1) break;

         /* For each file descriptor we check it. If it is not in a list, we
          * ignore it. If it is, then we see if there is space to count it.
          */
         if (readset != NULL && FD_ISSET(_fdlist[it].fdn, readset)) {
            if (_fdlist[it].buffer.size() > 0) rets = rets + 1;
         }
         if (writeset != NULL && FD_ISSET(_fdlist[it].fdn, writeset)) {
            if (WiFiSerial.availableForWrite() > 0) rets = rets + 1;
         }
      }

      /* Once we are done, if we found descriptors, we stop looking. */
      if (rets > 0) break;
   }

   /* Now we need to indicate which descriptors are ready, if any. For this
    * we scan the list again. We remove all matching cases plus any that
    * is over the maxfdp1 indicator.
    */
   rets = 0;
   for(it = 0; it < (int)_fdlist.size(); it = it + 1) {
      if (readset != NULL && FD_ISSET(_fdlist[it].fdn, readset)) {
         if (_fdlist[it].fdn < maxfdp1
            && _fdlist[it].buffer.size() > 0) rets = rets + 1;
         else FD_CLR(_fdlist[it].fdn, readset);
      }
      if (writeset != NULL && FD_ISSET(_fdlist[it].fdn, writeset)) {
         if (_fdlist[it].fdn < maxfdp1
            && WiFiSerial.availableForWrite() > 0) rets = rets + 1;
         else FD_CLR(_fdlist[it].fdn, writeset);
      }
   }

   return rets;
}

/* The socket function creates a new descriptor. The arguments are currently
 * ignored.
 */
int espm_socket(int domain, int type, int protocol) {
   int fd;
   errno = 0;
   /* We do not have the fancy protocols. */
   if (domain != AF_INET) {
      errno = EAFNOSUPPORT;
      return -1;
   }
   /* Currently we only support SOCK_STREAM and SOCK_DGRAM */
   if (type != SOCK_STREAM && type != SOCK_DGRAM) {
      errno = EPROTONOSUPPORT;
      return -1;
   }
   /* Currently we only support protocol 0 */
   if (protocol != 0) {
      errno = EPROTONOSUPPORT;
      return -1;
   }
   fd = alloc();
   if (fd < 0) return fd;
   int ind = espm_getind(fd);
   if (ind < 0) { errno = EBADF; return -1; }
   _fdlist[ind].type = type;
   return fd;
}
/* For now all we do is set the timeout value. */
int espm_setsockopt(int s, int level, int optname, const void *optval,
      socklen_t optlen) {
   errno = 0;
   if (optval == NULL) { errno = EFAULT; return -1; }
   int ind = espm_getind(s);
   if (ind < 0) { errno = EBADF; return -1; }
   if (level == SOL_SOCKET
         && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)) {
      struct timeval tv;
      if (optlen < sizeof(struct timeval)) {
         errno = EINVAL;
         return -1;
      }
      tv.tv_sec = ((struct timeval *)optval)->tv_sec;
      tv.tv_usec = ((struct timeval *)optval)->tv_usec;
      _fdlist[ind].tmout = tv.tv_sec*1000 + tv.tv_usec/1000;
   }
   else if (level == SOL_SOCKET && optname == SO_KEEPALIVE) {
      if (optlen < sizeof(int)) {
         errno = EINVAL;
         return -1;
      }
      _fdlist[ind].keepalive = *(int *)optval;
   }
   else if (level == SOL_SOCKET && optname == SO_REUSEADDR) {
      /* For now we ignore this option. Perhaps it should be added later. */
   }
   else if (level == IPPROTO_TCP && optname == TCP_NODELAY) {
      if (optlen < sizeof(int)) {
         errno = EINVAL;
         return -1;
      }
      _fdlist[ind].tcpnodelay = *(int *)optval;
   }
   else {
      errno = EINVAL;
      return -1;
   }

   return 0;
}
int espm_connect(int s, const struct sockaddr *name, socklen_t namelen) {
   int ind = espm_getind(s);
   const struct sockaddr_in *rip;
   String msg;
   char request_ipstr[50]; /* Quite long in case it is IPv6. */

   errno = 0;
   /* We first do some basic checking. */
   if (ind < 0) { errno = EBADF; return -1; }
   if (name == NULL) { errno = EPROTOTYPE; return -1; }
   if (namelen < sizeof(sockaddr_in)) { errno = EINVAL; return -1; }

   /* Now we check the data, that the porotype is ok, the file descriptor is
    * valid and so on.
    */
   rip = (const struct sockaddr_in *)name;
   if (rip->sin_family != AF_INET) { errno = EPROTOTYPE; return -1; }

   /* If the socket is of type SOCK_DGRAM, this basically does a bind. */
   if (_fdlist[ind].type == SOCK_DGRAM) return espm_bind(s, name, namelen);

   /* The filedescriptor should not be already connected. */
   if (_fdlist[ind].connected) { errno = EISCONN; return -1; }

   /* If it all looks ok, we can get the IP address and port. */
   _fdlist[ind].ip = IPAddress(rip->sin_addr.s_addr);
   _fdlist[ind].port = ntohs(rip->sin_port);

   /* We now set the timeout and read the connect request. So we set a timeout
    * and send the connect request. After this we wait for the response to
    * return. If it is a "y" it succeeded. If it is a "n" it failed.
    */
   snprintf(request_ipstr, 50, "\xff""c %s:%d\r\n",
      _fdlist[ind].ip.toString().c_str(), _fdlist[ind].port);
   __onewrite.wait();
   int resp;
   WiFiSerial.setTimeout(_fdlist[ind].tmout);
   resp = __espm_sendmsg(_controlport, request_ipstr);
   __onewrite.post();
   if (resp < 0) { return -1; }
   __oneread.wait();
   resp = __espm_readline(_fdlist[ind].fdn, &msg);
   __oneread.post();
   if (resp < 0) { return -1; }

   /* Then we check the message, if it starts with a control followed by a "y"
    * we then tag the socket as connected and return successful.
    */
   if (msg.startsWith("\xffy")) {
      _fdlist[ind].connected = true;
      _fdlist[ind].bound = false;
      PRINTF_INFO("SOCK", "Connected socket %d to IP %s port %d", s,
            _fdlist[ind].ip.toString().c_str(), _fdlist[ind].port);
      return 0;
   }
   /* If we get anything else, we return that it was refused. Now, there are
    * other potential errors, but for simplicity we return always the same
    * one.
    */
   else {
      errno = ECONNREFUSED;
      return -1;
   }
}
int espm_connect_r(int s, const struct sockaddr *name, socklen_t namelen) {
   return espm_connect(s, name, namelen);
}

int espm_bind(int socket, const struct sockaddr *address,
      socklen_t address_len) {
   struct sockaddr_in *radd = (struct sockaddr_in *)address;
   int ind = espm_getind(socket);
   errno = 0;

   /* We first do some basic checking. */
   if (ind < 0) { errno = EBADF; return -1; }
   if (address == NULL) { errno = EINVAL; return -1; }
   if (address_len < sizeof(sockaddr_in)) { errno = EINVAL; return -1; }
   if (_fdlist[ind].bound) { errno = EINVAL; return -1; }

   /* We check that the protocol is the correct one. */
   if (radd->sin_family != AF_INET) { errno = EOPNOTSUPP; return -1; }
   _fdlist[ind].ip = IPAddress(radd->sin_addr.s_addr);
   _fdlist[ind].port = ntohs(radd->sin_port);
   _fdlist[ind].bound = true;
   /* The maxconnect is -1 until listen runs. */
   _fdlist[ind].maxconnect = -1;

   return ESP_OK;
}

int espm_listen(int socket, int backlog) {
   int ind = espm_getind(socket);
   errno = 0;

   /* Listen can only be ran on bound sockets of type SOCK_STREAM or SOCK_SEQPACKET.
    */
   if (ind < 0) { errno = EBADF; return -1; }
   if (!_fdlist[ind].bound) { errno = EINVAL; return -1; }
   if (_fdlist[ind].type != SOCK_STREAM && _fdlist[ind].type != SOCK_SEQPACKET) {
      errno = EOPNOTSUPP;
      return -1;
   }
   _fdlist[ind].maxconnect = backlog;
   return ESP_OK;
}

int espm_accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
   int a1, a2, a3, a4, port;
   int afd, aind;
   String msg;
   int ind = espm_getind(s);
   errno = 0;
   /* We do the basic checking. */
   if (addr == NULL && addrlen != NULL || addr != NULL && addrlen == NULL) {
      errno = EINVAL; return -1;
   }
   if (addrlen != NULL && *addrlen < sizeof(struct sockaddr_in))
   { errno = EINVAL; return -1; }
   if (ind < 0) { errno = EBADF; return -1; }
   if (_fdlist[ind].type != SOCK_STREAM && _fdlist[ind].type != SOCK_SEQPACKET)
   { errno = EOPNOTSUPP; return -1; }
   if (!_fdlist[ind].islistening()) { errno = EINVAL; return -1; }

   /* If the other side of a file descriptor has been closed we also return
    * zero width, but only if there is nothing else in the stream. */
   if (_fdlist[ind].closed && _fdlist[ind].buffer.size() == 0)
   { errno = EINVAL; return -1; }
   if ((_fdlist[ind].flags & O_NONBLOCK) == O_NONBLOCK
         && _fdlist[ind].buffer.size() == 0) {
      errno = EWOULDBLOCK;
      return -1;
   }

   int resp;
   __oneread.wait();
   resp = __espm_readline(s, &msg);
   __oneread.post();
   if (resp < 0) return -1;

   /* Now we parse the message to get the port.  If it is not correct, we
    * return a proto error.
    */
   if (5 != sscanf(msg.c_str(),
            "\xff""c %d.%d.%d.%d:%d", &a1, &a2, &a3, &a4, &port)) {
      errno = EPROTO;
      return -1;
   }

   /* And we create the accept socket. */
   afd = alloc(s);
   if (afd < 0) return -1;
   aind = espm_getind(afd);
   if (aind < 0) { errno = EBADF; return -1; }
   _fdlist[aind].ip = _fdlist[ind].ip;
   _fdlist[aind].port = _fdlist[ind].port;
   _fdlist[aind].flags = _fdlist[ind].flags;
   _fdlist[aind].connected = true;
   _fdlist[aind].bound = false;
   _fdlist[aind].maxconnect = 0;

   /* We now can send the accept message to the client. */
   char buffer[80];
   uint8_t mac[6];
   tcpip_adapter_ip_info_t ipinfo;
   tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipinfo);
   esp_wifi_get_mac(WIFI_IF_AP, mac);
   snprintf(buffer, 80, "\xff%dy %02x:%02x:%02x:%02x:%02x:%02x %s:%d\r\n",
      _fdlist[aind].port, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
      IPAddress(ipinfo.ip.addr).toString().c_str(), _fdlist[aind].port);
   __onewrite.wait();
   resp = __espm_sendmsg(_controlport, buffer);
   __onewrite.post();
   if (resp <= 0) { espm_close(afd); errno = ECONNABORTED; return -1; }

   PRINTF_INFO("SOCK", "Accepted socket %d lidstening %d from IP %s port %d", afd,
         s, _fdlist[ind].ip.toString().c_str(), _fdlist[ind].port);

   /* If the address is not null, then we fill the struct with the connection
    * data. It should never fail as we should have done all the checking already.
    */
   if (addr != NULL) (void)espm_getsockname(afd, addr, addrlen);

   /* And we return the accept socket. */
   return afd;
}

/* Close */
int espm_close(int s) {
   errno = 0;
   int ind = espm_getind(s);

   /* Illegal descriptors we block. We also block a close on the control
    * descriptor as it is an internal thing.
    */
   if (ind < 0 || s == _controlfd) {
      errno = EBADF;
      return -1;
   }
   PRINTF_INFO("SOCK", "Closing fd %d port %d (%s)", s, _fdlist[ind].port,
      (_fdlist[ind].islistening())?"listening":"notlistening");

   /* If this FD has an owner, we need the index. */
   int ownerind = espm_getind(_fdlist[ind].owner);

   /* If this is a listen port, we just close it and drop the rest. */
   if (_fdlist[ind].islistening()) {
      int fd;
      for (fd = 0; fd < (int)_fdlist.size(); fd = fd + 1) {
         if (ownerind >= 0) espm_close(_fdlist[fd].owner);
      }
   }
   /* If it is an accepted connection, we need to decrement the connection.
    */
   else if (ownerind >= 0)
      _fdlist[ownerind].connections = _fdlist[ownerind].connections - 1;

   /* If any destructors in the firmware calls espm_close, we will get extra
    * closes when the simulation is exiting. We can then get a coredump as the
    * channels have been dealocated. Removing elements from the fdlist in this
    * phase will also kill the tool for some reason. So we check it, and if
    * the simulation is no longer running, we just quit and let the system
    * deallocate everything nicely.
    */
   if (!sc_is_running()) return 0;

   /* If connected, we send a close to the other side. */
   if (_fdlist[ind].connected) {
      char buf[15];
      snprintf(buf, 15, "\xff%d!\r\n", _fdlist[ind].port);
      __onewrite.wait();
      (void)__espm_sendmsg(_controlport, buf);
      __onewrite.post();
   }
   /* We remove the file descriptor */
   _fdlist.erase(_fdlist.begin()+ind);

   /* We raise a flag so that the fillbuffers() task knows it needs to
    * recalculate the index.
    */
   _portclosed = true;

   return 0;
}
int espm_ioctl_r(int s, long cmd, void *argp) {
   int ind = espm_getind(s);
   errno = 0;
   if (cmd != FIONREAD) {
      errno = EINVAL;
      return -1;
   }
   if (argp == NULL) {
      errno = EINVAL;
      return -1;
   }
   if (ind < 0) {
      errno = ENOTTY;
      return -1;
   }
   *(int *)argp = _fdlist[ind].buffer.size();
   return 0;
}
int espm_fcntl(int s, int cmd, int val) {
   int ind = espm_getind(s);
   errno = 0;
   if (ind < 0) {
      errno = EBADF;
      return -1;
   }
   switch(cmd) {
      case F_GETFL: return _fdlist[ind].flags;
      case F_SETFL:
         _fdlist[ind].flags = val;
         return 0;
      default:
         errno = EINVAL;
         return -1;
   }
}
int espm_getsockopt (int s, int level, int optname, void *optval,
      socklen_t *optlen) {
   errno = 0;
   int ind = espm_getind(s);
   if (ind < 0) {
      errno = EBADF;
      return -1;
   }
   if (optval == NULL) { errno = EFAULT; return -1; }
   if (optlen == NULL) { errno = EFAULT; return -1; }
   if (level == SOL_SOCKET
         && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)) {
      struct timeval tv;
      tv.tv_sec = _fdlist[ind].tmout / 1000;
      tv.tv_usec = (_fdlist[ind].tmout % 1000) * 1000;
      if (*optlen > sizeof(struct timeval))
         *optlen = sizeof(struct timeval);
      memcpy((void *)optval, (void *)&tv, *optlen);
   }
   else if (level == SOL_SOCKET && optname == SO_KEEPALIVE) {
      if (*optlen > sizeof(int)) *optlen = sizeof(int);
      memcpy((void *)optval, (void *)&(_fdlist[ind].keepalive), *optlen);
   }
   else if (level == IPPROTO_TCP && optname == TCP_NODELAY) {
      if (*optlen > sizeof(int)) *optlen = sizeof(int);
      memcpy((void *)optval, (void *)&(_fdlist[ind].tcpnodelay), *optlen);
   }
   else {
      errno = EINVAL;
      return -1;
   }
   return 0;
}
int espm_write (int fd, const void *buf, size_t count) {
   int resp;
   const char *msg = (const char *)buf;
   int ind = espm_getind(fd);

   if (ind < 0) { errno = EBADF; return -1; }
   /* SOCK_DGRAM needs to be bound to have an address. */
   if (!_fdlist[ind].bound && _fdlist[ind].type == SOCK_DGRAM) {
      errno = EDESTADDRREQ; return -1;
   }
   /* SOCK_STREAM must be connected, either via a connect() or listening accept(). */
   else if (!_fdlist[ind].connected && _fdlist[ind].type != SOCK_DGRAM) {
      errno = EBADF; return -1;
   }

   __onewrite.wait();
   resp = __espm_sendmsg(_fdlist[ind].port, msg, count, ESCAPEALL,
         ((_fdlist[ind].flags & O_NONBLOCK) == O_NONBLOCK)?false:true);
   __onewrite.post();
   return resp;
}

int espm_recv(int s, void *mem, size_t len, int flags) {
   int resp;

   __oneread.wait();
   errno = 0;
   resp = __espm_receive(s, mem, len, flags);
   __oneread.post();
   return resp;
}

int espm_recvfrom(int s, void *mem, size_t len, int flags,
      struct sockaddr *src_addr, socklen_t *addrlen) {
   int resplen;

   /* This should not happen. If there is an address to write to we need
    * an address length. The address length valid and the address null does
    * not make any sense, so we will be picky and complain.
    */
   if (src_addr != NULL && addrlen == NULL) { errno = EINVAL; return -1; }
   if (src_addr == NULL && addrlen != NULL) { errno = EINVAL; return -1; }

   /* The message seems to be valid, so we can attempt to receive it. */
   __oneread.wait();
   errno = 0;
   resplen = __espm_receive(s, mem, len, flags);
   __oneread.post();

   /* If it failed, we then go ahead and return failure. */
   if (resplen < 0) return -1;

   /* If it succeded, we can return the source address. We only do this if the
    * address points to a real place. If it fails, we return a failure.
    */
   if (src_addr != NULL && addrlen != NULL
      && espm_getpeername(s, src_addr, addrlen) > 0) return -1;

   /* If it succeeded, we return the responded length. */
   return resplen;
}

int espm_read(int s, void *mem, size_t len) {
   return espm_recv(s, mem, len, MSG_WAITALL);
}

int __espm_readline(int s, String *msg, bool usetimeout) {
   char nchar;
   int starttime = millis();
   int ind;
   bool crseen;
   int p;
   unsigned long timeouttarg;

   ind = espm_getind(s);
   errno = 0;
   if (ind < 0) { errno = EBADF; return -1; }

   /* We precalculate the timeout target. */
   if (!usetimeout || _fdlist[ind].tmout == 0) timeouttarg = 0;
   else timeouttarg = starttime + _fdlist[ind].tmout;

   /* We are going to wait for the message to come in. We start off initializing
    * the crseen bool to false. Later it is used to scan for the crlf sequence.
    */
   crseen = false;
   while(1) {
      /* Now we wait for either the data or the timeout. */
      while (_fdlist[ind].buffer.size() == 0) {
         /* If there is no timeout, we wait indefinitely. If there is one, then
          * we wait only the specified time.
          */
         if (timeouttarg == 0) wait(__fifowrite_ev);
         else wait(sc_time(timeouttarg - millis(), SC_MS), __fifowrite_ev);

         /* We then check the result. If the timeout expired, we raise a timeout. */
         if (!__fifowrite_ev.triggered() && timeouttarg != 0 &&
               millis() >= timeouttarg) {
             errno = ETIMEDOUT;
             return -1;
         }
      }

      /* Now we look at the char. We are looking for "\r\n". */
      nchar = (char)(_fdlist[ind].buffer.front());
      _fdlist[ind].buffer.pop_front();

      /* If we see the \r\n, we break. */
      if (nchar == '\n' && crseen) break;
      /* If we see a \r followed by something else, we put the \r in the msg. */
      else if (crseen) {
         *msg = *msg + '\r';
         p = p + 1;
         crseen = false;
      }

      /* Now we look at the char that came in. If it is a \r,
       * crseen goes true. If not, we then just push it onto the list. */
      if (nchar == '\r') crseen = true;
      else {
         *msg = *msg + nchar;
         p = p + 1;
      }
   }

   return p;
}

int espm_send(int socket, const void *buffer, size_t length, int flags) {
   int resp;
   const char *msg = (const char *)buffer;
   int ind = espm_getind(socket);
   errno = 0;
   if (ind < 0) { errno = EBADF; return -1; }
   if (length > 1460) { errno = EMSGSIZE; return -1; }
   /* SOCK_DGRAM needs to be bound to have an address. */
   if (!_fdlist[ind].bound && _fdlist[ind].type == SOCK_DGRAM) {
      errno = EDESTADDRREQ; return -1;
   }
   /* SOCK_STREAM must be connected, either via a connect() or listening accept(). */
   else if (!_fdlist[ind].connected && _fdlist[ind].type != SOCK_DGRAM) {
      errno = ENOTCONN; return -1;
   }

   if (_fdlist[ind].type != SOCK_STREAM && _fdlist[ind].connected) {
      errno = EISCONN;
      return -1;
   }
   /* SOCK_DGRAMs also must be bound. */
   if (_fdlist[ind].type != SOCK_STREAM && !_fdlist[ind].bound) {
      errno = EAGAIN;
      return -1;
   }
   /* And we call the internal send task if it is all ok. */
   __onewrite.wait();
   resp = __espm_sendmsg(_fdlist[ind].port, msg, length, ESCAPEALL,
         ((_fdlist[ind].flags & O_NONBLOCK) == O_NONBLOCK ||
          (flags & MSG_DONTWAIT) == MSG_DONTWAIT)?false:true);
   __onewrite.post();
   return resp;
}

/* sendto is intended for UDP connections. It does not need a connection
 * to be made. It simply sends the packet to the provided destination.
 */
int espm_sendto(int socket, const void *buffer, size_t length, int flags,
      const struct sockaddr *dest_addr, socklen_t dest_len) {
   int ind;
   int resp;
   const struct sockaddr_in *dip;
   dip = (const struct sockaddr_in *)dest_addr;

   errno = 0;
   /* First we get rid of some dumb errors. */
   if (dest_addr == NULL && dest_len != 0) { errno = EINVAL; return -1; }
   if (dest_addr != NULL && dest_len == 0) { errno = EINVAL; return -1; }

   /* If no dest_addr was given, as the man page states, it is the same as
    * calling send().
    */
   if (dest_addr == NULL) return espm_send(socket, buffer, length, flags);

   /* We get the descriptor and do the basic checks. */
   ind = espm_getind(socket);
   if (ind < 0) { errno = EBADF; return -1; }
   if (length > 1460) { errno = EMSGSIZE; return -1; }

   /* We can optionally give an EISCONN if the we are dealing with a SOCK_STREAM and
    * we get an address.
    */
   if (_fdlist[ind].type == SOCK_STREAM || _fdlist[ind].type==SOCK_SEQPACKET) {
      errno = EISCONN;
      return -1;
   }
   /* Regular DGRAMS also need to return EISCONN if they are connected. */
   else if (_fdlist[ind].connected) { errno = EISCONN; return -1; }

   /* Now we make the packet and send it. The packet must have an IP address. */
   char *msgtosend = new char[length+50];
   if (msgtosend == NULL) { errno = ENOMEM; return -1; }
   snprintf(msgtosend, 50, "\xff""s %s:%d:",
      IPAddress(dip->sin_addr.s_addr).toString().c_str(), ntohs(dip->sin_port));
   /* We tack the buffer to the end of the command. */
   int cmdlen = strlen(msgtosend);
   memcpy(&(msgtosend[cmdlen]), buffer, length);
   /* And we send it. */
   __onewrite.wait();
   resp = __espm_sendmsg(_controlport, msgtosend, cmdlen+length, ESCAPEFROMCOLON,
      (flags & MSG_DONTWAIT) == MSG_DONTWAIT||_fdlist[ind].flags & O_NONBLOCK);
   __onewrite.post();
   delete msgtosend;
   /* We return the characters we sent, but we do not include the header. */
   if (resp < 0) return resp;
   else if (resp - cmdlen <= 0) return 0;
   else return resp - cmdlen;
}

int espm_getpeername(int s, struct sockaddr *addr, socklen_t *addrlen) {
   static struct sockaddr_in ad;

   int ind = espm_getind(s);
   errno = 0;
   /* We do the basic checking. */
   if (addr == NULL || addrlen == NULL) { errno = EFAULT; return -1; }
   if (ind < 0) { errno = EBADF; return -1; }
   if (*addrlen < 0) { errno = EINVAL; return -1; }
   if (!_fdlist[ind].connected) { errno = ENOTCONN; return -1; }
   /* We build the sockaddr. */
   ad.sin_family = AF_INET;
   ad.sin_port = htons(_fdlist[ind].port);
   ad.sin_addr.s_addr = _fdlist[ind].ip;
   /* We now copy what we can into the place pointed to by addr. addrlen caps
    * our copy if it is too small. If it is too big, we update the value in the
    * addrlen.
    */
   if (*addrlen >= sizeof(struct sockaddr)) *addrlen = sizeof(sockaddr);
   memcpy((void *)addr, (void *)&ad, *addrlen);
   return 0;
}

int espm_getsockname(int s, struct sockaddr *addr, socklen_t *addrlen) {
   int ind = espm_getind(s);
   errno = 0;
   if (ind < 0) { errno = EBADF; return -1; }
   if (addr == NULL || addrlen == NULL) { errno = EFAULT; return -1; }
   if (*addrlen < sizeof(struct sockaddr_in)) { errno = EINVAL; return -1; }

   struct sockaddr_in *ad = (struct sockaddr_in *)addr;
   ad->sin_family = AF_INET;
   ad->sin_port = htons(_fdlist[ind].port);
   ad->sin_addr.s_addr = _fdlist[ind].ip;
   *addrlen = sizeof(struct sockaddr_in);
   return 0;
}
/***************************************************************************/
int alloc(int owner) {
   fd_t newfd(owner);
   int newfdn;
   if (_fdlist.size() == 0) newfdn = 1;
   else {
      newfdn = _fdlist[_fdlist.size()-1].fdn;
      /* This could have been done better, but for now will work. */
      if (newfdn == INT32_MAX) {
         errno = EMFILE;
         PRINTF_ERROR("SOCK", "Too many file descriptors.");
         return -1;
      }
      newfdn = newfdn + 1;
   }
   newfd.fdn = newfdn;
   _fdlist.push_back(newfd);
   return newfdn;
}

bool isopen(int fd) {
   if (fd < 0) return false;
   if (fd == 0) return true;
   int ind = espm_getind(fd);
   if (ind < 0) return false;
   return _fdlist[ind].connected;
}

int espm_getind(int fd) {
   int it;
   /* For some reason sometimes the functions are called with a fictitious
    * fd = -1. We then check and if we see a negative file descriptor we
    * return a negative index. This will be seen as a bad descriptor.
    */
   if (fd < 0) return -1;

   /* And we scan for the descriptor */
   for(it = 0; it != (int)_fdlist.size(); it = it + 1) {
      if (fd == _fdlist[it].fdn) return it;
   }
   return -1;
}

int __espm_receive(int s, void *mem, size_t len, int flags) {
   size_t p;
   unsigned long starttime = millis();
   unsigned long timeouttarg;
   unsigned char *msg = (unsigned char *)mem;

   int ind = espm_getind(s);
   if (ind < 0) {
      errno = EBADF;
      return -1;
   }

   /* To read a SOCK_STREAM must be set to connected. SOCK_DGRAM need to be bound. */
   if (_fdlist[ind].type == SOCK_STREAM && !_fdlist[ind].connected) {
      errno = ENOTCONN;
      return -1;
   }
   else if (_fdlist[ind].type != SOCK_STREAM && !_fdlist[ind].bound) {
      errno = EAGAIN;
      return -1;
   }

   /* We now check the flags. */
   if (((flags & MSG_DONTWAIT) == MSG_DONTWAIT || 
         _fdlist[ind].flags & O_NONBLOCK)
         && (_fdlist[ind].buffer.size() == 0 || len == 0)) {
      errno = EWOULDBLOCK;
      return -1;
   }
   /* This could change later but for now we do not support peeking. */
   if (flags & ~(MSG_WAITALL | MSG_DONTWAIT) > 0) {
      errno = EOPNOTSUPP;
      return -1;
   }
   /* If the other side of a file descriptor has been closed we also return
    * zero width, but only if there is nothing else in the stream. */
   if (_fdlist[ind].closed && _fdlist[ind].buffer.size() == 0) {
      return 0;
   }

   /* If we now read until we take in all chars or, if WAITALL was not present,
    * until we have emptied out the buffer.
    */
   p = 0;

   /* We precalculate the timeout target. */
   if (_fdlist[ind].tmout == 0) timeouttarg = 0;
   else timeouttarg = starttime + _fdlist[ind].tmout;

   while(p < len) {
      /* If we don't have waitall and the buffer is empty, we stop. */
      if ((flags & MSG_WAITALL) != MSG_WAITALL
          && _fdlist[ind].buffer.size() == 0) break;

      /* If we do have the WAITALL and there is no data, we wait. */
      if ((flags & MSG_WAITALL) == MSG_WAITALL) {
         while(_fdlist[ind].buffer.size() == 0) {
            /* We wait for the next notification or the timeout. We do not have one
             * event for each buffer, so we just wait for any write and then check
             * to see if we got the data we needed.
             */
            if (timeouttarg == 0) wait(__fifowrite_ev);
            else wait(sc_time(timeouttarg - millis(), SC_MS), __fifowrite_ev);

            /* If we did not get an event triggered, we check the timeout. If it was
             * triggered, we raise the flag.
             */
            if (!__fifowrite_ev.triggered() && timeouttarg != 0 &&
                  millis() >= timeouttarg) {
                errno = ETIMEDOUT;
                return -1;
            }
         }
      }
      /* And we take the char */
      msg[p] = _fdlist[ind].buffer.front();
      _fdlist[ind].buffer.pop_front();
      p = p + 1;
   }
   return p;
}

int __espm_sendmsg(int port, const char *msg, int size, int escstyle,
      bool wait) {
   int p;
   int portstrptr;
   static int lastport = -1;
   char portstr[14];
   int len;
   bool escape;
   /* If requested, we begin sending the port number. This, we do only if the
    * previous packet was not a control packet (they have no ports) nor was
    * the same as the current port.
    */
   if (port >= 0 && lastport != port) {
      sprintf(portstr, "\xff%d:", port);
      lastport = port; /* And we record the port for the next pass. */
   }
   /* If the lastport is the same as this one, we simply continue the previous
    * print message. There is no need to declare a second port if it was already
    * declared.
    */
   else if (port >= 0 && lastport == port) {
      portstr[0] = '\0';
   }
   /* Finally we have the control messages. These do not necessarily have an
    * associated port. We just trust it is correct and send it. We simply send
    * the message along. We also reset the last port indicator as even if the
    * port remains the same we need to send the port indicator again.
    */
   else {
      portstr[0] = '\0';
      lastport = -1;
   }

   /* We check the escape style to see if we should escape or not. */
   if (escstyle == ESCAPEALL) escape = true;
   else escape = false;

   /* We only start sending if there is enough space for the port string
    * (if requested) and at least one character. If the first char is an
    * escape, then two chars.
    */
   len = strlen(portstr);
   if (WiFiSerial.availableForWrite() < len+1 && msg[0] != 0xff ||
         WiFiSerial.availableForWrite() < len+2 &&
         msg[0] == 0xff && msg[1] == 0xff) {
      return 0;
   }

   /* Now we send the message. We first will send the port string, if any, and
    * then we send the main string.  If size is -1, we go until msg[p] hits
    * the nullchar. If size is >= 0, we then go until all chars have been sent.
    */
   portstrptr = 0; p = 0;
   while(portstr[portstrptr]!='\0' || size>0 && p<size || size<0 && msg[p]!='\0') {
      /* Each time we delay one cycle to prevent us from getting stuck in a busy
       * loop without time delays.
       */
      //if (port >= 0) del1cycle();

      /* It is important to distinguish between the portstr and the main
       * message. The port string we already know there is space, so we simply
       * send it. There is no escaping either.
       */
      if (portstr[portstrptr] != '\0') {
         WiFiSerial.write(portstr[portstrptr]);
         /* And we increment the pointer but not the sent count as these chars
          * were not requested by the client. */
         portstrptr = portstrptr + 1;
         continue;
      }

      /* We look at the current character. If it is a colon and we are set to
       * start escaping from the first colon, we then switch the flag.
       */
      if (!escape && escstyle == ESCAPEFROMCOLON && msg[p] == ':')
         escape = true;

      /* Now we look at the main message. If this is a non-blocking operation
       * and there is no space in the FIFO we just quit. So we see if there is
       * space to send a char. If the char is an FF (and we are escaping them)
       * we then need two spaces so that we can send two FF chars.
       */
      if (!wait && (WiFiSerial.availableForWrite() == 0 ||
            msg[p] == 0xff && WiFiSerial.availableForWrite() == 1 && escape)) {
         /* We release the semaphore and return what has been sent so far. */
         return p;
      }

      /* And we send the char. We need to keep in mind if we are sending the
       * port string or the main message.
       */
      WiFiSerial.write(msg[p]);

      /* If we see an FF and we got the request to escape them, we then send
       * it twice.
       */
      if (escape && msg[p] == 0xff) WiFiSerial.write(msg[p]);

      /* And we count the char as sent. Note that escaped 0xff are not counted
       * as two but as one.
       */
      p = p + 1;
   }
   return p;
}

int getnotlisten(int port) {
   int it;
   if (port < 0) return -1;
   for(it = 0; it < (int)_fdlist.size(); it = it + 1) {
      if (!_fdlist[it].islistening() && _fdlist[it].port == port) return it;
   }
   return -1;
}

void fillbuffers() {
   int port;
   int ind;
   unsigned char token, escaped;
   String msg;

   /* We also need a control socket. It should be attached to the control
    * port -1. We then create it now.
    */
   _controlfd = alloc();
   _controlport = -1;
   _fdlist[espm_getind(_controlfd)].port = _controlport;

   /* The default though will not be this one but it will be -1, meaning none
    * selected. We also set the port to an invalid value.
    */
   ind = -1;
   port = -1;

   /* If the ports have not been initialized, we have to just stop. */
   if (!WiFiSerial.isinit()) {
      SC_REPORT_INFO("SOCK", "Socket model not active");
      return;
   }

   while(1) {
      /* We start by reading the next char. We do a blocking read as this is
       * an internal function, so we can sleep until something comes in.
       */
      token = WiFiSerial.bl_read();

      /* If an escape comes in we do a second read. */
      if (token == 0xff) escaped = WiFiSerial.bl_read();
      else escaped = '\0';

      /* If we are in the middle of a message (ind > 0) and data came in,
       * we simply forwarded it to the corresponding stream. Escaped FF we
       * treat the same way.
       */
      if (token == 0xff && escaped == 0xff || token != 0xff) {
         if (_portclosed) {
            /* A port was closed since last time we looked. So we need to
             * find out the index again. And we also clear the flag.
             */
            ind = getnotlisten(port);
            _portclosed = false;
         }
         /* If the index is not negative, then it is valid. We can write to
          * it.
          */
         if (ind >= 0) _fdlist[ind].buffer.push_back(token);
         __fifowrite_ev.notify();
         continue;
      }

      /* We got a control message, so we look at it to see what it is. If it
       * begins with a number it is either data or a close request. We then
       * extract the number and then we can see what to do.
       */
      if (escaped >= '0' && escaped <= '9') {
         port = 0;
         while(escaped >= '0' && escaped <= '9') {
            port = port * 10 + (int)(escaped - '0');
            escaped = WiFiSerial.bl_read();
         }
         /* Now we know the port, we need to find out what is the index.
          * Note that listening sockets and accepting sockets have the same
          * port number. If we get -1, then there was no match.
          */
         ind = getnotlisten(port);
         if (ind < 0) continue;

         /* We should see now either colon or exclamation mark. This will tell
          * us if this is a close or a char. Only the first char can do this.
          */
         if (escaped == '!') {
            /* If this was a close, we then flush out until the next newline
             * and then close the port.
             */
            while (WiFiSerial.bl_read() != '\n') {}
         }
         else if (escaped == 'y' || escaped == 'n') {
            /* We got a y or n. We then put it in the correct port and flush
             * the whole message until we see the newline.
             */
            _fdlist[ind].buffer.push_back((unsigned char)'\xff');
            _fdlist[ind].buffer.push_back(escaped);
            do {
               escaped = WiFiSerial.bl_read();
               _fdlist[ind].buffer.push_back(escaped);
            } while(escaped != '\n');
            /* Once the command is in we issue the notification. */
            __fifowrite_ev.notify();
         }
         /* If it was a colon, we just discard it.  The rest should go to the
          * port. */
      }
      /* We got a connect request. These are handled automatically here,
       * as long as there is a socket listening for it.
       */
      else if (escaped == 'c') takerequest(NULL);
      /* The send messages are used by UDP. These packets look kinda a conenct
       * followed by data. So, we use the same function. If it is valid, then
       * the function changes the ind so that the data will be taken too.
       */
      else if (escaped == 's') takerequest(&ind);
      /* We have another control command. We then just send it to the control
       * buffer and let the requesting command deal with it.
       */
      else {
         /* We take all chars until the end of the command. Then we send the
          * notification. Note that we do not need to send the notification
          * for every character as we know the command has to have an end.
          */
         _fdlist[0].buffer.push_back(token);
         do {
            _fdlist[0].buffer.push_back(escaped);
            escaped = WiFiSerial.bl_read();
         } while (escaped != '\n');
         _fdlist[0].buffer.push_back(escaped);
         __fifowrite_ev.notify();
      }
   }
}

/* Take request is for handling connect and send requests. For connect, the
 * ind argument should be NULL. For the send command, the ind should be a
 * pointer to the index variable so the data can be collected.
 */
void takerequest(int *ind) {
   unsigned char rec;
   bool bad;
   String msg;
   int a1, a2, a3, a4, port;
   int it;
   bool connect;

   /* We can have a 'c' or an 's' command. We then look for either:
    *    %d.%d.%d.%d:%p   <connect>
    *
    *    or
    *
    *    %d.%d.%d.%d:%p:  <send>
    */
   /* If the ind pointer is NULL, this is a connect command. If not, it is
    * a send.
    */
   if (ind == NULL) connect = true;
   else connect = false;
   
   /* We then first look for a colon. We skip any spaces. */
   do {
      rec = WiFiSerial.bl_read();
      if (rec == ' ') continue;
      msg = msg + (char)rec;
   } while(rec != ':');
   /* Now we go until the newline or second colon. */
   do {
      rec = WiFiSerial.bl_read();
      if (rec == ' ') continue;
      msg = msg + (char)rec;
   } while(rec != ':' && rec != '\n');

   /* If this is a connect and we did not get a \n, we discard everything that
    * comes in until the newline.
    */
   if (connect) while(rec != '\n') rec = WiFiSerial.bl_read();

   /* We should now have in msg the connection part of the command. We can then
    * parse it to see if we have all we need and it is correct.
    */
   if (5 != sscanf(msg.c_str(), "%d.%d.%d.%d:%d", &a1, &a2, &a3, &a4, &port)) {
      bad = true;
   }

   /* We now look to see if we have a receiving descriptor. This depends on the
    * protocol type.
    *    - the port must match
    *    - connect can only go to SOCK_STREAM and send to the others.
    *    - the port must be listening, either because it was bound and listen()
    *        or just bound. The just bound is for SOCK_DGRAM.
    */
   if (!bad) for(it = 0; it < (int)_fdlist.size(); it = it + 1) {
      if (_fdlist[it].islistening() && _fdlist[it].port == port &&
         (connect && _fdlist[it].type == SOCK_STREAM ||
         !connect && _fdlist[it].type != SOCK_STREAM)) break;
   }

   /* First we look at datagrams we got. If the message is not bad, the
    * descriptor is valid and it is not of type STREAM, we then return ok. The
    * calling function will take in the data.
    */
   if (!bad && it < (int)_fdlist.size() && _fdlist[it].type != SOCK_STREAM) {
      *ind = it;
      return;
   }
    
   /* SOCK_STREAM packets need a connection. We then check that the number
    * of connections has been exceeded, we reject it. We also reject bad
    * messages. We respond by sending the request back so that the correct
    * client knows the message is for him.
    */
   if (bad || it == (int)_fdlist.size()
         || _fdlist[it].connections == _fdlist[it].maxconnect) {
      msg = _fdlist[it].port + msg;
      msg = String("\xff") + msg;
      __onewrite.wait();
      (void)__espm_sendmsg(_controlport, msg.c_str());
      __onewrite.post();
   }
   else {
      /* The others we put in the list to be accepted. We already count it
       * as an accepted connection although the accept function is the one
       * that does the accepting.
       */
      unsigned int p;
      _fdlist[it].buffer.push_back((unsigned char)0xff);
      _fdlist[it].buffer.push_back((unsigned char)'c');
      _fdlist[it].buffer.push_back((unsigned char)' ');
      for(p = 0; p < msg.length(); p = p + 1)
         _fdlist[it].buffer.push_back((unsigned char)(msg[p]));
      _fdlist[it].connections = _fdlist[it].connections + 1;
   }
}
