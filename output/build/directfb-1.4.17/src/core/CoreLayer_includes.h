#include "CoreDFB_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


#include <core/layer_context.h>
#include <core/layers_internal.h>
#include <core/windows.h>
#include <core/windows_internal.h>


static __inline__ DirectResult
CoreLayer_Call( CoreLayer           *layer,
                FusionCallExecFlags  flags,
                int                  call_arg,
                void                *ptr,
                unsigned int         length,
                void                *ret_ptr,
                unsigned int         ret_size,
                unsigned int        *ret_length )
{
     D_ASSERT( layer != NULL );
     D_ASSERT( layer->shared != NULL );

     return fusion_call_execute3( &layer->shared->call, flags, call_arg, ptr, length, ret_ptr, ret_size, ret_length );
}

#ifdef __cplusplus
}
#endif

