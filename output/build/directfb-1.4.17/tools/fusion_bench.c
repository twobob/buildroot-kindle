/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <stdio.h>
#include <unistd.h>

#include <sys/file.h>

#include <pthread.h>

#include <direct/clock.h>

#include <fusion/build.h>
#include <fusion/fusion.h>
#include <fusion/lock.h>
#include <fusion/property.h>
#include <fusion/reactor.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>

#include <directfb.h>

#include <core/system.h>


static long long     t1, t2;
static unsigned int  loops;
static FusionWorld  *world;

#define BENCH_START()       do { sync(); usleep(100000); sync(); t1 = direct_clock_get_millis(); loops = 0; } while (0)
#define BENCH_STOP()        do { t2 = direct_clock_get_millis(); } while (0)

#define BENCH_LOOP()        while ((++loops & 0xfff) || (direct_clock_get_millis() - t1 < 1000))

#define BENCH_RESULT()      (loops / (float)(t2 - t1))
#define BENCH_RESULT_BY(x)  ((loops * x) / (float)(t2 - t1))


static ReactionResult
reaction_callback (const void *msg_data,
                   void       *ctx)
{
     return RS_OK;
}

static void
bench_reactor( void )
{
     FusionReactor  *reactor;
     Reaction        reaction;
     Reaction        reaction2;
     GlobalReaction  global_reaction;

     reactor = fusion_reactor_new( 16, "Benchmark", world );
     if (!reactor) {
          fprintf( stderr, "Fusion Error\n" );
          return;
     }


     /* reactor attach/detach */
     BENCH_START();

     BENCH_LOOP() {
          fusion_reactor_attach( reactor, reaction_callback, NULL, &reaction );
          fusion_reactor_detach( reactor, &reaction );
     }

     BENCH_STOP();

     printf( "reactor attach/detach                 -> %8.2f k/sec\n", BENCH_RESULT() );


     /* reactor attach/detach (2nd) */
     fusion_reactor_attach( reactor, reaction_callback, NULL, &reaction );

     BENCH_START();

     BENCH_LOOP() {
          fusion_reactor_attach( reactor, reaction_callback, NULL, &reaction2 );
          fusion_reactor_detach( reactor, &reaction2 );
     }

     BENCH_STOP();

     fusion_reactor_detach( reactor, &reaction );

     printf( "reactor attach/detach (2nd)           -> %8.2f k/sec\n", BENCH_RESULT() );


     /* reactor attach/detach (global) */
     fusion_reactor_attach( reactor, reaction_callback, NULL, &reaction );

     BENCH_START();

     BENCH_LOOP() {
          fusion_reactor_attach_global( reactor, 0, NULL, &global_reaction );
          fusion_reactor_detach_global( reactor, &global_reaction );
     }

     BENCH_STOP();

     fusion_reactor_detach( reactor, &reaction );

     printf( "reactor attach/detach (global)        -> %8.2f k/sec\n", BENCH_RESULT() );


     /* reactor dispatch */
     fusion_reactor_attach( reactor, reaction_callback, NULL, &reaction );

     BENCH_START();

     BENCH_LOOP() {
          char msg[16];

          fusion_reactor_dispatch( reactor, msg, true, NULL );
     }

     BENCH_STOP();

     printf( "reactor dispatch                      -> %8.2f k/sec\n", BENCH_RESULT() );


     fusion_reactor_detach( reactor, &reaction );


     fusion_reactor_free( reactor );

     printf( "\n" );
}

static void
bench_ref( void )
{
     DirectResult ret;
     FusionRef    ref;

     ret = fusion_ref_init( &ref, "Benchmark", world );
     if (ret) {
          fprintf( stderr, "Fusion Error %d\n", ret );
          return;
     }


     /* ref up/down (local) */
     BENCH_START();

     BENCH_LOOP() {
          fusion_ref_up( &ref, false );
          fusion_ref_down( &ref, false );
     }

     BENCH_STOP();

     printf( "ref up/down (local)                   -> %8.2f k/sec\n", BENCH_RESULT() );


     /* ref up/down (global) */
     BENCH_START();

     BENCH_LOOP() {
          fusion_ref_up( &ref, true );
          fusion_ref_down( &ref, true );
     }

     BENCH_STOP();

     printf( "ref up/down (global)                  -> %8.2f k/sec\n", BENCH_RESULT() );


     fusion_ref_destroy( &ref );

     printf( "\n" );
}

static void
bench_property( void )
{
     DirectResult   ret;
     FusionProperty property;

     ret = fusion_property_init( &property, world );
     if (ret) {
          fprintf( stderr, "Fusion Error %d\n", ret );
          return;
     }


     /* property lease/cede */
     BENCH_START();

     BENCH_LOOP() {
          fusion_property_lease( &property );
          fusion_property_cede( &property );
     }

     BENCH_STOP();

     printf( "property lease/cede                   -> %8.2f k/sec\n", BENCH_RESULT() );


     fusion_property_destroy( &property );

     printf( "\n" );
}

static void
bench_skirmish( void )
{
     DirectResult   ret;
     FusionSkirmish skirmish;

     ret = fusion_skirmish_init( &skirmish, "Benchmark", world );
     if (ret) {
          fprintf( stderr, "Fusion Error %d\n", ret );
          return;
     }


     /* skirmish prevail/dismiss */
     BENCH_START();

     BENCH_LOOP() {
          fusion_skirmish_prevail( &skirmish );
          fusion_skirmish_dismiss( &skirmish );
     }

     BENCH_STOP();

     printf( "skirmish prevail/dismiss              -> %8.2f k/sec\n", BENCH_RESULT() );


     fusion_skirmish_destroy( &skirmish );

     printf( "\n" );
}

static void *
prevail_dismiss_loop( void *arg )
{
     FusionSkirmish *skirmish = (FusionSkirmish *) arg;

     BENCH_LOOP() {
          fusion_skirmish_prevail( skirmish );
          fusion_skirmish_dismiss( skirmish );
     }

     return NULL;
}

static void
bench_skirmish_threaded( void )
{
     int            i;
     DirectResult   ret;
     FusionSkirmish skirmish;

     ret = fusion_skirmish_init( &skirmish, "Threaded Benchmark", world );
     if (ret) {
          fprintf( stderr, "Fusion Error %d\n", ret );
          return;
     }


     /* skirmish prevail/dismiss (2-5 threads) */
     for (i=2; i<=5; i++) {
          int       t;
          pthread_t threads[i];

          BENCH_START();

          for (t=0; t<i; t++)
               pthread_create( &threads[t], NULL, prevail_dismiss_loop, &skirmish );

          for (t=0; t<i; t++)
               pthread_join( threads[t], NULL );

          BENCH_STOP();

          printf( "skirmish prevail/dismiss (%d threads)  -> %8.2f k/sec\n", i, BENCH_RESULT() );
     }


     fusion_skirmish_destroy( &skirmish );

     printf( "\n" );
}

static void *
mutex_lock_unlock_loop( void *arg )
{
     pthread_mutex_t *lock = (pthread_mutex_t *) arg;

     BENCH_LOOP() {
          pthread_mutex_lock( lock );
          pthread_mutex_unlock( lock );
     }

     return NULL;
}

static void
bench_mutex_threaded( void )
{
     int                 i;
     pthread_mutex_t     lock;
     pthread_mutexattr_t attr;

     pthread_mutexattr_init( &attr );
     pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

     pthread_mutex_init( &lock, &attr );

     pthread_mutexattr_destroy( &attr );


     /* mutex lock/unlock (2-5 threads) */
     for (i=2; i<=5; i++) {
          int       t;
          pthread_t threads[i];

          BENCH_START();

          for (t=0; t<i; t++)
               pthread_create( &threads[t], NULL, mutex_lock_unlock_loop, &lock );

          for (t=0; t<i; t++)
               pthread_join( threads[t], NULL );

          BENCH_STOP();

          printf( "mutex lock/unlock (rec., %d threads)   -> %8.2f k/sec\n", i, BENCH_RESULT() );
     }


     pthread_mutex_destroy( &lock );

     printf( "\n" );
}

static void
bench_mutex( void )
{
     pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
     pthread_mutex_t     rmutex;
     pthread_mutexattr_t attr;

     pthread_mutexattr_init( &attr );
     pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

     pthread_mutex_init( &rmutex, &attr );

     pthread_mutexattr_destroy( &attr );


     /* pthread_mutex lock/unlock */
     BENCH_START();

     BENCH_LOOP() {
          pthread_mutex_lock( &mutex );
          pthread_mutex_unlock( &mutex );
     }

     BENCH_STOP();

     printf( "mutex lock/unlock                     -> %8.2f k/sec\n", BENCH_RESULT() );


     /* pthread_mutex lock/unlock */
     BENCH_START();

     BENCH_LOOP() {
          pthread_mutex_lock( &rmutex );
          pthread_mutex_unlock( &rmutex );
     }

     BENCH_STOP();

     printf( "mutex lock/unlock (recursive)         -> %8.2f k/sec\n", BENCH_RESULT() );


     pthread_mutex_destroy( &mutex );
     pthread_mutex_destroy( &rmutex );

     printf( "\n" );
}

static void
bench_flock( void )
{
     int   fd;
     FILE *tmp;

     tmp = tmpfile();
     if (!tmp) {
          perror( "tmpfile()" );
          return;
     }

     fd = fileno( tmp );
     if (fd < 0) {
          perror( "fileno()" );
          fclose( tmp );
          return;
     }

     BENCH_START();

     BENCH_LOOP() {
          flock( fd, LOCK_EX );
          flock( fd, LOCK_UN );
     }

     BENCH_STOP();

     printf( "flock lock/unlock                     -> %8.2f k/sec\n", BENCH_RESULT() );
     printf( "\n" );

     fclose( tmp );
}

static void
bench_shmpool( bool debug )
{
     DirectResult  ret;
     void         *mem[256];
     const int     sizes[8] = { 12, 36, 200, 120, 39, 3082, 8, 1040 };

     FusionSHMPoolShared *pool;

     ret = fusion_shm_pool_create( world, "Benchmark Pool", 524288, debug, &pool );
     if (ret) {
          DirectFBError( "fusion_shm_pool_create() failed", ret );
          return;
     }

     BENCH_START();

     BENCH_LOOP() {
          int i;

          for (i=0; i<128; i++)
               mem[i] = SHMALLOC( pool, sizes[i&7] );

          for (i=0; i<64; i++)
               SHFREE( pool, mem[i] );

          for (i=128; i<192; i++)
               mem[i] = SHMALLOC( pool, sizes[i&7] );

          for (i=64; i<128; i++)
               SHFREE( pool, mem[i] );

          for (i=192; i<256; i++)
               mem[i] = SHMALLOC( pool, sizes[i&7] );

          for (i=128; i<256; i++)
               SHFREE( pool, mem[i] );
     }

     BENCH_STOP();

     printf( "shm pool alloc/free %s           -> %8.2f k/sec\n",
             debug ? "(debug)" : "       ", BENCH_RESULT_BY(256) );

     fusion_shm_pool_destroy( world, pool );
}

int
main( int argc, char *argv[] )
{
     DirectResult ret;

     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret)
          return DirectFBError( "DirectFBInit()", ret );

     dfb_system_lookup();

     ret = fusion_enter( -1, 0, FER_MASTER, &world );
     if (ret)
          return DirectFBError( "fusion_enter()", ret );

     printf( "\n" );

#if FUSION_BUILD_MULTI
     printf( "Fusion Benchmark (Multi Application Core)\n" );
#else
     printf( "Fusion Benchmark (Single Application Core)\n" );
#endif

     printf( "\n" );

     bench_flock();

     bench_mutex();
     bench_mutex_threaded();

     bench_skirmish();
     bench_skirmish_threaded();

     //bench_spinlock_threaded();

     bench_property();

     bench_ref();

     bench_reactor();

     bench_shmpool( false );
     bench_shmpool( true );

     printf( "\n" );

     fusion_exit( world, false );

     return 0;
}

