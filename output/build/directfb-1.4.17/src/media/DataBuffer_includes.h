#ifndef __DataBuffer_includes_h__
#define __DataBuffer_includes_h__

#ifdef __cplusplus
extern "C" {
#endif


#include <core/coretypes.h>

#include <fusion/types.h>
#include <fusion/lock.h>
#include <fusion/object.h>

#include <directfb.h>

typedef struct {
     FusionCall     call;
} DataBuffer;

static __inline__ DirectResult
DataBuffer_Call( DataBuffer             *buffer,
              FusionCallExecFlags  flags,
              int                  call_arg,
              void                *ptr,
              unsigned int         length,
              void                *ret_ptr,
              unsigned int         ret_size,
              unsigned int        *ret_length )
{
     return fusion_call_execute3( &buffer->call, flags, call_arg, ptr, length, ret_ptr, ret_size, ret_length );
}


#ifdef __cplusplus
}
#endif


#endif

