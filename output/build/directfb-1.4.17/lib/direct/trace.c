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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <direct/build.h>
#include <direct/list.h>
#include <direct/log.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/system.h>
#include <direct/thread.h>
#include <direct/trace.h>
#include <direct/util.h>


#ifdef PIC
#define DYNAMIC_LINKING
#endif


#if DIRECT_BUILD_TRACE

#ifdef DYNAMIC_LINKING
#include <dlfcn.h>
#endif

#define MAX_BUFFERS 200
#define MAX_LEVEL   200

#define NAME_LEN    92


typedef enum {
     TF_NONE   = 0x00000000,

     TF_DEBUG  = 0x00000001
} TraceFlags;

typedef struct {
     void       *addr;
     TraceFlags  flags;
} Trace;

struct __D_DirectTraceBuffer {
     pid_t tid;
     char *name;
     int   level;
     bool  in_trace;
     Trace trace[MAX_LEVEL];
};

/**************************************************************************************************/

static DirectTraceBuffer *buffers[MAX_BUFFERS];
static int                buffers_num  = 0;
#ifdef HAVE_DECL_PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
static pthread_mutex_t    buffers_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
static pthread_mutex_t    buffers_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static pthread_key_t      trace_key    = -1;

/**************************************************************************************************/

__attribute__((no_instrument_function))
static void
buffer_destroy( void *arg )
{
     int                i;
     DirectTraceBuffer *buffer = arg;

     pthread_mutex_lock( &buffers_lock );

     /* Remove from list. */
     for (i=0; i<buffers_num; i++) {
          if (buffers[i] == buffer)
               break;
     }

     for (; i<buffers_num-1; i++)
          buffers[i] = buffers[i+1];

     buffers_num--;

     /* Deallocate the buffer. */
     direct_trace_free_buffer( buffer );

     pthread_mutex_unlock( &buffers_lock );
}

__attribute__((no_instrument_function))
static inline DirectTraceBuffer *
get_trace_buffer( void )
{
     DirectTraceBuffer *buffer;

     buffer = pthread_getspecific( trace_key );
     if (!buffer) {
          const char *name = direct_thread_self_name();

          pthread_mutex_lock( &buffers_lock );

          if (!buffers_num)
               pthread_key_create( &trace_key, buffer_destroy );
          else if (buffers_num == MAX_BUFFERS) {
               D_ERROR( "Direct/Trace: Maximum number of threads (%d) reached!\n", MAX_BUFFERS );
               pthread_mutex_unlock( &buffers_lock );
               return NULL;
          }

          pthread_setspecific( trace_key,
                               buffer = calloc( 1, sizeof(DirectTraceBuffer) ) );

          buffer->tid  = direct_gettid();
          buffer->name = name ? strdup( name ) : NULL;

          buffers[buffers_num++] = buffer;

          pthread_mutex_unlock( &buffers_lock );
     }

     return buffer;
}

/**************************************************************************************************/

typedef struct {
     long offset;
     char name[NAME_LEN];
} Symbol;

typedef struct {
     DirectLink  link;

     char       *filename;
     Symbol     *symbols;
     int         capacity;
     int         num_symbols;
} SymbolTable;

static DirectLink      *tables      = NULL;
#ifdef HAVE_DECL_PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
static pthread_mutex_t  tables_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
static pthread_mutex_t  tables_lock = PTHREAD_MUTEX_INITIALIZER;
#endif


__attribute__((no_instrument_function))
static void
add_symbol( SymbolTable *table, long offset, const char *name )
{
     Symbol *symbol;

     if (table->num_symbols == table->capacity) {
          Symbol *symbols;
          int     capacity = table->capacity * 2;

          if (!capacity)
               capacity = 256;

          symbols = malloc( capacity * sizeof(Symbol) );
          if (!symbols) {
               D_WARN( "out of memory" );
               return;
          }

          direct_memcpy( symbols, table->symbols, table->num_symbols * sizeof(Symbol) );

          free( table->symbols );

          table->symbols  = symbols;
          table->capacity = capacity;
     }

     symbol = &table->symbols[ table->num_symbols++ ];

     symbol->offset = offset;

     direct_snputs( symbol->name, name, NAME_LEN );
}

__attribute__((no_instrument_function))
static SymbolTable *
load_symbols( const char *filename )
{
     SymbolTable *table;
     FILE        *fp = NULL;
     bool         is_pipe = false;
     char         file[1024];
     char         line[1024];
     int          command_len;
     char        *command;
     const char  *full_path = filename;
     char        *tmp;

     if (filename) {
          if (access( filename, R_OK ) < 0 && errno == ENOENT) {
               int len;

               if ((len = readlink( "/proc/self/exe", file, sizeof(file) - 1 )) < 0) {
                    D_PERROR( "Direct/Trace: readlink( \"/proc/self/exe\" ) failed!\n" );
                    return NULL;
               }

               file[len] = 0;


               tmp = strrchr( file, '/' );
               if (!tmp)
                    return NULL;

               if (strcmp( filename, tmp + 1 ))
                    return NULL;

               full_path = file;
          }
     }
     else {
          int   len;

          if ((len = readlink( "/proc/self/exe", file, sizeof(file) - 1 )) < 0) {
               D_PERROR( "Direct/Trace: readlink( \"/proc/self/exe\" ) failed!\n" );
               return NULL;
          }

          file[len] = 0;

          full_path = file;
     }

     command_len = strlen( full_path ) + 32;
     command     = alloca( command_len );

     /* First check if there's an "nm-n" file. */
     tmp = strrchr( full_path, '/' );
     if (!tmp)
          return NULL;

     *tmp = 0;
     snprintf( command, command_len, "%s/nm-n.%s", full_path, tmp + 1 );
     *tmp = '/';

     if (access( command, R_OK ) == 0) {
          fp = fopen( command, "r" );
          if (!fp)
               D_PERROR( "Direct/Trace: fopen( \"%s\", \"r\" ) failed!\n", command );
     }
     else {
          snprintf( command, command_len, "%s.nm", full_path );
          if (access( command, R_OK ) == 0) {
               fp = fopen( command, "r" );
               if (!fp)
                    D_PERROR( "Direct/Trace: fopen( \"%s\", \"r\" ) failed!\n", command );
          }
     }

     /* Fallback to live mode. */
     if (!fp) {
          snprintf( command, command_len, "nm -n %s", full_path );

          fp = popen( command, "r" );
          if (!fp) {
               D_PERROR( "Direct/Trace: popen( \"%s\", \"r\" ) failed!\n", command );
               return NULL;
          }

          is_pipe = true;
     }

     table = calloc( 1, sizeof(SymbolTable) );
     if (!table) {
          D_OOM();
          goto out;
     }

     if (filename)
          table->filename = strdup( filename );

     while (fgets( line, sizeof(line), fp )) {
          int  n;
          int  digits = sizeof(long) * 2;
          long offset = 0;
          int  length = strlen(line);

          if (line[0] == ' ' || length < (digits + 5) || line[length-1] != '\n')
               continue;

          if (line[digits + 1] != 't' && line[digits + 1] != 'T' && line[digits + 1] != 'W')
               continue;

          if (line[digits] != ' ' || line[digits + 2] != ' ' || line[digits + 3] == '.')
               continue;

          for (n=0; n<digits; n++) {
               char c = line[n];

               offset <<= 4;

               if (c >= '0' && c <= '9')
                    offset |= c - '0';
               else
                    offset |= c - 'a' + 10;
          }

          line[length-1] = 0;

          add_symbol( table, offset, line + digits + 3 );
     }

out:
     if (is_pipe)
          pclose( fp );
     else
          fclose( fp );

     return table;
}

__attribute__((no_instrument_function))
static int
compare_symbols(const void *x, const void *y)
{
     return  *((const long*) x)  -  *((const long*) y);
}

__attribute__((no_instrument_function))
static SymbolTable *
find_table( const char *filename )
{
     SymbolTable *table;

     if (filename) {
          direct_list_foreach (table, tables) {
               if (table->filename && !strcmp( filename, table->filename ))
                    return table;
          }
     }
     else {
          direct_list_foreach (table, tables) {
               if (!table->filename)
                    return table;
          }
     }

     return NULL;
}

/**************************************************************************************************/

__attribute__((no_instrument_function))
const char *
direct_trace_lookup_symbol( const char *filename, long offset )
{
     Symbol      *symbol;
     SymbolTable *table;

     pthread_mutex_lock( &tables_lock );

     table = find_table( filename );
     if (!table) {
          table = load_symbols( filename );
          if (!table) {
               pthread_mutex_unlock( &tables_lock );
               return false;
          }

          direct_list_prepend( &tables, &table->link );
     }

     pthread_mutex_unlock( &tables_lock );

     symbol = bsearch( &offset, table->symbols, table->num_symbols,
                       sizeof(Symbol), compare_symbols );

     return symbol ? symbol->name : NULL;
}

__attribute__((no_instrument_function))
const char *
direct_trace_lookup_file( void *address, void **ret_base )
{
#ifdef DYNAMIC_LINKING
     Dl_info info;

     if (dladdr( address, &info )) {
          if (ret_base)
               *ret_base = info.dli_fbase;

          return info.dli_fname;
     }
     else
#endif
     {
          if (ret_base)
               *ret_base = NULL;
     }

     return NULL;
}

__attribute__((no_instrument_function))
static void
direct_trace_print_stack_recursive( DirectTraceBuffer *buffer, int level, int levels )
{
     void *fn = buffer->trace[level].addr;

#ifdef DYNAMIC_LINKING
     Dl_info info;
#endif

     if (level == levels) {
          /* lock, print header, unroll stack */

          direct_log_lock( NULL );

          if (buffer->name)
               direct_log_printf( NULL, "(-) [%5d: -STACK- '%s']\n", buffer->tid, buffer->name );
          else
               direct_log_printf( NULL, "(-) [%5d: -STACK- ]\n", buffer->tid );

          return;
     }

#ifdef DYNAMIC_LINKING
     if (dladdr( fn, &info )) {
          if (info.dli_fname) {
               const char *symbol = NULL;//info.dli_sname;

               if (!symbol) {
                    symbol = direct_trace_lookup_symbol(info.dli_fname, (long)(fn - info.dli_fbase));
                    if (!symbol) {
                         symbol = direct_trace_lookup_symbol(info.dli_fname, (long)(fn));
                         if (!symbol) {
                              if (info.dli_sname)
                                   symbol = info.dli_sname;
                              else
                                   symbol = "??";
                         }
                    }
               }

               direct_trace_print_stack_recursive( buffer, level+1, levels );
               direct_log_printf( NULL, "  #%-2d 0x%08lx in %s () from %s [%p]\n",
                                  levels - level - 1, (unsigned long) fn, symbol, info.dli_fname, info.dli_fbase );
          }
          else if (info.dli_sname) {
               direct_trace_print_stack_recursive( buffer, level+1, levels );
               direct_log_printf( NULL, "  #%-2d 0x%08lx in %s ()\n",
                                  levels - level - 1, (unsigned long) fn, info.dli_sname );
          }
          else {
               direct_trace_print_stack_recursive( buffer, level+1, levels );
               direct_log_printf( NULL, "  #%-2d 0x%08lx in ?? ()\n",
                                  levels - level - 1, (unsigned long) fn );
          }
     }
     else
#endif
     {
          const char *symbol = direct_trace_lookup_symbol(NULL, (long)(fn));

          direct_trace_print_stack_recursive( buffer, level+1, levels );
          direct_log_printf( NULL, "  #%-2d 0x%08lx in %s ()\n",
                             levels - level - 1, (unsigned long) fn, symbol ? symbol : "??" );
     }

     if (level == 0) {
          /* stack unrolled, print end, unlock */
          direct_log_printf( NULL, "\n" );
          direct_log_unlock( NULL );
     }
}

__attribute__((no_instrument_function))
void
direct_trace_print_stack( DirectTraceBuffer *buffer )
{
     int     level;

     if (!direct_config->trace)
          return;

     if (!buffer)
          buffer = get_trace_buffer();

     if (buffer->in_trace)
          return;

     buffer->in_trace = true;


     level = buffer->level;
     if (level > MAX_LEVEL) {
          D_WARN( "only showing %d of %d items", MAX_LEVEL, level );
          level = MAX_LEVEL;
     }
     else if (level == 0) {
          buffer->in_trace = false;
          return;
     }

     /* we are starting with the last entry (0) */
     direct_trace_print_stack_recursive( buffer, 0, level );

     buffer->in_trace = false;
}

__attribute__((no_instrument_function))
void
direct_trace_print_stacks( void )
{
     int                i;
     DirectTraceBuffer *buffer = get_trace_buffer();

     if (buffer->level)
          direct_trace_print_stack( buffer );

     pthread_mutex_lock( &buffers_lock );

     for (i=0; i<buffers_num; i++) {
          if (buffers[i] != buffer && buffers[i]->level)
               direct_trace_print_stack( buffers[i] );
     }

     pthread_mutex_unlock( &buffers_lock );
}

__attribute__((no_instrument_function))
int
direct_trace_debug_indent( void )
{
     int                in;
     DirectTraceBuffer *buffer = get_trace_buffer();
     int                level  = buffer->level - 1;

     if (level < 0)
          return 0;

     buffer->trace[level--].flags |= TF_DEBUG;

     for (in=0; level>=0; level--) {
          if (buffer->trace[level].flags & TF_DEBUG)
               in++;
     }

     return in;
}

__attribute__((no_instrument_function))
DirectTraceBuffer *
direct_trace_copy_buffer( DirectTraceBuffer *buffer )
{
     int                level;
     DirectTraceBuffer *copy;

     if (!buffer)
          buffer = get_trace_buffer();

     level = buffer->level;
     if (level > MAX_LEVEL) {
          D_WARN( "only copying %d of %d items", MAX_LEVEL, level );
          level = MAX_LEVEL;
     }

     copy = calloc( 1, sizeof(*buffer) - sizeof(buffer->trace) + sizeof(buffer->trace[0]) * level );
     if (!copy)
          return NULL;

     if (buffer->name)
          copy->name = strdup( buffer->name );

     copy->tid   = buffer->tid;
     copy->level = buffer->level;

     direct_memcpy( copy->trace, buffer->trace, level * sizeof(buffer->trace[0]) );

     return copy;
}

__attribute__((no_instrument_function))
void
direct_trace_free_buffer( DirectTraceBuffer *buffer )
{
     if (buffer->name)
          free( buffer->name );

     free( buffer );
}

/**************************************************************************************************/

__attribute__((no_instrument_function))
void
__cyg_profile_func_enter (void *this_fn,
                          void *call_site)
{
     if (direct_config->trace) {
          DirectTraceBuffer *buffer = get_trace_buffer();
          int                level  = buffer->level++;
          Trace             *trace  = &buffer->trace[level];

          if (level < MAX_LEVEL) {
               trace->addr  = this_fn;
               trace->flags = TF_NONE;
          }
     }
}

__attribute__((no_instrument_function))
void
__cyg_profile_func_exit (void *this_fn,
                         void *call_site)
{
     if (direct_config->trace) {
          DirectTraceBuffer *buffer = get_trace_buffer();

          if (buffer->level > 0)
               buffer->level--;
     }
}

#else

const char *
direct_trace_lookup_symbol( const char *filename, long offset )
{
     return NULL;
}

const char *
direct_trace_lookup_file( void *address, void **ret_base )
{
     if (ret_base)
          *ret_base = NULL;
          
     return NULL;
}

void
direct_trace_print_stack( DirectTraceBuffer *buffer )
{
}

void
direct_trace_print_stacks( void )
{
}

int
direct_trace_debug_indent( void )
{
     return 0;
}

DirectTraceBuffer *
direct_trace_copy_buffer( DirectTraceBuffer *buffer )
{
     return NULL;
}

void
direct_trace_free_buffer( DirectTraceBuffer *buffer )
{
}

#endif

