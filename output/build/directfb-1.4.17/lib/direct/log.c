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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <netdb.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/log.h>
#include <direct/util.h>


struct __D_DirectLog {
     int             magic;

     DirectLogType   type;

     int             fd;

     pthread_mutex_t lock;
};

/**********************************************************************************************************************/

/* Statically allocated to avoid endless loops between D_CALLOC() and D_DEBUG(), while the latter would only
 * call the allocation once, if there wouldn't be the loopback...
 */
static DirectLog       fallback_log;

static DirectLog      *default_log   = NULL;
static pthread_once_t  init_fallback = PTHREAD_ONCE_INIT;

/**********************************************************************************************************************/

static DirectResult init_stderr( DirectLog  *log );

static DirectResult init_file  ( DirectLog  *log,
                                 const char *filename );

static DirectResult init_udp   ( DirectLog  *log,
                                 const char *hostport );

/**********************************************************************************************************************/

DirectResult
direct_log_create( DirectLogType   type,
                   const char     *param,
                   DirectLog     **ret_log )
{
     DirectResult  ret = DR_INVARG;
     DirectLog    *log;

     log = D_CALLOC( 1, sizeof(DirectLog) );
     if (!log)
          return D_OOM();

     log->type = type;

     switch (type) {
          case DLT_STDERR:
               ret = init_stderr( log );
               break;

          case DLT_FILE:
               ret = init_file( log, param );
               break;

          case DLT_UDP:
               ret = init_udp( log, param );
               break;
     }

     if (ret)
          D_FREE( log );
     else {
          direct_util_recursive_pthread_mutex_init( &log->lock );

          D_MAGIC_SET( log, DirectLog );

          *ret_log = log;
     }

     return ret;
}

DirectResult
direct_log_destroy( DirectLog *log )
{
     D_MAGIC_ASSERT( log, DirectLog );

     D_ASSERT( &fallback_log != log );

     if (log == default_log)
          default_log = NULL;

     close( log->fd );

     D_MAGIC_CLEAR( log );

     D_FREE( log );

     return DR_OK;
}

__attribute__((no_instrument_function))
DirectResult
direct_log_printf( DirectLog  *log,
                   const char *format, ... )
{
     va_list args;

     /*
      * Don't use D_MAGIC_ASSERT or any other
      * macros/functions that might cause an endless loop.
      */

     va_start( args, format );

     /* Use the default log if passed log is invalid. */
     if (!log || log->magic != D_MAGIC("DirectLog"))
          log = direct_log_default();

     /* Write to stderr as a fallback if default is invalid, too. */
     if (!log || log->magic != D_MAGIC("DirectLog")) {
          vfprintf( stderr, format, args );
          fflush( stderr );
     }
     else {
          int  len;
          char buf[512];

          len = vsnprintf( buf, sizeof(buf), format, args );

          pthread_mutex_lock( &log->lock );

          write( log->fd, buf, len );

          pthread_mutex_unlock( &log->lock );
     }

     va_end( args );

     return DR_OK;
}

DirectResult
direct_log_set_default( DirectLog *log )
{
     D_MAGIC_ASSERT( log, DirectLog );

     default_log = log;

     return DR_OK;
}

__attribute__((no_instrument_function))
void
direct_log_lock( DirectLog *log )
{
     D_MAGIC_ASSERT_IF( log, DirectLog );

     if (!log)
          log = direct_log_default();

     D_MAGIC_ASSERT( log, DirectLog );

     pthread_mutex_lock( &log->lock );
}

__attribute__((no_instrument_function))
void
direct_log_unlock( DirectLog *log )
{
     D_MAGIC_ASSERT_IF( log, DirectLog );

     if (!log)
          log = direct_log_default();

     D_MAGIC_ASSERT( log, DirectLog );

     pthread_mutex_unlock( &log->lock );
}

__attribute__((no_instrument_function))
static void
init_fallback_log( void )
{
     fallback_log.type = DLT_STDERR;
     fallback_log.fd   = fileno( stderr );

     direct_util_recursive_pthread_mutex_init( &fallback_log.lock );

     D_MAGIC_SET( &fallback_log, DirectLog );
}

__attribute__((no_instrument_function))
DirectLog *
direct_log_default( void )
{
     pthread_once( &init_fallback, init_fallback_log );

     if (!default_log)
          default_log = &fallback_log;

     D_MAGIC_ASSERT( default_log, DirectLog );

     return default_log;
}

/**********************************************************************************************************************/

static DirectResult
init_stderr( DirectLog *log )
{
     log->fd = dup( fileno( stderr ) );

     return DR_OK;
}

static DirectResult
init_file( DirectLog  *log,
           const char *filename )
{
     DirectResult ret;
     int          fd;

     fd = open( filename, O_WRONLY | O_CREAT | O_APPEND, 0664 );
     if (fd < 0) {
          ret = errno2result( errno );
          D_PERROR( "Direct/Log: Could not open '%s' for writing!\n", filename );
          return ret;
     }

     log->fd = fd;

     return DR_OK;
}

static DirectResult
parse_host_addr( const char       *hostport,
                 struct addrinfo **ret_addr )
{
     int   i, ret;
     
     int   size = strlen( hostport ) + 1;
     char  buf[size];
     
     char *hoststr = buf;
     char *portstr = NULL;
     char *end;

     struct addrinfo hints;

     memcpy( buf, hostport, size );

     for (i=0; i<size; i++) {
          if (buf[i] == ':') {
               buf[i]  = 0;
               portstr = &buf[i+1];

               break;
          }
     }

     if (!portstr) {
          D_ERROR( "Direct/Log: Parse error in '%s' that should be '<host>:<port>'!\n", hostport );
          return DR_INVARG;
     }

     strtoul( portstr, &end, 10 );
     if (end && *end) {
          D_ERROR( "Direct/Log: Parse error in port number '%s'!\n", portstr );
          return DR_INVARG;
     }
     
     memset( &hints, 0, sizeof(hints) );
     hints.ai_socktype = SOCK_DGRAM;
     hints.ai_family   = PF_UNSPEC;
     
     ret = getaddrinfo( hoststr, portstr, &hints, ret_addr );
     if (ret) {
          switch (ret) {
               case EAI_FAMILY:
                    D_ERROR( "Direct/Log: Unsupported address family!\n" );
                    return DR_UNSUPPORTED;
               
               case EAI_SOCKTYPE:
                    D_ERROR( "Direct/Log: Unsupported socket type!\n" );
                    return DR_UNSUPPORTED;
               
               case EAI_NONAME:
                    D_ERROR( "Direct/Log: Host not found!\n" );
                    return DR_FAILURE;
                    
               case EAI_SERVICE:
                    D_ERROR( "Direct/Log: Port %s is unreachable!\n", portstr );
                    return DR_FAILURE;
               
#ifdef EAI_ADDRFAMILY
               case EAI_ADDRFAMILY:
#endif
               case EAI_NODATA:
                    D_ERROR( "Direct/Log: Host found, but has no address!\n" );
                    return DR_FAILURE;
                    
               case EAI_MEMORY:
                    return D_OOM();

               case EAI_FAIL:
                    D_ERROR( "Direct/Log: A non-recoverable name server error occurred!\n" );
                    return DR_FAILURE;

               case EAI_AGAIN:
                    D_ERROR( "Direct/Log: Temporary error, try again!\n" );
                    return DR_TEMPUNAVAIL;
                    
               default:
                    D_ERROR( "Direct/Log: Unknown error occured!?\n" );
                    return DR_FAILURE;
          }
     }

     return DR_OK;
}

static DirectResult
init_udp( DirectLog  *log,
          const char *hostport )
{
     DirectResult     ret;
     int              fd;
     struct addrinfo *addr;
     
     ret = parse_host_addr( hostport, &addr );
     if (ret)
          return ret;

     fd = socket( addr->ai_family, SOCK_DGRAM, 0 );
     if (fd < 0) {
          ret = errno2result( errno );
          D_PERROR( "Direct/Log: Could not create a UDP socket!\n" );
          freeaddrinfo( addr );
          return ret;
     }

     ret = connect( fd, addr->ai_addr, addr->ai_addrlen );
     freeaddrinfo( addr );
     
     if (ret) {
          ret = errno2result( errno );
          D_PERROR( "Direct/Log: Could not connect UDP socket to '%s'!\n", hostport );
          close( fd );
          return ret;
     }

     log->fd = fd;

     return DR_OK;
}
