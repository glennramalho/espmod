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

#ifndef _SOCKETS_H
#define _SOCKETS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/ip_addr.h"
typedef uint16_t in_port_t;
#include <unistd.h>
#include <arpa/inet.h>

/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 * lwip_ioctl only supports FIONREAD and FIONBIO, for now
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#if !defined(FIONREAD) || !defined(FIONBIO)
#define IOCPARM_MASK    0x7fU           /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000UL    /* no parameters */
#define IOC_OUT         0x40000000UL    /* copy out parameters */
#define IOC_IN          0x80000000UL    /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#endif /* !defined(FIONREAD) || !defined(FIONBIO) */

#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, unsigned long) /* get # bytes to read */
#endif
#ifndef FIONBIO
#define FIONBIO     _IOW('f', 126, unsigned long) /* set/clear non-blocking i/o */
#endif

/* commands for fnctl */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* File status flags and file access modes for fnctl,
   these are bits in an int. */
#ifndef O_NONBLOCK
#define O_NONBLOCK  1 /* nonblocking I/O */
#endif
#ifndef O_NDELAY
#define O_NDELAY    1 /* same as O_NONBLOCK, for compatibility */
#endif

#ifndef SHUT_RD
  #define SHUT_RD   0
  #define SHUT_WR   1
  #define SHUT_RDWR 2
#endif

/* FD_SET used for lwip_select */
#ifndef FD_SET
#undef  FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define FD_SETSIZE    MEMP_NUM_NETCONN
#define FDSETSAFESET(n, code) do { \
  if (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0)) { \
  code; }} while(0)
#define FDSETSAFEGET(n, code) (((n) - LWIP_SOCKET_OFFSET < MEMP_NUM_NETCONN) && (((int)(n) - LWIP_SOCKET_OFFSET) >= 0) ?\
  (code) : 0)
#define FD_SET(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] |=  (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_CLR(n, p)  FDSETSAFESET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &= ~(1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ISSET(n,p) FDSETSAFEGET(n, (p)->fd_bits[((n)-LWIP_SOCKET_OFFSET)/8] &   (1 << (((n)-LWIP_SOCKET_OFFSET) & 7)))
#define FD_ZERO(p)    memset((void*)(p), 0, sizeof(*(p)))

typedef struct fd_set
{
  unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;

#endif /* FD_SET */

/*
 * Options for level IPPROTO_TCP
 */
#define TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */


#define select(maxfdp1,readset,writeset,exceptset,timeout)     espm_select(maxfdp1,readset,writeset,exceptset,timeout)
#define socket(domain,type,protocol)              espm_socket(domain,type,protocol)
#define lwip_connect_r                            espm_connect_r
#define close(s)                                  espm_close(s)
#define ioctlsocket(s,cmd,argp)                   espm_ioctl(s,cmd,argp)
#define lwip_ioctl_r(s,cmd,argp)                  espm_ioctl_r(s,cmd,argp)
#define recv(s,mem,len,flags)                     espm_recv(s,mem,len,flags)
#define fcntl(s,cmd,val)                          espm_fcntl(s,cmd,val)
#define getsockopt(s,level,optname,opval,optlen)  espm_getsockopt(s,level,optname,opval,optlen)

void espm_socket_init();
int espm_select(int maxfdp1, fd_set *readset, fd_set *writeset,
   fd_set *exceptset, struct timeval *timeout);
int espm_socket(int domain, int type, int protocol);
int espm_connect(int s, const struct sockaddr *name, socklen_t namelen);
int espm_connect_r(int s, const struct sockaddr *name, socklen_t namelen);
int espm_bind(int socket, const struct sockaddr *address,
      socklen_t address_len);
int espm_listen(int socket, int backlog);
int espm_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int espm_close(int s);
int espm_connect_r(int s, const struct sockaddr *name, socklen_t namelen);
int espm_ioctl(int s, long cmd, void *argp);
int espm_fcntl(int s, int cmd, int val);
int espm_ioctl_r(int s, long cmd, void *argp);
int espm_recv(int s, void *mem, size_t len, int flags);
int espm_recvfrom(int s, void *mem, size_t len, int flags,
   struct sockaddr *src_addr, socklen_t *addrlen);
int espm_read(int s, void *mem, size_t len);
int espm_setsockopt (int s, int level, int optname, const void *optval,
   socklen_t optlen);
int espm_getsockopt (int s, int level, int optname, void *optval,
   socklen_t *optlen);
int espm_write (int fd, const void *buf, size_t count);
int espm_send(int socket, const void *buffer, size_t length, int flags);
int espm_sendto(int socket, const void *buffer, size_t length, int flags,
      const struct sockaddr *dest_addr, socklen_t dest_len);
int espm_getpeername(int s, struct sockaddr *addr, socklen_t *addrlen);
int espm_getsockname(int s, struct sockaddr *addr, socklen_t *addrlen);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
   #include <Arduino.h>
#endif

#endif
