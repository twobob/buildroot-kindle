/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <direct/build.h>
#include <direct/types.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/debug.h>
#include <direct/util.h>

#include <direct/stream.h>

struct __D_DirectStream {
     int                   magic;
     int                   ref;
     
     int                   fd;
     unsigned int          offset;
     int                   length;
     
     char                 *mime;

     /* cache for piped streams */
     void                 *cache;
     unsigned int          cache_size;

#if DIRECT_BUILD_NETWORK
     /* remote streams data */
     struct {
          int              sd;
          
          char            *host;
          int              port;
          struct addrinfo *addr;
          
          char            *user;
          char            *pass;
          char            *auth;
          
          char            *path;
          
          int              redirects;
          
          void            *data;
          
          bool             real_rtsp;
          bool             real_pack;
     } remote;
#endif

     DirectResult  (*wait)  ( DirectStream   *stream,
                              unsigned int    length,
                              struct timeval *tv );
     DirectResult  (*peek)  ( DirectStream *stream,
                              unsigned int  length,
                              int           offset,
                              void         *buf,
                              unsigned int *read_out );
     DirectResult  (*read)  ( DirectStream *stream,
                              unsigned int  length,
                              void         *buf, 
                              unsigned int *read_out );
     DirectResult  (*seek)  ( DirectStream *stream,
                              unsigned int  offset );
};


static void direct_stream_close( DirectStream *stream );


#if DIRECT_BUILD_NETWORK
D_DEBUG_DOMAIN( Direct_Stream, "Direct/Stream", "Stream wrapper" );
#endif

/************************** Begin Network Support ***************************/

#if DIRECT_BUILD_NETWORK

#define NET_TIMEOUT         15
#define HTTP_PORT           80
#define FTP_PORT            21
#define RTSP_PORT           554
#define HTTP_MAX_REDIRECTS  15

static DirectResult http_open( DirectStream *stream, const char *filename );
static DirectResult ftp_open ( DirectStream *stream, const char *filename );
static DirectResult rtsp_open( DirectStream *stream, const char *filename );


static inline char* trim( char *s )
{
     char *e;

#define space( c ) ((c) == ' '  || (c) == '\t' || \
                    (c) == '\r' || (c) == '\n' || \
                    (c) == '"'  || (c) == '\'')  
     
     for (; space(*s); s++);
     
     e = s + strlen(s) - 1;
     for (; e > s && space(*e); *e-- = '\0');
     
#undef space

     return s;
}

static void
parse_url( const char *url, char **ret_host, int *ret_port, 
           char **ret_user, char **ret_pass, char **ret_path )
{
     char *host = NULL;
     int   port = 0;
     char *user = NULL;
     char *pass = NULL;
     char *path;
     char *tmp;
     
     tmp = strchr( url, '/' );
     if (tmp) {
          host = alloca( tmp - url + 1 );
          memcpy( host, url, tmp - url );
          host[tmp-url] = '\0';
          path = tmp;
     } else {
          host = alloca( strlen( url ) + 1 );
          memcpy( host, url, strlen( url ) + 1 );
          path = "/";
     }

     tmp = strrchr( host, '@' );
     if (tmp) {
          *tmp = '\0';
          pass = strchr( host, ':' );
          if (pass) {
               *pass = '\0';
               pass++;
          }
          user = host;
          host = tmp + 1;
     }

     tmp = strchr( host, ':' );
     if (tmp) {
          port = strtol( tmp+1, NULL, 10 );
          *tmp = '\0';
     }

     /* IPv6 variant (host within brackets) */
     if (*host == '[') {
          host++;
          tmp = strchr( host, ']' );
          if (tmp)
               *tmp = '\0';
     }
     
     if (ret_host)
          *ret_host = D_STRDUP( host );
     
     if (ret_port && port)
          *ret_port = port;

     if (ret_user && user)
          *ret_user = D_STRDUP( user );

     if (ret_pass && pass)
          *ret_pass = D_STRDUP( pass );

     if (ret_path)
          *ret_path = D_STRDUP( path );
}

/*****************************************************************************/

static int
net_response( DirectStream *stream, char *buf, size_t size )
{
     fd_set         set;
     struct timeval tv;
     int            i;
     
     FD_ZERO( &set );
     FD_SET( stream->remote.sd, &set );

     for (i = 0; i < size-1; i++) {
          tv.tv_sec  = NET_TIMEOUT;
          tv.tv_usec = 0;
          select( stream->remote.sd+1, &set, NULL, NULL, &tv );
          
          if (recv( stream->remote.sd, buf+i, 1, 0 ) != 1)
               break;

          if (buf[i] == '\n') {
               if (i > 0 && buf[i-1] == '\r')
                    i--;
               break;
          }
     }

     buf[i] = '\0';
     
     D_DEBUG_AT( Direct_Stream, "got [%s].\n", buf );
     
     return i;
}

static int
net_command( DirectStream *stream, char *buf, size_t size )
{
     fd_set         s;
     struct timeval t;
     int            status;
     int            version;
     char           space;

     FD_ZERO( &s );
     FD_SET( stream->remote.sd, &s );

     t.tv_sec  = NET_TIMEOUT;
     t.tv_usec = 0;

     switch (select( stream->remote.sd+1, NULL, &s, NULL, &t )) {
          case 0:
               D_DEBUG_AT( Direct_Stream, "Timeout!\n" );
          case -1:
               return -1;
     }
     
     send( stream->remote.sd, buf, strlen(buf), 0 );
     send( stream->remote.sd, "\r\n", 2, 0 );
     
     D_DEBUG_AT( Direct_Stream, "sent [%s].\n", buf );

     while (net_response( stream, buf, size ) > 0) {
          status = 0;
          if (sscanf( buf, "HTTP/1.%d %3d", &version, &status ) == 2 ||
              sscanf( buf, "RTSP/1.%d %3d", &version, &status ) == 2 ||
              sscanf( buf, "ICY %3d", &status )                 == 1 ||
              sscanf( buf, "%3d%[ ]", &status, &space )         == 2)
               return status;
     }
     
     return 0;
}

static DirectResult
net_wait( DirectStream   *stream,
          unsigned int    length,
          struct timeval *tv )
{
     fd_set s;

     if (stream->cache_size >= length)
          return DR_OK;

     if (stream->fd == -1)
          return DR_EOF;
     
     FD_ZERO( &s );
     FD_SET( stream->fd, &s );
     
     switch (select( stream->fd+1, &s, NULL, NULL, tv )) {
          case 0:
               if (!tv && !stream->cache_size)
                    return DR_EOF;
               return DR_TIMEOUT;
          case -1:
               return errno2result( errno );
     }

     return DR_OK;
}

static DirectResult
net_peek( DirectStream *stream,
          unsigned int  length,
          int           offset,
          void         *buf,
          unsigned int *read_out )
{
     char *tmp;
     int   size;

     if (offset < 0)
          return DR_UNSUPPORTED;

     tmp = alloca( length + offset );
     
     size = recv( stream->fd, tmp, length+offset, MSG_PEEK );
     switch (size) {
          case 0:
               return DR_EOF;
          case -1:
               if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return DR_BUFFEREMPTY;
               return errno2result( errno );
          default:
               if (size < offset)
                    return DR_BUFFEREMPTY;
               size -= offset;
               break;
     }

     direct_memcpy( buf, tmp+offset, size );

     if (read_out)
          *read_out = size;

     return DR_OK;
}

static DirectResult
net_read( DirectStream *stream, 
          unsigned int  length,
          void         *buf,
          unsigned int *read_out )
{
     int size;
     
     size = recv( stream->fd, buf, length, 0 );
     switch (size) {
          case 0:
               return DR_EOF;
          case -1:
               if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return DR_BUFFEREMPTY;
               return errno2result( errno );
     }

     stream->offset += size;
     
     if (read_out)
          *read_out = size;

     return DR_OK;
}

static DirectResult
net_connect( struct addrinfo *addr, int sock, int proto, int *ret_fd )
{
     DirectResult     ret = DR_OK;
     int              fd  = -1;
     struct addrinfo *tmp;

     D_ASSERT( addr != NULL );
     D_ASSERT( ret_fd != NULL );

     for (tmp = addr; tmp; tmp = tmp->ai_next) {
          int err;
          
          fd = socket( tmp->ai_family, sock, proto );
          if (fd < 0) {
               ret = errno2result( errno );
               D_DEBUG_AT( Direct_Stream,
                           "failed to create socket!\n\t->%s",
                           strerror(errno) );
               continue;
          }

          fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK );

          D_DEBUG_AT( Direct_Stream, 
                      "connecting to %s...\n", tmp->ai_canonname );

          if (proto == IPPROTO_UDP)
               err = bind( fd, tmp->ai_addr, tmp->ai_addrlen );
          else
               err = connect( fd, tmp->ai_addr, tmp->ai_addrlen );

          if (err == 0 || errno == EINPROGRESS) {
               struct timeval t = { NET_TIMEOUT, 0 };
               fd_set         s;

               /* Join multicast group? */
               if (tmp->ai_addr->sa_family == AF_INET) {
                    struct sockaddr_in *saddr = (struct sockaddr_in *) tmp->ai_addr;

                    if (IN_MULTICAST( ntohl(saddr->sin_addr.s_addr) )) {
                         struct ip_mreq req;

                         D_DEBUG_AT( Direct_Stream, 
                                     "joining multicast group (%u.%u.%u.%u)...\n",
                                     (u8)tmp->ai_addr->sa_data[2], (u8)tmp->ai_addr->sa_data[3],
                                     (u8)tmp->ai_addr->sa_data[4], (u8)tmp->ai_addr->sa_data[5] );

                         req.imr_multiaddr.s_addr = saddr->sin_addr.s_addr;
                         req.imr_interface.s_addr = 0;

                         err = setsockopt( fd, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req) );
                         if (err < 0) {
                              ret = errno2result( errno );
                              D_PERROR( "Direct/Stream: Could not join multicast group (%u.%u.%u.%u)!\n",
                                        (u8)tmp->ai_addr->sa_data[2], (u8)tmp->ai_addr->sa_data[3],
                                        (u8)tmp->ai_addr->sa_data[4], (u8)tmp->ai_addr->sa_data[5] );
                              close( fd );
                              continue;
                         }

                         setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, saddr, sizeof(*saddr) );
                    }
               }

               FD_ZERO( &s );
               FD_SET( fd, &s );
              
               err = select( fd+1, NULL, &s, NULL, &t );
               if (err < 1) {
                    D_DEBUG_AT( Direct_Stream, "...connection failed.\n" );
                    
                    close( fd );
                    fd = -1;

                    if (err == 0) {
                         ret = DR_TIMEOUT;
                         continue;
                    } else {
                         ret = errno2result( errno );
                         break;
                    }
               }

               D_DEBUG_AT( Direct_Stream, "...connected.\n" );
               
               ret = DR_OK;
               break;
          }
     }

     *ret_fd = fd;

     return ret;
}

static DirectResult
net_open( DirectStream *stream, const char *filename, int proto )
{
     DirectResult    ret  = DR_OK;
     int             sock = (proto == IPPROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;
     struct addrinfo hints;
     char            port[16];
     
     parse_url( filename, 
                &stream->remote.host,
                &stream->remote.port,
                &stream->remote.user,
                &stream->remote.pass,
                &stream->remote.path );
     
     snprintf( port, sizeof(port), "%d", stream->remote.port );

     memset( &hints, 0, sizeof(hints) );
     hints.ai_flags    = AI_CANONNAME;
     hints.ai_socktype = sock;
     hints.ai_family   = PF_UNSPEC;
     
     if (getaddrinfo( stream->remote.host, port,
                      &hints, &stream->remote.addr )) {
          D_ERROR( "Direct/Stream: "
                   "failed to resolve host '%s'!\n", stream->remote.host );
          return DR_FAILURE;
     }

     ret = net_connect( stream->remote.addr, sock, proto, &stream->remote.sd );
     if (ret)
          return ret;

     stream->fd     = stream->remote.sd;
     stream->length = -1; 
     stream->wait   = net_wait;
     stream->peek   = net_peek;
     stream->read   = net_read;

     return ret;
}

/*****************************************************************************/

static DirectResult
http_seek( DirectStream *stream, unsigned int offset )
{
     DirectResult ret;
     char         buf[1280];
     int          status, len;
     
     close( stream->remote.sd );
     stream->remote.sd = -1;

     ret = net_connect( stream->remote.addr, 
                        SOCK_STREAM, IPPROTO_TCP, &stream->remote.sd );
     if (ret)
          return ret;
     
     stream->fd = stream->remote.sd;
 
     len = snprintf( buf, sizeof(buf),
                     "GET %s HTTP/1.0\r\n"
                     "Host: %s:%d\r\n",
                     stream->remote.path,
                     stream->remote.host,
                     stream->remote.port );
     if (stream->remote.auth) {
          len += snprintf( buf+len, sizeof(buf)-len,
                           "Authorization: Basic %s\r\n",
                           stream->remote.auth );
     }
     snprintf( buf+len, sizeof(buf)-len,
               "User-Agent: DirectFB/%s\r\n"
               "Accept: */*\r\n"
               "Range: bytes=%d-\r\n"
               "Connection: Close\r\n",
               DIRECTFB_VERSION, offset );
     
     status = net_command( stream, buf, sizeof(buf) );
     switch (status) {
          case 200 ... 299:
               stream->offset = offset;
               break;
          default:
               if (status)
                    D_DEBUG_AT( Direct_Stream,
                                "server returned status %d.\n", status );
               return DR_FAILURE;
     }

     /* discard remaining headers */
     while (net_response( stream, buf, sizeof(buf) ) > 0);

     return DR_OK;
}    

static DirectResult
http_open( DirectStream *stream, const char *filename )
{
     DirectResult ret;
     char         buf[1280];
     int          status, len;
     
     stream->remote.port = HTTP_PORT;

     ret = net_open( stream, filename, IPPROTO_TCP );
     if (ret)
          return ret;

     if (stream->remote.user) {
          char *tmp;

          if (stream->remote.pass) {
               tmp = alloca( strlen( stream->remote.user ) +
                             strlen( stream->remote.pass ) + 2 );
               len = sprintf( tmp, "%s:%s",
                              stream->remote.user, stream->remote.pass );
          } else {
               tmp = alloca( strlen( stream->remote.user ) + 2 );
               len = sprintf( tmp, "%s:", stream->remote.user );
          }

          stream->remote.auth = direct_base64_encode( tmp, len );
     }

     len = snprintf( buf, sizeof(buf),
                     "GET %s HTTP/1.0\r\n"
                     "Host: %s:%d\r\n",
                     stream->remote.path,
                     stream->remote.host,
                     stream->remote.port );
     if (stream->remote.auth) {
          len += snprintf( buf+len, sizeof(buf)-len,
                           "Authorization: Basic %s\r\n",
                           stream->remote.auth );
     }
     snprintf( buf+len, sizeof(buf)-len,
               "User-Agent: DirectFB/%s\r\n"
               "Accept: */*\r\n"
               "Connection: Close\r\n",
               DIRECTFB_VERSION );
     
     status = net_command( stream, buf, sizeof(buf) );

     while (net_response( stream, buf, sizeof(buf) ) > 0) {
          if (!strncasecmp( buf, "Accept-Ranges:", 14 )) {
               if (strcmp( trim( buf+14 ), "none" ))
                    stream->seek = http_seek;
          }
          else if (!strncasecmp( buf, "Content-Type:", 13 )) {
               char *mime = trim( buf+13 );
               char *tmp  = strchr( mime, ';' );
               if (tmp)
                    *tmp = '\0';
               if (stream->mime)
                    D_FREE( stream->mime );
               stream->mime = D_STRDUP( mime );
          }
          else if (!strncasecmp( buf, "Content-Length:", 15 )) {
               char *tmp = trim( buf+15 );
               if (sscanf( tmp, "%d", &stream->length ) < 1)
                    sscanf( tmp, "bytes=%d", &stream->length );
          }
          else if (!strncasecmp( buf, "Location:", 9 )) { 
               direct_stream_close( stream );
               stream->seek = NULL;
               
               if (++stream->remote.redirects > HTTP_MAX_REDIRECTS) {
                    D_ERROR( "Direct/Stream: "
                             "reached maximum number of redirects (%d).\n",
                             HTTP_MAX_REDIRECTS );
                    return DR_LIMITEXCEEDED;
               }
               
               filename = trim( buf+9 );
               if (!strncmp( filename, "http://", 7 ))
                    return http_open( stream, filename+7 );
               if (!strncmp( filename, "ftp://", 6 ))
                    return ftp_open( stream, filename+6 );
               if (!strncmp( filename, "rtsp://", 7 ))
                    return rtsp_open( stream, filename+7 );
               
               return DR_UNSUPPORTED;
          }
     }

     switch (status) {
          case 200 ... 299:
               break;
          default:
               if (status)
                    D_DEBUG_AT( Direct_Stream,
                                "server returned status %d.\n", status );
               return (status == 404) ? DR_FILENOTFOUND : DR_FAILURE;
     }

     return DR_OK;
}

/*****************************************************************************/

static DirectResult
ftp_open_pasv( DirectStream *stream, char *buf, size_t size )
{
     DirectResult ret;
     int          i, len;

     snprintf( buf, size, "PASV" );
     if (net_command( stream, buf, size ) != 227)
          return DR_FAILURE;

     /* parse IP and port for passive mode */
     for (i = 4; buf[i]; i++) {
          unsigned int d[6];
     
          if (sscanf( &buf[i], "%u,%u,%u,%u,%u,%u",
                      &d[0], &d[1], &d[2], &d[3], &d[4], &d[5] ) == 6)
          {
               struct addrinfo hints, *addr;
                    
               /* address */
               len = snprintf( buf, size, 
                               "%u.%u.%u.%u",
                               d[0], d[1], d[2], d[3] );
               /* port */
               snprintf( buf+len+1, size-len-1,
                         "%u", ((d[4] & 0xff) << 8) | (d[5] & 0xff) );

               memset( &hints, 0, sizeof(hints) );
               hints.ai_flags    = AI_CANONNAME;
               hints.ai_socktype = SOCK_STREAM;
               hints.ai_family   = PF_UNSPEC;

               if (getaddrinfo( buf, buf+len+1, &hints, &addr )) {
                    D_DEBUG_AT( Direct_Stream, 
                                "failed to resolve host '%s'.\n", buf );
                    return DR_FAILURE;
               }
                    
               ret = net_connect( addr, SOCK_STREAM, IPPROTO_TCP, &stream->fd );
               
               freeaddrinfo( addr );
               
               return ret;
          }
     }

     return DR_FAILURE;
}

static DirectResult
ftp_seek( DirectStream *stream, unsigned int offset )
{
     DirectResult ret = DR_OK;
     char         buf[512];
    
     if (stream->fd > 0) {
          int status;
          
          close( stream->fd );
          stream->fd = -1;

          /* ignore response */
          while (net_response( stream, buf, sizeof(buf) ) > 0) {
               if (sscanf( buf, "%3d%[ ]", &status, buf ) == 2)
                    break;
          }
     }

     ret = ftp_open_pasv( stream, buf, sizeof(buf) );
     if (ret)
          return ret;
     
     snprintf( buf, sizeof(buf), "REST %d", offset );
     if (net_command( stream, buf, sizeof(buf) ) != 350)
          goto error;

     snprintf( buf, sizeof(buf), "RETR %s", stream->remote.path );
     switch (net_command( stream, buf, sizeof(buf) )) {
          case 150:
          case 125:
               break;
          default:
               goto error;
     }

     stream->offset = offset;

     return DR_OK;

error:
     close( stream->fd );
     stream->fd = -1;
     
     return DR_FAILURE;
}

static DirectResult
ftp_open( DirectStream *stream, const char *filename )
{
     DirectResult ret;
     int          status = 0;
     char         buf[512];
     
     stream->remote.port = FTP_PORT;

     ret = net_open( stream, filename, IPPROTO_TCP );
     if (ret)
          return ret;

     while (net_response( stream, buf, sizeof(buf) ) > 0) {
          if (sscanf( buf, "%3d%[ ]", &status, buf ) == 2)
               break;
     }
     if (status != 220)
          return DR_FAILURE;
     
     /* login */
     snprintf( buf, sizeof(buf), "USER %s", stream->remote.user ? : "anonymous" );
     switch (net_command( stream, buf, sizeof(buf) )) {
          case 230:
          case 331:
               break;
          default:
               return DR_FAILURE;
     }

     if (stream->remote.pass) {
          snprintf( buf, sizeof(buf), "PASS %s", stream->remote.pass );
          if (net_command( stream, buf, sizeof(buf) ) != 230)
               return DR_FAILURE;
     }
     
     /* enter binary mode */
     snprintf( buf, sizeof(buf), "TYPE I" );
     if (net_command( stream, buf, sizeof(buf) ) != 200)
          return DR_FAILURE;

     /* get file size */
     snprintf( buf, sizeof(buf), "SIZE %s", stream->remote.path );
     if (net_command( stream, buf, sizeof(buf) ) == 213)
          stream->length = strtol( buf+4, NULL, 10 );

     /* enter passive mode by default */
     ret = ftp_open_pasv( stream, buf, sizeof(buf) );
     if (ret)
          return ret;

     /* retrieve file */
     snprintf( buf, sizeof(buf), "RETR %s", stream->remote.path );
     switch (net_command( stream, buf, sizeof(buf) )) {
          case 125:
          case 150:
               break;
          default:
               return DR_FAILURE;
     }

     stream->seek = ftp_seek;
     
     return DR_OK;
}

/*****************************************************************************/

typedef struct {
     s8          pt;   // payload type (-1: dymanic)
     u8          type; // codec type
     const char *name;
     const char *mime;
     u32         fcc;
} RTPPayload;

typedef struct {
     char              *control;
     
     int                pt;
     const RTPPayload  *payload;
     
     int                dur; // duration
     int                abr; // avg bitrate
     int                mbr; // max bitrate
     int                aps; // avg packet size
     int                mps; // max packet size
     int                str; // start time
     int                prl; // preroll
     
     char              *mime;
     int                mime_size;
     
     void              *data;
     int                data_size;
} SDPStreamDesc;

#define PAYLOAD_VIDEO 1
#define PAYLOAD_AUDIO 2

#define FCC( a, b, c, d ) ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))

static const RTPPayload payloads[] = {
     { 31, PAYLOAD_VIDEO, "H261",          "video/h261",   FCC('H','2','6','1') },
     { 34, PAYLOAD_VIDEO, "H263",          "video/h263",   FCC('H','2','6','3') },
   //{ -1, PAYLOAD_VIDEO, "H264",          "video/h264",   FCC('H','2','6','4') },
     { 32, PAYLOAD_VIDEO, "MPV",           "video/mpeg",   FCC('M','P','E','G') },
     { 33,             0, "MP2T",          "video/mpegts", 0 },
     { -1, PAYLOAD_VIDEO, "MP4V-ES",       "video/mpeg4",  FCC('M','P','4','S') },
     { 14, PAYLOAD_AUDIO, "MPA",           "audio/mpeg",   0x0055 },
   //{ -1, PAYLOAD_AUDIO, "mpeg4-generic", "audio/aac",    0x00ff },
};

#define NUM_PAYLOADS D_ARRAY_SIZE( payloads )

static DirectResult
sdp_parse( DirectStream *stream, int length, SDPStreamDesc **ret_streams, int *ret_num )
{
     char          *buf, *tmp;
     SDPStreamDesc *desc = NULL;
     int            num  = 0;
     fd_set         set;
     struct timeval tv;
     int            i;

     buf = D_CALLOC( 1, length+1 );
     if (!buf)
          return D_OOM();
          
     FD_ZERO( &set );
     FD_SET( stream->remote.sd, &set );
     
     for (tmp = buf; length;) {
          int size;
          
          tv.tv_sec  = NET_TIMEOUT;
          tv.tv_usec = 0;
          select( stream->remote.sd+1, &set, NULL, NULL, &tv );
          
          size = recv( stream->remote.sd, tmp, length, MSG_WAITALL );
          if (size < 1)
               break;
          tmp += size;
          length -= size;
     }
     
     for (tmp = buf; tmp && *tmp;) {
          char *end;
          
          end = strchr( tmp, '\n' );
          if (end) {
               if (end > tmp && *(end-1) == '\r')
                    *(end-1) = '\0';
               *end = '\0';
          }
          
          D_DEBUG_AT( Direct_Stream, "SDP [%s]\n", tmp );
          
          switch (*tmp) {
               case 'm':
                    /* media */
                    if (*(tmp+1) == '=') {                   
                         desc = D_REALLOC( desc, ++num*sizeof(SDPStreamDesc) );
                         memset( &desc[num-1], 0, sizeof(SDPStreamDesc) );
                         
                         tmp += 2;
                         if (sscanf( tmp, "audio %d RTP/AVP %d", &i, &desc[num-1].pt ) == 2 ||
                             sscanf( tmp, "video %d RTP/AVP %d", &i, &desc[num-1].pt ) == 2) {
                              for (i = 0; i < NUM_PAYLOADS; i++) {
                                   if (desc[num-1].pt == payloads[i].pt) {
                                        desc[num-1].payload = &payloads[i];
                                        desc[num-1].mime = D_STRDUP( payloads[i].mime );
                                        desc[num-1].mime_size = strlen( payloads[i].mime );
                                        break;
                                   }
                              }
                         }   
                    }
                    break;
               case 'a':
                    /* attribute */
                    if (*(tmp+1) == '=' && num) {
                         tmp += 2;
                         if (!strncmp( tmp, "control:", 8 )) {
                              desc[num-1].control = D_STRDUP( trim( tmp+8 ) );
                         }
                         else if (!strncmp( tmp, "rtpmap:", 7 )) {
                              if (!desc[num-1].payload && desc[num-1].pt) {
                                   char *sep;
                                   
                                   tmp = strchr( trim( tmp+7 ), ' ' );
                                   if (!tmp) break;
                                   sep = strchr( ++tmp, '/' );
                                   if (sep) *sep = '\0';
                                   
                                   for (i = 0; i < NUM_PAYLOADS; i++) {
                                        if (strcmp( tmp, payloads[i].name ))
                                             continue;
                                        desc[num-1].payload = &payloads[i];
                                        desc[num-1].mime = D_STRDUP( payloads[i].mime );
                                        desc[num-1].mime_size = strlen( payloads[i].mime );
                                        break;
                                   }
                              }
                         }                                   
                         else if (!strncmp( tmp, "length:npt=", 11 )) {
                              double val = atof( tmp+11 );
                              desc[num-1].dur = val * 1000.0;
                         }
                         else if (!strncmp( tmp, "mimetype:string;", 16 )) {
                              if (desc[num-1].mime)
                                   D_FREE( desc[num-1].mime );
                              desc[num-1].mime = D_STRDUP( trim( tmp+16 ) );
                              desc[num-1].mime_size = strlen( desc[num-1].mime );
                         }                               
                         else if (!strncmp( tmp, "AvgBitRate:", 11 )) {
                              sscanf( tmp+11, "integer;%d", &desc[num-1].abr );
                         }
                         else if (!strncmp( tmp, "MaxBitRate:", 11 )) {
                              sscanf( tmp+11, "integer;%d", &desc[num-1].mbr );
                         }
                         else if (!strncmp( tmp, "AvgPacketSize:", 14 )) {
                              sscanf( tmp+14, "integer;%d", &desc[num-1].aps );
                         }
                         else if (!strncmp( tmp, "MaxPacketSize:", 14 )) {
                              sscanf( tmp+14, "integer;%d", &desc[num-1].mps );
                         }
                         else if (!strncmp( tmp, "StartTime:", 10 )) {
                              sscanf( tmp+10, "integer;%d", &desc[num-1].str );
                         }
                         else if (!strncmp( tmp, "Preroll:", 8 )) {
                              sscanf( tmp+8, "integer;%d", &desc[num-1].prl );
                         }
                         else if (!strncmp( tmp, "OpaqueData:buffer;", 18 )) {
                              desc[num-1].data = 
                                   direct_base64_decode( trim( tmp+18 ),
                                                         &desc[num-1].data_size );
                         }
                    }
                    break;
               default:
                    break;
          }
          
          tmp = end;
          if (tmp) tmp++;
     }
     
     D_FREE( buf );                   
     
     *ret_streams = desc;
     *ret_num     = num;

     return desc ? DR_OK : DR_EOF;
}

static void
sdp_free( SDPStreamDesc *streams, int num )
{
     int i;
     
     for (i = 0; i < num; i++) {
          if (streams[i].control)
               D_FREE( streams[i].control );
          if (streams[i].mime)
               D_FREE( streams[i].mime );
          if (streams[i].data)
               D_FREE( streams[i].data );
     }
     D_FREE( streams );
}

static DirectResult
rmf_write_header( SDPStreamDesc *streams, int n_streams, void **ret_buf, unsigned int *ret_size )
{
     unsigned char *dst, *tmp;
     int            abr = 0;
     int            mbr = 0;
     int            aps = 0;
     int            mps = 0;
     int            str = 0;
     int            prl = 0;
     int            dur = 0;
     int            i, len;
          
     len = 86 + n_streams*46;
     for (i = 0; i < n_streams; i++) {
          abr += streams[i].abr;
          aps += streams[i].aps;
          if (mbr < streams[i].mbr)
               mbr = streams[i].mbr;
          if (mps < streams[i].mps)
               mps = streams[i].mps;
          if (dur < streams[i].dur)
               dur = streams[i].dur;
          if (streams[i].mime)
               len += streams[i].mime_size;
          if (streams[i].data)
               len += streams[i].data_size;
          else if (streams[i].payload)
               len += 74;
     }
          
     *ret_buf = dst = D_MALLOC( len );
     if (!dst)
          return D_OOM();
     *ret_size = len;
          
     /* RMF */
     dst[0] = '.'; dst[1] = 'R', dst[2] = 'M'; dst[3] = 'F';
     dst[4] = dst[5] = dst[6] = 0; dst[7] = 18;              // size
     dst[8] = dst[9] = 0;                                    // version
     dst[10] = dst[11] = dst[12] = dst[13] = 0;              // ??
     dst[14] = dst[15] = dst[16] = 0; dst[17] = 4+n_streams; // num streams
     dst += 18;
          
     /* PROP */
     dst[0] = 'P'; dst[1] = 'R'; dst[2] = 'O'; dst[3] = 'P';
     dst[4] = dst[5] = dst[6] = 0; dst[7] = 50;
     dst[8] = dst[9] = 0;
     dst[10] = mbr>>24; dst[11] = mbr>>16; dst[12] = mbr>>8; dst[13] = mbr;
     dst[14] = abr>>24; dst[15] = abr>>16; dst[16] = abr>>8; dst[17] = abr;
     dst[18] = mps>>24; dst[19] = mps>>16; dst[20] = mps>>8; dst[21] = mps;
     dst[22] = aps>>24; dst[23] = aps>>16; dst[24] = aps>>8; dst[25] = aps;
     dst[26] = dst[27] = dst[28] = dst[29] = 0; // num packets
     dst[30] = dur>>24; dst[31] = dur>>16; dst[32] = dur>>8; dst[33] = dur;
     dst[34] = dst[35] = dst[36] = dst[37] = 0; // preroll
     dst[38] = dst[39] = dst[40] = dst[41] = 0; // index offset
     dst[42] = len>>24; dst[43] = len>>16; dst[44] = len>>8; dst[45] = len;
     dst[46] = 0; dst[47] = n_streams;          // num streams
     dst[48] = 0; dst[49] = 7;                  // flags
     dst += 50;
          
     for (i = 0; i < n_streams; i++) {
          len = 46 + streams[i].mime_size;
          if (streams[i].data)
               len += streams[i].data_size;
          else if (streams[i].payload)
               len += 74;
               
          abr = streams[i].abr;
          mbr = streams[i].mbr;
          aps = streams[i].aps;
          mps = streams[i].mps;
          str = streams[i].str;
          prl = streams[i].prl;
          dur = streams[i].dur;
               
          /* MDPR */
          dst[0] = 'M'; dst[1] = 'D'; dst[2] = 'P'; dst[3] = 'R';
          dst[4] = len>>24; dst[5] = len>>16; dst[6] = len>>8; dst[7] = len;
          dst[8] = dst[9] = 0;
          dst[10] = 0; dst[11] = i;
          dst[12] = mbr>>24; dst[13] = mbr>>16; dst[14] = mbr>>8; dst[15] = mbr;
          dst[16] = abr>>24; dst[17] = abr>>16; dst[18] = abr>>8; dst[19] = abr;
          dst[20] = mps>>24; dst[21] = mps>>16; dst[22] = mps>>8; dst[23] = mps;
          dst[24] = aps>>24; dst[25] = aps>>16; dst[26] = aps>>8; dst[27] = aps;
          dst[28] = str>>24; dst[29] = str>>16; dst[30] = str>>8; dst[31] = str;
          dst[32] = prl>>24; dst[33] = prl>>16; dst[34] = prl>>8; dst[35] = prl;
          dst[36] = dur>>24; dst[37] = dur>>16; dst[38] = dur>>8; dst[39] = dur;
          dst += 40;
               
          /* description */
          *dst++ = 0;
          /* mimetype */
          *dst++ = streams[i].mime_size;
          for (tmp = (unsigned char*)streams[i].mime; tmp && *tmp;)
               *dst++ = *tmp++;
               
          /* codec data */
          if (streams[i].data) {
               len = streams[i].data_size;
               dst[0] = len>>24; dst[1] = len>>16; dst[2] = len>>8; dst[3] = len;
               direct_memcpy( dst+4, streams[i].data, streams[i].data_size );
               dst += len+4;
          }
          else if (streams[i].payload) {
               u32 fcc = streams[i].payload->fcc;
               
               dst[0] = dst[1] = dst[2] = 0; dst[3] = 74;
               dst += 4;
               memset( dst, 0, 74 );
               
               if (streams[i].payload->type == PAYLOAD_AUDIO) {
                    dst[0] = '.'; dst[1] = 'r'; dst[2] = 'a'; dst[3] = 0xfd;
                    dst[4] = 0; dst[5] = 5; // version
                    dst[66] = fcc; dst[67] = fcc>>8; dst[68] = fcc>>16; dst[69] = fcc>>24;
               }
               else {
                    dst[0] = dst[1] = dst[2] = 0; dst[3] = 34;
                    dst[4] = 'V'; dst[5] = 'I'; dst[6] = 'D'; dst[7] = 'O';
                    dst[8] = fcc; dst[9] = fcc>>8; dst[10] = fcc>>16; dst[11] = fcc>>24;
                    dst[30] = 0x10;
               }
               dst += 74;
          }
          else {
               dst[0] = dst[1] = dst[2] = dst[3] = 0;
               dst += 4;
          }
     }
          
     /* DATA */
     dst[0] = 'D'; dst[1] = 'A'; dst[2] = 'T'; dst[3] = 'A';
     dst[4] = dst[5] = dst[6] = 0; dst[7] = 18; // size
     dst[8] = dst[9] = 0;                       // version
     dst[10] = dst[11] = dst[12] = dst[13] = 0; // num packets
     dst[14] = dst[15] = dst[16] = dst[17] = 0; // next data 
     
     return DR_OK;
}

static int
rmf_write_pheader( unsigned char *dst, int id, int sz, unsigned int ts )
{
     /* version */
     dst[0] = dst[1] = 0;
     /* length */
     dst[2] = (sz+12)>>8; dst[3] = sz+12;
     /* indentifier */
     dst[4] = id>>8; dst[5] = id;
     /* timestamp */
     dst[6] = ts>>24; dst[7] = ts>>16; dst[8] = ts>>8; dst[9] = ts;
     /* reserved */
     dst[10] = 0;
     /* flags */
     dst[11] = 0;

     return 12;
}

static void
real_calc_challenge2( char response[64], char checksum[32], char *challenge )
{
     const unsigned char xor_table[37] = {
          0x05, 0x18, 0x74, 0xd0, 0x0d, 0x09, 0x02, 0x53,
          0xc0, 0x01, 0x05, 0x05, 0x67, 0x03, 0x19, 0x70,
          0x08, 0x27, 0x66, 0x10, 0x10, 0x72, 0x08, 0x09,
          0x63, 0x11, 0x03, 0x71, 0x08, 0x08, 0x70, 0x02,
          0x10, 0x57, 0x05, 0x18, 0x54
     };
     char buf[128];
     char md5[16];
     int  len;
     int  i;
     
     memset( response, 0, 64 );
     memset( checksum, 0, 32 );
     
     buf[0] = 0xa1; buf[1] = 0xe9; buf[2] = 0x14; buf[3] = 0x9d;
     buf[4] = 0x0e; buf[5] = 0x6b; buf[6] = 0x3b; buf[7] = 0x59;
     memset( buf+8, 0, 120 );
     
     len = strlen( challenge );
     if (len == 40) {
          challenge[32] = '\0';
          len = 32;
     }
     memcpy( buf+8, challenge, MAX(len,56) );
     
     for (i = 0; i < 37; i++)
          buf[8+i] ^= xor_table[i];
      
     /* compute response */    
     direct_md5_sum( md5, buf, 64 );
     /* convert to ascii */
     for (i = 0; i < 16; i++) {
          char a, b;
          a = (md5[i] >> 4) & 15;
          b =  md5[i]       & 15;
          response[i*2+0] = ((a < 10) ? (a + 48) : (a + 87)) & 255;
          response[i*2+1] = ((b < 10) ? (b + 48) : (b + 87)) & 255;
     }
     /* tail */
     len = strlen( response );
     direct_snputs( &response[len], "01d0a8e3", 64-len );
     
     /* compute checksum */
     for (i = 0; i < len/4; i++)
          checksum[i] = response[i*4];
}

static DirectResult
rtsp_session_open( DirectStream *stream )
{
     DirectResult   ret;
     int            status;
     int            cseq        = 0;
     SDPStreamDesc *streams     = NULL;
     int            n_streams   = 0;
     char           session[32] = {0, };
     char           challen[64] = {0, };
     char           buf[1280];
     int            i, len;
     
     snprintf( buf, sizeof(buf),
               "OPTIONS rtsp://%s:%d RTSP/1.0\r\n"
               "CSeq: %d\r\n"
               "User-Agent: DirectFB/%s\r\n"
               "ClientChallenge: 9e26d33f2984236010ef6253fb1887f7\r\n"
               "PlayerStarttime: [28/03/2003:22:50:23 00:00]\r\n"
               "CompanyID: KnKV4M4I/B2FjJ1TToLycw==\r\n"
               "GUID: 00000000-0000-0000-0000-000000000000\r\n"
               "RegionData: 0\r\n",
               stream->remote.host,
               stream->remote.port,
               ++cseq, DIRECTFB_VERSION );
     
     if (net_command( stream, buf, sizeof(buf) ) != 200)
          return DR_FAILURE;
     
     while (net_response( stream, buf, sizeof(buf) ) > 0) {
          if (!strncmp( buf, "RealChallenge1:", 15 )) {
               snprintf( challen, sizeof(challen), "%s", trim( buf+15 ) );
               stream->remote.real_rtsp = true;
          }
     }

     len = snprintf( buf, sizeof(buf),
                     "DESCRIBE rtsp://%s:%d%s RTSP/1.0\r\n"
                     "CSeq: %d\r\n"
                     "Accept: application/sdp\r\n"
                     "Bandwidth: 10485800\r\n",
                     stream->remote.host,
                     stream->remote.port,
                     stream->remote.path,
                     ++cseq );
     if (stream->remote.real_rtsp) {
          snprintf( buf+len, sizeof(buf)-len,
                    "GUID: 00000000-0000-0000-0000-000000000000\r\n"
                    "RegionData: 0\r\n"
                    "ClientID: Linux_2.4_6.0.9.1235_play32_RN01_EN_586\r\n"
                    "SupportsMaximumASMBandwidth: 1\r\n"
                    "Require: com.real.retain-entity-for-setup\r\n" );
     }
     
     status = net_command( stream, buf, sizeof(buf) );
     if (status != 200)
          return (status == 404) ? DR_FILENOTFOUND : DR_FAILURE;
     
     len = 0;
     while (net_response( stream, buf, sizeof(buf) ) > 0) {
          if (!strncasecmp( buf, "ETag:", 5 )) {
               snprintf( session, sizeof(session), "%s", trim( buf+5 ) );
          }
          else if (!strncasecmp( buf, "Content-Length:", 15 )) {
               char *tmp = trim( buf+15 );
               if (sscanf( tmp, "%d", &len ) != 1)
                    sscanf( tmp, "bytes=%d", &len );
          }
     }
     
     if (!len) {
          D_DEBUG_AT( Direct_Stream, "Couldn't get sdp length!\n" );
          return DR_FAILURE;
     }       
         
     ret = sdp_parse( stream, len, &streams, &n_streams );
     if (ret)
          return ret;
     
     for (i = 0; i < n_streams; i++) {
          /* skip unhandled payload types */
          if (!stream->remote.real_rtsp && !streams[i].payload)
               continue;
               
          len = snprintf( buf, sizeof(buf),
                         "SETUP rtsp://%s:%d%s/%s RTSP/1.0\r\n"
                         "CSeq: %d\r\n",
                         stream->remote.host,
                         stream->remote.port,
                         stream->remote.path,
                         streams[i].control, ++cseq );
          if (*session) {
               if (*challen) {
                    char response[64];
                    char checksum[32];
                    
                    real_calc_challenge2( response, checksum, challen );
                    len += snprintf( buf+len, sizeof(buf)-len,
                                     "RealChallenge2: %s, sd=%s\r\n",
                                     response, checksum );
                    *challen = '\0';
               }
               len += snprintf( buf+len, sizeof(buf)-len,
                                "%s: %s\r\n",
                                i ? "Session" : "If-Match", session );
          }
          snprintf( buf+len, sizeof(buf)-len,
                    "Transport: %s\r\n",
                    stream->remote.real_rtsp
                    ? "x-pn-tng/tcp;mode=play,rtp/avp/tcp;unicast;mode=play"
                    : "RTP/AVP/TCP;unicast" );

          if (net_command( stream, buf, sizeof(buf) ) != 200) {
               sdp_free( streams, n_streams );
               return DR_FAILURE;
          }
          
          while (net_response( stream, buf, sizeof(buf) ) > 0) {
               if (!strncmp( buf, "Session:", 8 ))
                    snprintf( session, sizeof(session), "%s", trim( buf+8 ) );
          }
     }
     
     len = snprintf( buf, sizeof(buf),
                    "PLAY rtsp://%s:%d%s RTSP/1.0\r\n"
                    "CSeq: %d\r\n",
                    stream->remote.host,
                    stream->remote.port,
                    stream->remote.path,
                    ++cseq );
     if (*session) {
          len += snprintf( buf+len, sizeof(buf)-len,
                           "Session: %s\r\n", session );
     }
     snprintf( buf+len, sizeof(buf)-len, 
               "Range: npt=0-\r\n"
               "Connection: Close\r\n" );
     
     if (net_command( stream, buf, sizeof(buf) ) != 200) {
          sdp_free( streams, n_streams );
          return DR_FAILURE;
     }
     
     /* discard remaining headers */
     while (net_response( stream, buf, sizeof(buf) ) > 0);
     
     /* revert to blocking mode */
     fcntl( stream->fd, F_SETFL, 
            fcntl( stream->fd, F_GETFL ) & ~O_NONBLOCK );
            
     if (!stream->remote.real_rtsp) {
          RTPPayload *p;
          
          stream->remote.data = D_CALLOC( 1, (n_streams+1)*sizeof(RTPPayload) );
          if (!stream->remote.data) {
               sdp_free( streams, n_streams );
               return D_OOM();
          }
          
          p = (RTPPayload*) stream->remote.data;
          for (i = 0; i < n_streams; i++) {
               if (streams[i].payload) {
                    *p = *(streams[i].payload);
                    p->pt = streams[i].pt;
                    p++;
               }
          }  
     }
     
     stream->remote.real_pack = true;
     if (n_streams == 1 && streams[0].mime) {
          if (!strcmp( streams[0].mime, "audio/mpeg" ) ||
              !strcmp( streams[0].mime, "audio/aac" )  ||
              !strcmp( streams[0].mime, "video/mpeg" ) ||
              !strcmp( streams[0].mime, "video/mpegts" ))
          {
               stream->mime = D_STRDUP( streams[0].mime );
               stream->remote.real_pack = false;
          }
     }
     if (stream->remote.real_pack) {
          ret = rmf_write_header( streams, n_streams,
                                 &stream->cache, &stream->cache_size );
          if (ret) {
               sdp_free( streams, n_streams );
               return ret;
          }
          stream->mime = D_STRDUP( "application/vnd.rn-realmedia" );
     }         

     sdp_free( streams, n_streams );
     
     return DR_OK;
}

static DirectResult
rvp_read_packet( DirectStream *stream )
{
     unsigned char  buf[9];
     int            size;
     int            len;
     unsigned char  id;
     unsigned int   ts;
     
     while (1) {
          do {
               size = recv( stream->fd, buf, 1, MSG_WAITALL );
               if (size < 1)
                    return DR_EOF;
          } while (buf[0] != '$');
     
          size = recv( stream->fd, buf, 7, MSG_WAITALL );
          if (size < 7)
               return DR_EOF;
          
          len = (buf[0] << 16) + (buf[1] << 8) + buf[2]; 
          id = buf[3];
          if (id != 0x40 && id != 0x42) {
               if (buf[5] == 0x06) // EOS
                    return DR_EOF;
               size = recv( stream->fd, buf, 9, MSG_WAITALL );
               if (size < 9)
                    return DR_EOF;
               id = buf[5];
               len -= 9;
          }
          id = (id >> 1) & 1;
          
          size = recv( stream->fd, buf, 6, MSG_WAITALL );
          if (size < 6)
               return DR_EOF;
          ts = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
               
          len -= 10;
          if (len > 0) {
               unsigned char *dst;
               
               size = len + 12;   
               stream->cache = D_REALLOC( stream->cache, stream->cache_size+size );
               if (!stream->cache)
                    return D_OOM();
               dst = stream->cache+stream->cache_size;
               stream->cache_size += size;
               
               dst += rmf_write_pheader( dst, id, len, ts ); 
               
               while (len) {
                    size = recv( stream->fd, dst, len, MSG_WAITALL );
                    if (size < 1)
                         return DR_EOF;
                    dst += size;
                    len -= size;
               }
               break;
          }
     }
     
     return DR_OK;
}

static DirectResult
rtp_read_packet( DirectStream *stream )
{
     RTPPayload    *payloads = (RTPPayload*)stream->remote.data;
     unsigned char  buf[12];
     int            size;
     int            len;
     unsigned char  id;
     unsigned short seq;
     unsigned int   ts;
     int            skip;
     
     while (1) {
          RTPPayload *p;
          
          do {
               size = recv( stream->fd, buf, 1, MSG_WAITALL );
               if (size < 1)
                    return DR_EOF;
          } while (buf[0] != '$');
     
          size = recv( stream->fd, buf, 3, MSG_WAITALL );
          if (size < 3)
               return DR_EOF;
          
          id = buf[0];
          len = (buf[1] << 8) | buf[2];
          if (len < 12)
               continue;
     
          size = recv( stream->fd, buf, 12, MSG_WAITALL );
          if (size < 12)
               return DR_EOF;
          len -= 12;

          if ((buf[0] & 0xc0) != (2 << 6))
               D_DEBUG_AT( Direct_Stream, "Bad RTP version %d!\n", buf[0] );
          seq = (buf[2] <<  8) |  buf[3];
          ts  = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
          
          for (p = payloads; p->pt; p++) {
               if (p->pt == (buf[1] & 0x7f))
                    break;
          }
          
          switch (p->pt) {
               case  0: // Unhandled
                    skip = len;
                    break;
               case 14: // MPEG Audio
                    skip = 4;
                    break;
               case 32: // MPEG Video
                    size = recv( stream->fd, buf, 1, MSG_WAITALL );
                    if (size < 1)
                         return DR_EOF;
                    len--;
                    skip = 3;
                    if (buf[0] & (1 << 2))
                         skip += 4;
                    break;
               case 34: // H263
                    size = recv( stream->fd, buf, 1, MSG_WAITALL );
                    if (size < 1)
                         return DR_EOF;
                    len--;
                    skip = 3;
                    if (buf[0] & (1 << 7))
                         skip += 4;
                    if (buf[0] & (1 << 6))
                         skip += 4;
                    break;                   
               default:
                    skip = 0;
                    break;
          }
          
          while (skip) {
               size = recv( stream->fd, buf, MIN(skip,12), MSG_WAITALL );
               if (size < 1)
                    return DR_EOF;
               len -= size;
               skip -= size;
          }
          
          if (len > 0) {
               unsigned char *dst;
               
               size = len;
               if (stream->remote.real_pack) {
                    size += 12;
                    if (p->type == PAYLOAD_VIDEO)
                         size += 7;
               }
                  
               stream->cache = D_REALLOC( stream->cache, stream->cache_size+size );
               if (!stream->cache)
                    return D_OOM();
               dst = stream->cache+stream->cache_size;
               stream->cache_size += size;
               
               if (stream->remote.real_pack) {                              
                    if (p->type == PAYLOAD_VIDEO) {
                         dst += rmf_write_pheader( dst, id, len+7, ts );
                         dst[0] = 0x81;
                         dst[1] = 0x01;
                         dst[2] = (len | 0x4000)>>8; dst[3] = len;
                         dst[4] = (len | 0x4000)>>8; dst[5] = len;
                         dst[6] = seq;
                         dst += 7;
                    } else {
                         dst += rmf_write_pheader( dst, id, len, ts );
                    }
               }
               
               while (len) {
                    size = recv( stream->fd, dst, len, MSG_WAITALL );
                    if (size < 1)
                         return DR_EOF;
                    dst += size;
                    len -= size;
               }
               break;
          }
     }
     
     return DR_OK;
}

static DirectResult
rtsp_peek( DirectStream *stream,
           unsigned int  length,
           int           offset,
           void         *buf,
           unsigned int *read_out )
{
     DirectResult ret;
     unsigned int len;

     if (offset < 0)
          return DR_UNSUPPORTED;

     len = length + offset;
     while (len > stream->cache_size) {
          ret = (stream->remote.real_rtsp) 
                ? rvp_read_packet( stream ) 
                : rtp_read_packet( stream );
          if (ret) {
               if (stream->cache_size < offset)
                    return ret;
               break;
          }
     }

     len = MIN( stream->cache_size-offset, length );
     direct_memcpy( buf, stream->cache+offset, len );

     if (read_out)
          *read_out = len;

     return DR_OK;
}

static DirectResult
rtsp_read( DirectStream *stream,
           unsigned int  length,
           void         *buf,
           unsigned int *read_out )
{
     DirectResult ret;
     unsigned int size = 0;

     while (size < length) {
          if (stream->cache_size) {
               unsigned int len = MIN( stream->cache_size, length-size );
          
               direct_memcpy( buf+size, stream->cache, len );
               size += len;
               stream->cache_size -= len;
          
               if (stream->cache_size) {
                    direct_memcpy( stream->cache, 
                                   stream->cache+len, stream->cache_size );
               } else {
                    D_FREE( stream->cache );
                    stream->cache = NULL;
               }
          }
          
          if (size < length) {
               ret = (stream->remote.real_rtsp) 
                     ? rvp_read_packet( stream ) 
                     : rtp_read_packet( stream );
               if (ret) {
                    if (!size)
                         return ret;
                    break;
               }
          }          
     }

     stream->offset += size;

     if (read_out)
          *read_out = size;

     return DR_OK;
}           
 
static DirectResult 
rtsp_open( DirectStream *stream, const char *filename )
{
     DirectResult ret;
     
     stream->remote.port = RTSP_PORT;

     ret = net_open( stream, filename, IPPROTO_TCP );
     if (ret)
          return ret;
          
     ret = rtsp_session_open( stream );
     if (ret) {
          close( stream->remote.sd );
          return ret;
     }
       
     stream->peek = rtsp_peek;
     stream->read = rtsp_read;
     
     return DR_OK;
}

#endif /* DIRECT_BUILD_NETWORK */   

/************************** End of Network Support ***************************/

static DirectResult
pipe_wait( DirectStream   *stream,
           unsigned int    length,
           struct timeval *tv )
{
     fd_set s;

     if (stream->cache_size >= length)
          return DR_OK;
     
     FD_ZERO( &s );
     FD_SET( stream->fd, &s );
     
     switch (select( stream->fd+1, &s, NULL, NULL, tv )) {
          case 0:
               if (!tv && !stream->cache_size)
                    return DR_EOF;
               return DR_TIMEOUT;
          case -1:
               return errno2result( errno );
     }

     return DR_OK;
}

static DirectResult
pipe_peek( DirectStream *stream,
           unsigned int  length,
           int           offset,
           void         *buf,
           unsigned int *read_out )
{
     unsigned int size = length;
     int          len;

     if (offset < 0)
          return DR_UNSUPPORTED;

     len = length + offset;
     if (len > stream->cache_size) {
          ssize_t s;
          
          stream->cache = D_REALLOC( stream->cache, len );
          if (!stream->cache) {
               stream->cache_size = 0;
               return D_OOM();
          }

          s = read( stream->fd, 
                    stream->cache + stream->cache_size,
                    len - stream->cache_size );
          if (s < 0) {
               if (errno != EAGAIN || stream->cache_size == 0)
                    return errno2result( errno );
               s = 0;
          }
          
          stream->cache_size += s;
          if (stream->cache_size <= offset)
               return DR_BUFFEREMPTY;
          
          size = stream->cache_size - offset;
     }

     direct_memcpy( buf, stream->cache+offset, size );

     if (read_out)
          *read_out = size;

     return DR_OK;
}

static DirectResult
pipe_read( DirectStream *stream,
           unsigned int  length,
           void         *buf,
           unsigned int *read_out )
{
     unsigned int size = 0;

     if (stream->cache_size) {
          size = MIN( stream->cache_size, length );
          
          direct_memcpy( buf, stream->cache, size );
     
          length -= size;
          stream->cache_size -= size; 
          
          if (stream->cache_size) {
               direct_memcpy( stream->cache, 
                              stream->cache+size, stream->cache_size );
          } else {
               D_FREE( stream->cache );
               stream->cache = NULL;
          }
     }

     if (length) {
          ssize_t s;
          
          s = read( stream->fd, buf+size, length-size );
          switch (s) {
               case 0:
                    if (!size)
                         return DR_EOF;
                    break;
               case -1:
                    if (!size) {
                         return (errno == EAGAIN)
                                ? DR_BUFFEREMPTY
                                : errno2result( errno );
                    }
                    break;
               default:
                    size += s;
                    break;
          }
     }

     stream->offset += size;

     if (read_out)
          *read_out = size;

     return DR_OK;
}
 
/*****************************************************************************/

static DirectResult
file_peek( DirectStream *stream,
           unsigned int  length,
           int           offset,
           void         *buf,
           unsigned int *read_out )
{
     DirectResult ret = DR_OK;
     ssize_t      size;
     
     if (lseek( stream->fd, offset, SEEK_CUR ) < 0)
          return DR_FAILURE;

     size = read( stream->fd, buf, length );
     switch (size) {
          case 0:
               ret = DR_EOF;
               break;
          case -1:
               ret = (errno == EAGAIN)
                     ? DR_BUFFEREMPTY
                     : errno2result( errno );
               size = 0;
               break;
     }

     if (lseek( stream->fd, - offset - size, SEEK_CUR ) < 0)
          return DR_FAILURE;
     
     if (read_out)
          *read_out = size;

     return ret;
}

static DirectResult
file_read( DirectStream *stream,
           unsigned int  length,
           void         *buf,
           unsigned int *read_out )
{
     ssize_t size;

     size = read( stream->fd, buf, length );
     switch (size) {
          case 0:
               return DR_EOF;
          case -1:
               if (errno == EAGAIN)
                    return DR_BUFFEREMPTY;
               return errno2result( errno );
     }

     stream->offset += size;
     
     if (read_out)
          *read_out = size;

     return DR_OK;
}

static DirectResult
file_seek( DirectStream *stream, unsigned int offset )
{
     off_t off;

     off = lseek( stream->fd, offset, SEEK_SET );
     if (off < 0)
          return DR_FAILURE;

     stream->offset = off;

     return DR_OK;
}

static DirectResult
file_open( DirectStream *stream, const char *filename, int fileno )
{
     if (filename)
          stream->fd = open( filename, O_RDONLY | O_NONBLOCK );
     else
          stream->fd = dup( fileno );
     
     if (stream->fd < 0)
          return errno2result( errno );
    
     fcntl( stream->fd, F_SETFL, 
               fcntl( stream->fd, F_GETFL ) | O_NONBLOCK );

     if (lseek( stream->fd, 0, SEEK_CUR ) < 0 && errno == ESPIPE) {
          stream->length = -1;
          stream->wait   = pipe_wait;
          stream->peek   = pipe_peek;
          stream->read   = pipe_read;
     }
     else {
          struct stat s;

          if (fstat( stream->fd, &s ) < 0)
               return errno2result( errno );
      
          stream->length = s.st_size;
          stream->peek   = file_peek;
          stream->read   = file_read;
          stream->seek   = file_seek;
     }
     
     return DR_OK;
}

/*****************************************************************************/

DirectResult
direct_stream_create( const char    *filename,
                      DirectStream **ret_stream )
{
     DirectStream *stream;
     DirectResult  ret;

     D_ASSERT( filename != NULL );
     D_ASSERT( ret_stream != NULL );
     
     stream = D_CALLOC( 1, sizeof(DirectStream) );
     if (!stream)
          return D_OOM();

     D_MAGIC_SET( stream, DirectStream );
     
     stream->ref =  1;
     stream->fd  = -1;

     if (!strncmp( filename, "stdin:/", 7 )) {
          ret = file_open( stream, NULL, STDIN_FILENO );
     }
     else if (!strncmp( filename, "file:/", 6 )) {
          ret = file_open( stream, filename+6, -1 );
     }
     else if (!strncmp( filename, "fd:/", 4 )) {
          ret = (filename[4] >= '0' && filename[4] <= '9')
                ? file_open( stream, NULL, atoi(filename+4) ) : DR_INVARG;
     }
#if DIRECT_BUILD_NETWORK
     else if (!strncmp( filename, "http://", 7 ) ||
              !strncmp( filename, "unsv://", 7 )) {
          ret = http_open( stream, filename+7 );
     }
     else if (!strncmp( filename, "ftp://", 6 )) {
          ret = ftp_open( stream, filename+6 );
     }
     else if (!strncmp( filename, "rtsp://", 7 )) {
          ret = rtsp_open( stream, filename+7 );
     }
     else if (!strncmp( filename, "tcp://", 6 )) {
          ret = net_open( stream, filename+6, IPPROTO_TCP );
     }
     else if (!strncmp( filename, "udp://", 6 )) {
          ret = net_open( stream, filename+6, IPPROTO_UDP );
     }
#endif
     else {
          ret = file_open( stream, filename, -1 );
     }

     if (ret) {
          direct_stream_close( stream );
          D_FREE( stream );
          return ret;
     }

     *ret_stream = stream;

     return DR_OK;
}

DirectStream*
direct_stream_dup( DirectStream *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
     
     stream->ref++;
     
     return stream;
}

int
direct_stream_fileno( DirectStream  *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
     
     return stream->fd;
}

bool
direct_stream_seekable( DirectStream *stream )
{
     D_ASSERT( stream != NULL );
     
     D_MAGIC_ASSERT( stream, DirectStream );

     return stream->seek ? true : false;
}

bool
direct_stream_remote( DirectStream *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
     
#if DIRECT_BUILD_NETWORK
     if (stream->remote.host)
          return true;
#endif
     return false;
}

const char*
direct_stream_mime( DirectStream *stream )
{
     D_ASSERT( stream != NULL );
     
     D_MAGIC_ASSERT( stream, DirectStream );
     
     return stream->mime;
}

unsigned int 
direct_stream_offset( DirectStream *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
     
     return stream->offset;
}

unsigned int
direct_stream_length( DirectStream *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );

     return (stream->length >= 0) ? stream->length : stream->offset;
}

DirectResult
direct_stream_wait( DirectStream   *stream,
                    unsigned int    length,
                    struct timeval *tv )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
 
     if (length && stream->wait)
          return stream->wait( stream, length, tv );

     return DR_OK;
}

DirectResult
direct_stream_peek( DirectStream *stream,
                    unsigned int  length,
                    int           offset,
                    void         *buf,
                    unsigned int *read_out )
{
     D_ASSERT( stream != NULL );
     D_ASSERT( length != 0 );
     D_ASSERT( buf != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );
     
     if (stream->length >= 0 && (stream->offset + offset) >= stream->length)
          return DR_EOF;

     if (stream->peek)
          return stream->peek( stream, length, offset, buf, read_out );

     return DR_UNSUPPORTED;
}

DirectResult
direct_stream_read( DirectStream *stream,
                    unsigned int  length,
                    void         *buf,
                    unsigned int *read_out )
{
     D_ASSERT( stream != NULL ); 
     D_ASSERT( length != 0 );
     D_ASSERT( buf != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );

     if (stream->length >= 0 && stream->offset >= stream->length)
          return DR_EOF;

     if (stream->read)
          return stream->read( stream, length, buf, read_out );

     return DR_UNSUPPORTED;
}

DirectResult
direct_stream_seek( DirectStream *stream,
                    unsigned int  offset )
{
     D_ASSERT( stream != NULL );
     
     D_MAGIC_ASSERT( stream, DirectStream );

     if (stream->offset == offset)
          return DR_OK;
     
     if (stream->length >= 0 && offset > stream->length)
          offset = stream->length;
     
     if (stream->seek)
          return stream->seek( stream, offset );

     return DR_UNSUPPORTED;
}

static void
direct_stream_close( DirectStream *stream )
{
#if DIRECT_BUILD_NETWORK
     if (stream->remote.host) {
          D_FREE( stream->remote.host );
          stream->remote.host = NULL;
     }

     if (stream->remote.user) {
          D_FREE( stream->remote.user );
          stream->remote.user = NULL;
     }

     if (stream->remote.pass) {
          D_FREE( stream->remote.pass );
          stream->remote.pass = NULL;
     }

     if (stream->remote.auth) {
          D_FREE( stream->remote.auth );
          stream->remote.auth = NULL;
     }

     if (stream->remote.path) {
          D_FREE( stream->remote.path );
          stream->remote.path = NULL;
     }
     
     if (stream->remote.addr) {
          freeaddrinfo( stream->remote.addr );
          stream->remote.addr = NULL;
     }
     
     if (stream->remote.data) {
          D_FREE( stream->remote.data );
          stream->remote.data = NULL;
     }

     if (stream->remote.sd > 0) {
          close( stream->remote.sd ); 
          stream->remote.sd = -1;
     }
#endif
     
     if (stream->mime) {
          D_FREE( stream->mime );
          stream->mime = NULL;
     }
     
     if (stream->cache) {
          D_FREE( stream->cache );
          stream->cache = NULL;
          stream->cache_size = 0;
     }

     if (stream->fd >= 0) {
          fcntl( stream->fd, F_SETFL, 
                    fcntl( stream->fd, F_GETFL ) & ~O_NONBLOCK );
          close( stream->fd );
          stream->fd = -1;
     }
}

void
direct_stream_destroy( DirectStream *stream )
{
     D_ASSERT( stream != NULL );

     D_MAGIC_ASSERT( stream, DirectStream );

     if (--stream->ref == 0) {
          direct_stream_close( stream );

          D_FREE( stream );
     }
}
