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
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include <pthread.h>

#include <fusion/reactor.h>
#include <direct/list.h>

#include <directfb.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>
#include <core/windows.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/util.h>

#include <media/idirectfbdatabuffer.h>


static void
IDirectFBDataBuffer_Memory_Destruct( IDirectFBDataBuffer *thiz )
{
     IDirectFBDataBuffer_Destruct( thiz );
}

static DirectResult
IDirectFBDataBuffer_Memory_Release( IDirectFBDataBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer)

     if (--data->ref == 0)
          IDirectFBDataBuffer_Memory_Destruct( thiz );

     return DR_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_Flush( IDirectFBDataBuffer *thiz )
{
     return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDataBuffer_Memory_Finish( IDirectFBDataBuffer *thiz )
{
     return DFB_UNSUPPORTED;
}

static DFBResult
IDirectFBDataBuffer_Memory_SeekTo( IDirectFBDataBuffer *thiz,
                                   unsigned int         offset )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (offset >= data->length)
          return DFB_INVARG;

     data->pos = offset;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_GetPosition( IDirectFBDataBuffer *thiz,
                                        unsigned int        *offset )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (!offset)
          return DFB_INVARG;

     *offset = data->pos;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_GetLength( IDirectFBDataBuffer *thiz,
                                      unsigned int        *length )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (!length)
          return DFB_INVARG;

     *length = data->length;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_WaitForData( IDirectFBDataBuffer *thiz,
                                        unsigned int         length )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (data->pos + length > data->length)
          return DFB_EOF;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_WaitForDataWithTimeout( IDirectFBDataBuffer *thiz,
                                                   unsigned int         length,
                                                   unsigned int         seconds,
                                                   unsigned int         milli_seconds )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (data->pos + length > data->length)
          return DFB_EOF;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_GetData( IDirectFBDataBuffer *thiz,
                                    unsigned int         length,
                                    void                *data_buffer,
                                    unsigned int        *read_out )
{
     unsigned int size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (!data_buffer || !length)
          return DFB_INVARG;

     if (data->pos >= data->length)
          return DFB_EOF;

     size = MIN( length, data->length - data->pos );

     direct_memcpy( data_buffer, data->buffer + data->pos, size );

     data->pos += size;

     if (read_out)
          *read_out = size;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_PeekData( IDirectFBDataBuffer *thiz,
                                     unsigned int         length,
                                     int                  offset,
                                     void                *data_buffer,
                                     unsigned int        *read_out )
{
     unsigned int size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (!data_buffer || !length)
          return DFB_INVARG;

     if (data->pos + offset >= data->length)
          return DFB_EOF;

     size = MIN( length, data->length - data->pos - offset );

     direct_memcpy( data_buffer, data->buffer + data->pos + offset, size );

     if (read_out)
          *read_out = size;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_HasData( IDirectFBDataBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBDataBuffer_Memory)

     if (data->pos >= data->length)
          return DFB_EOF;

     return DFB_OK;
}

static DFBResult
IDirectFBDataBuffer_Memory_PutData( IDirectFBDataBuffer *thiz,
                                    const void          *data_buffer,
                                    unsigned int         length )
{
     return DFB_UNSUPPORTED;
}

DFBResult
IDirectFBDataBuffer_Memory_Construct( IDirectFBDataBuffer *thiz,
                                      const void          *data_buffer,
                                      unsigned int         length,
                                      CoreDFB             *core )
{
     DFBResult ret;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBDataBuffer_Memory)

     ret = IDirectFBDataBuffer_Construct( thiz, NULL, core );
     if (ret)
          return ret;

     data->buffer = data_buffer;
     data->length = length;

     data->base.is_memory = true;

     thiz->Release                = IDirectFBDataBuffer_Memory_Release;
     thiz->Flush                  = IDirectFBDataBuffer_Memory_Flush;
     thiz->Finish                 = IDirectFBDataBuffer_Memory_Finish;
     thiz->SeekTo                 = IDirectFBDataBuffer_Memory_SeekTo;
     thiz->GetPosition            = IDirectFBDataBuffer_Memory_GetPosition;
     thiz->GetLength              = IDirectFBDataBuffer_Memory_GetLength;
     thiz->WaitForData            = IDirectFBDataBuffer_Memory_WaitForData;
     thiz->WaitForDataWithTimeout = IDirectFBDataBuffer_Memory_WaitForDataWithTimeout;
     thiz->GetData                = IDirectFBDataBuffer_Memory_GetData;
     thiz->PeekData               = IDirectFBDataBuffer_Memory_PeekData;
     thiz->HasData                = IDirectFBDataBuffer_Memory_HasData;
     thiz->PutData                = IDirectFBDataBuffer_Memory_PutData;

     return DFB_OK;
}

