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

#include <pthread.h>

#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <direct/clock.h>
#include <direct/conf.h>
#include <direct/debug.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/signals.h>
#include <direct/system.h>
#include <direct/trace.h>
#include <direct/util.h>

D_DEBUG_DOMAIN( Direct_Signals, "Direct/Signals", "Signal handling" );

#define SIG_CLOSE_SIGHANDLER 123

struct __D_DirectSignalHandler {
     DirectLink               link;

     int                      magic;

     int                      num;
     DirectSignalHandlerFunc  func;
     void                    *ctx;
};

/**************************************************************************************************/

typedef struct {
     int              signum;
     struct sigaction old_action;
} SigHandled;

static int sigs_to_handle[] = { /*SIGALRM,*/ SIGHUP, SIGINT, /*SIGPIPE,*/ /*SIGPOLL,*/
                                SIGTERM, /*SIGUSR1, SIGUSR2,*/ /*SIGVTALRM,*/
                                /*SIGSTKFLT,*/ SIGABRT, SIGFPE, SIGILL, SIGQUIT,
                                SIGSEGV, SIGTRAP, /*SIGSYS, SIGEMT,*/ SIGBUS,
                                SIGXCPU, SIGXFSZ, SIG_CLOSE_SIGHANDLER };

#define NUM_SIGS_TO_HANDLE ((int)D_ARRAY_SIZE( sigs_to_handle ))

static DirectLink      *handlers = NULL;
static pthread_mutex_t  handlers_lock;

static pthread_t sighandler_thread = -1;

/**************************************************************************************************/

static void *handle_signals( void *ptr );

/**************************************************************************************************/

DirectResult
direct_signals_initialize( void )
{
     sigset_t mask;
     int ret;
     int i;

     D_DEBUG_AT( Direct_Signals, "Initializing...\n" );

     direct_util_recursive_pthread_mutex_init( &handlers_lock );

     if (direct_config->sighandler) {
          sigemptyset( &mask );
          for (i=0; i<NUM_SIGS_TO_HANDLE; i++)
               sigaddset( &mask, sigs_to_handle[i] );

          pthread_sigmask( SIG_BLOCK, &mask, NULL );

          ret = pthread_create( &sighandler_thread, NULL, handle_signals, NULL );
          (void)ret;
          D_ASSERT( ret == 0 );
          D_ASSERT( sighandler_thread >= 0 );
     }

     return DR_OK;
}

DirectResult
direct_signals_shutdown( void )
{
     D_ASSERT( sighandler_thread >= 0 );
     D_DEBUG_AT( Direct_Signals, "Shutting down...\n" );

     if (direct_config->sighandler) {
          pthread_kill( sighandler_thread, SIG_CLOSE_SIGHANDLER );
          sighandler_thread = -1;
     }

     pthread_mutex_destroy( &handlers_lock );

     return DR_OK;
}

void
direct_signals_block_all( void )
{
     sigset_t signals;

     D_DEBUG_AT( Direct_Signals, "Blocking all signals from now on!\n" );

     sigfillset( &signals );

     if (pthread_sigmask( SIG_BLOCK, &signals, NULL ))
          D_PERROR( "Direct/Signals: Setting signal mask failed!\n" );
}

DirectResult
direct_signal_handler_add( int                       num,
                           DirectSignalHandlerFunc   func,
                           void                     *ctx,
                           DirectSignalHandler     **ret_handler )
{
     DirectSignalHandler *handler;

     D_ASSERT( func != NULL );
     D_ASSERT( ret_handler != NULL );

     D_DEBUG_AT( Direct_Signals,
                 "Adding handler %p for signal %d with context %p...\n", func, num, ctx );

     handler = D_CALLOC( 1, sizeof(DirectSignalHandler) );
     if (!handler) {
          D_WARN( "out of memory" );
          return DR_NOLOCALMEMORY;
     }

     handler->num  = num;
     handler->func = func;
     handler->ctx  = ctx;

     D_MAGIC_SET( handler, DirectSignalHandler );

     pthread_mutex_lock( &handlers_lock );
     direct_list_append( &handlers, &handler->link );
     pthread_mutex_unlock( &handlers_lock );

     *ret_handler = handler;

     return DR_OK;
}

DirectResult
direct_signal_handler_remove( DirectSignalHandler *handler )
{
     D_MAGIC_ASSERT( handler, DirectSignalHandler );

     D_DEBUG_AT( Direct_Signals, "Removing handler %p for signal %d with context %p...\n",
                 handler->func, handler->num, handler->ctx );

     pthread_mutex_lock( &handlers_lock );
     direct_list_remove( &handlers, &handler->link );
     pthread_mutex_unlock( &handlers_lock );

     D_MAGIC_CLEAR( handler );

     D_FREE( handler );

     return DR_OK;
}

/**************************************************************************************************/

static bool
show_segv( const siginfo_t *info )
{
     switch (info->si_code) {
#ifdef SEGV_MAPERR
          case SEGV_MAPERR:
               direct_log_printf( NULL, " (at %p, invalid address) <--\n", info->si_addr );
               return true;
#endif
#ifdef SEGV_ACCERR
          case SEGV_ACCERR:
               direct_log_printf( NULL, " (at %p, invalid permissions) <--\n", info->si_addr );
               return true;
#endif
     }
     return false;
}

static bool
show_bus( const siginfo_t *info )
{
     switch (info->si_code) {
#ifdef BUG_ADRALN
          case BUS_ADRALN:
               direct_log_printf( NULL, " (at %p, invalid address alignment) <--\n", info->si_addr );
               return true;
#endif
#ifdef BUS_ADRERR
          case BUS_ADRERR:
               direct_log_printf( NULL, " (at %p, non-existent physical address) <--\n", info->si_addr );
               return true;
#endif
#ifdef BUS_OBJERR
          case BUS_OBJERR:
               direct_log_printf( NULL, " (at %p, object specific hardware error) <--\n", info->si_addr );
               return true;
#endif
     }

     return false;
}

static bool
show_ill( const siginfo_t *info )
{
     switch (info->si_code) {
#ifdef ILL_ILLOPC
          case ILL_ILLOPC:
               direct_log_printf( NULL, " (at %p, illegal opcode) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_ILLOPN
          case ILL_ILLOPN:
               direct_log_printf( NULL, " (at %p, illegal operand) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_ILLADR
          case ILL_ILLADR:
               direct_log_printf( NULL, " (at %p, illegal addressing mode) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_ILLTRP
          case ILL_ILLTRP:
               direct_log_printf( NULL, " (at %p, illegal trap) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_PRVOPC
          case ILL_PRVOPC:
               direct_log_printf( NULL, " (at %p, privileged opcode) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_PRVREG
          case ILL_PRVREG:
               direct_log_printf( NULL, " (at %p, privileged register) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_COPROC
          case ILL_COPROC:
               direct_log_printf( NULL, " (at %p, coprocessor error) <--\n", info->si_addr );
               return true;
#endif
#ifdef ILL_BADSTK
          case ILL_BADSTK:
               direct_log_printf( NULL, " (at %p, internal stack error) <--\n", info->si_addr );
               return true;
#endif
     }

     return false;
}

static bool
show_fpe( const siginfo_t *info )
{
     switch (info->si_code) {
#ifdef FPE_INTDIV
          case FPE_INTDIV:
               direct_log_printf( NULL, " (at %p, integer divide by zero) <--\n", info->si_addr );
               return true;
#endif
#ifdef FPE_FLTDIV
          case FPE_FLTDIV:
               direct_log_printf( NULL, " (at %p, floating point divide by zero) <--\n", info->si_addr );
               return true;
#endif
     }

     direct_log_printf( NULL, " (at %p) <--\n", info->si_addr );

     return true;
}

static bool
show_any( const siginfo_t *info )
{
     switch (info->si_code) {
#ifdef SI_USER
          case SI_USER:
               direct_log_printf( NULL, " (sent by pid %d, uid %d) <--\n", info->si_pid, info->si_uid );
               return true;
#endif
#ifdef SI_KERNEL
          case SI_KERNEL:
               direct_log_printf( NULL, " (sent by the kernel) <--\n" );
               return true;
#endif
     }
     return false;
}

static void
#ifdef SA_SIGINFO
signal_handler( int num, siginfo_t *info, void *foo )
#else
signal_handler( int num )
#endif
{
     DirectLink *l, *n;
     sigset_t    mask;
     void       *addr   = NULL;
     int         pid    = direct_gettid();
     long long   millis = direct_clock_get_millis();

     fflush(stdout);
     fflush(stderr);

     direct_log_printf( NULL, "(!) [%5d: %4lld.%03lld] --> Caught signal %d",
                        pid, millis/1000, millis%1000, num );

#ifdef SA_SIGINFO
     if (info && info > (siginfo_t*) 0x100) {
          bool shown = false;

          if (info->si_code > 0 && info->si_code < 0x80) {
               addr = info->si_addr;

               switch (num) {
                    case SIGSEGV:
                         shown = show_segv( info );
                         break;

                    case SIGBUS:
                         shown = show_bus( info );
                         break;

                    case SIGILL:
                         shown = show_ill( info );
                         break;

                    case SIGFPE:
                         shown = show_fpe( info );
                         break;

                    default:
                         direct_log_printf( NULL, " <--\n" );
                         addr  = NULL;
                         shown = true;
                         break;
               }
          }
          else
               shown = show_any( info );

          if (!shown)
               direct_log_printf( NULL, " (unknown origin) <--\n" );
     }
     else
#endif
          direct_log_printf( NULL, ", no siginfo available <--\n" );

     direct_trace_print_stacks();

     /* Loop through all handlers. */
     pthread_mutex_lock( &handlers_lock );

     direct_list_foreach_safe (l, n, handlers) {
          DirectSignalHandler *handler = (DirectSignalHandler*) l;

          if (handler->num != num && handler->num != DIRECT_SIGNAL_ANY)
               continue;

          switch (handler->func( num, addr, handler->ctx )) {
               case DSHR_OK:
                    break;

               case DSHR_REMOVE:
                    direct_list_remove( &handlers, &handler->link );
                    D_MAGIC_CLEAR( handler );
                    D_FREE( handler );
                    break;

               case DSHR_RESUME:
                    millis = direct_clock_get_millis();

                    direct_log_printf( NULL, "(!) [%5d: %4lld.%03lld]      -> cured!\n",
                                       pid, millis / 1000, millis % 1000 );
                    pthread_mutex_unlock( &handlers_lock );
                    return;

               default:
                    D_BUG( "unknown result" );
                    break;
          }
     }

     pthread_mutex_unlock( &handlers_lock );

     sigemptyset( &mask );
     sigaddset( &mask, num );
     pthread_sigmask( SIG_UNBLOCK, &mask, NULL );

     raise( num );

     pthread_sigmask( SIG_BLOCK, &mask, NULL );
     
     abort();

     exit( -num );
}

/**************************************************************************************************/

static void *
handle_signals( void *ptr )
{
     int       i;
     int       res;
     siginfo_t info;
     sigset_t  mask;

     sigemptyset( &mask );

     for (i=0; i<NUM_SIGS_TO_HANDLE; i++) {
          if (direct_config->sighandler && !sigismember( &direct_config->dont_catch, sigs_to_handle[i] ))
               sigaddset( &mask, sigs_to_handle[i] );
     }

     pthread_sigmask( SIG_BLOCK, &mask, NULL );

     while (1) {
          res = sigwaitinfo( &mask, &info );

          if ( -1 == res ) {
               //error
          }
          else {
               if (SIG_CLOSE_SIGHANDLER == info.si_signo) {
                    if (getpid() == info.si_pid)
                         break;
               }
               else {
#ifdef SA_SIGINFO
                    signal_handler( info.si_signo, &info, NULL );
#else
                    signal_handler( info.si_signo );
#endif
               }
          }
     }

     return NULL;
}
