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

#include <direct/clock.h>
#include <direct/debug.h>
#include <direct/util.h>

#include <sys/time.h>
#include <time.h>

D_DEBUG_DOMAIN( Direct_Clock, "Direct/Clock", "Time measurement etc." );

static struct timeval start_time = { 0, 0 };

long long
direct_clock_get_micros( void )
{
     struct timeval tv;
     long long      ret;

     if (start_time.tv_sec == 0) {
          gettimeofday( &start_time, NULL );
          return 0;
     }

     gettimeofday( &tv, NULL );

     ret = (long long)(tv.tv_sec - start_time.tv_sec) * 1000000LL +
           (long long)(tv.tv_usec - start_time.tv_usec);
     if (ret < 0) {
          D_DEBUG_AT( Direct_Clock, "Clock skew detected (%lld in the past)!\n", -ret );
          start_time = tv;
          ret = 0;
     }

     return ret;
}

long long
direct_clock_get_millis( void )
{
     return direct_clock_get_micros() / 1000LL;
}

void
direct_clock_set_start( const struct timeval *start )
{
     long long diff;

     D_ASSERT( start != NULL );

     diff = (long long)(start->tv_sec  - start_time.tv_sec)  * 1000LL +
            (long long)(start->tv_usec - start_time.tv_usec) / 1000LL;

     D_DEBUG_AT( Direct_Clock, "Adjusting start time "
                 "(%lld.%lld seconds diff)\n", diff / 1000LL, ABS(diff) % 1000LL );

     start_time = *start;
}

long long
direct_clock_get_abs_micros( void )
{
     struct timeval tv;

     gettimeofday( &tv, NULL );

     return (long long)(tv.tv_sec) * 1000000LL + (long long)(tv.tv_usec);
}

long long
direct_clock_get_abs_millis( void )
{
     return direct_clock_get_abs_micros() / 1000LL;
}

