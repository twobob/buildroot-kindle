/*
   (c) Copyright 2001-2011  The world wide DirectFB Open Source Community (directfb.org)
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

#ifndef ___CoreSurface__H___
#define ___CoreSurface__H___

#include <core/CoreSurface_includes.h>

/**********************************************************************************************************************
 * CoreSurface
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreSurface_SetConfig(
                    CoreSurface                               *obj,
                    const CoreSurfaceConfig                   *config);

DFBResult CoreSurface_Flip(
                    CoreSurface                               *obj,
                    bool                                       swap);

DFBResult CoreSurface_GetPalette(
                    CoreSurface                               *obj,
                    CorePalette                              **ret_palette);

DFBResult CoreSurface_SetPalette(
                    CoreSurface                               *obj,
                    CorePalette                               *palette);

DFBResult CoreSurface_SetAlphaRamp(
                    CoreSurface                               *obj,
                    u8                                         a0,
                    u8                                         a1,
                    u8                                         a2,
                    u8                                         a3);

DFBResult CoreSurface_SetField(
                    CoreSurface                               *obj,
                    s32                                        field);

DFBResult CoreSurface_PreLockBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    CoreSurfaceAllocation                    **ret_allocation);

DFBResult CoreSurface_PreLockBuffer2(
                    CoreSurface                               *obj,
                    CoreSurfaceBufferRole                      role,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    bool                                       lock,
                    CoreSurfaceAllocation                    **ret_allocation);

DFBResult CoreSurface_PreReadBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation);

DFBResult CoreSurface_PreWriteBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation);


void CoreSurface_Init_Dispatch(
                    CoreDFB              *core,
                    CoreSurface          *obj,
                    FusionCall           *call
);

void  CoreSurface_Deinit_Dispatch(
                    FusionCall           *call
);


#ifdef __cplusplus
}
#endif




/*
 * CoreSurface Calls
 */
typedef enum {
    _CoreSurface_SetConfig = 1,
    _CoreSurface_Flip = 2,
    _CoreSurface_GetPalette = 3,
    _CoreSurface_SetPalette = 4,
    _CoreSurface_SetAlphaRamp = 5,
    _CoreSurface_SetField = 6,
    _CoreSurface_PreLockBuffer = 7,
    _CoreSurface_PreLockBuffer2 = 8,
    _CoreSurface_PreReadBuffer = 9,
    _CoreSurface_PreWriteBuffer = 10,
} CoreSurfaceCall;

/*
 * CoreSurface_SetConfig
 */
typedef struct {
    CoreSurfaceConfig                          config;
} CoreSurfaceSetConfig;

typedef struct {
    DFBResult                                  result;
} CoreSurfaceSetConfigReturn;


/*
 * CoreSurface_Flip
 */
typedef struct {
    bool                                       swap;
} CoreSurfaceFlip;

typedef struct {
    DFBResult                                  result;
} CoreSurfaceFlipReturn;


/*
 * CoreSurface_GetPalette
 */
typedef struct {
} CoreSurfaceGetPalette;

typedef struct {
    DFBResult                                  result;
    u32                                        palette_id;
} CoreSurfaceGetPaletteReturn;


/*
 * CoreSurface_SetPalette
 */
typedef struct {
    u32                                        palette_id;
} CoreSurfaceSetPalette;

typedef struct {
    DFBResult                                  result;
} CoreSurfaceSetPaletteReturn;


/*
 * CoreSurface_SetAlphaRamp
 */
typedef struct {
    u8                                         a0;
    u8                                         a1;
    u8                                         a2;
    u8                                         a3;
} CoreSurfaceSetAlphaRamp;

typedef struct {
    DFBResult                                  result;
} CoreSurfaceSetAlphaRampReturn;


/*
 * CoreSurface_SetField
 */
typedef struct {
    s32                                        field;
} CoreSurfaceSetField;

typedef struct {
    DFBResult                                  result;
} CoreSurfaceSetFieldReturn;


/*
 * CoreSurface_PreLockBuffer
 */
typedef struct {
    u32                                        buffer_id;
    CoreSurfaceAccessorID                      accessor;
    CoreSurfaceAccessFlags                     access;
} CoreSurfacePreLockBuffer;

typedef struct {
    DFBResult                                  result;
    u32                                        allocation_id;
} CoreSurfacePreLockBufferReturn;


/*
 * CoreSurface_PreLockBuffer2
 */
typedef struct {
    CoreSurfaceBufferRole                      role;
    CoreSurfaceAccessorID                      accessor;
    CoreSurfaceAccessFlags                     access;
    bool                                       lock;
} CoreSurfacePreLockBuffer2;

typedef struct {
    DFBResult                                  result;
    u32                                        allocation_id;
} CoreSurfacePreLockBuffer2Return;


/*
 * CoreSurface_PreReadBuffer
 */
typedef struct {
    u32                                        buffer_id;
    DFBRectangle                               rect;
} CoreSurfacePreReadBuffer;

typedef struct {
    DFBResult                                  result;
    u32                                        allocation_id;
} CoreSurfacePreReadBufferReturn;


/*
 * CoreSurface_PreWriteBuffer
 */
typedef struct {
    u32                                        buffer_id;
    DFBRectangle                               rect;
} CoreSurfacePreWriteBuffer;

typedef struct {
    DFBResult                                  result;
    u32                                        allocation_id;
} CoreSurfacePreWriteBufferReturn;


DFBResult ISurface_Real__SetConfig( CoreSurface *obj,
                    const CoreSurfaceConfig                   *config );

DFBResult ISurface_Real__Flip( CoreSurface *obj,
                    bool                                       swap );

DFBResult ISurface_Real__GetPalette( CoreSurface *obj,
                    CorePalette                              **ret_palette );

DFBResult ISurface_Real__SetPalette( CoreSurface *obj,
                    CorePalette                               *palette );

DFBResult ISurface_Real__SetAlphaRamp( CoreSurface *obj,
                    u8                                         a0,
                    u8                                         a1,
                    u8                                         a2,
                    u8                                         a3 );

DFBResult ISurface_Real__SetField( CoreSurface *obj,
                    s32                                        field );

DFBResult ISurface_Real__PreLockBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Real__PreLockBuffer2( CoreSurface *obj,
                    CoreSurfaceBufferRole                      role,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    bool                                       lock,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Real__PreReadBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Real__PreWriteBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Requestor__SetConfig( CoreSurface *obj,
                    const CoreSurfaceConfig                   *config );

DFBResult ISurface_Requestor__Flip( CoreSurface *obj,
                    bool                                       swap );

DFBResult ISurface_Requestor__GetPalette( CoreSurface *obj,
                    CorePalette                              **ret_palette );

DFBResult ISurface_Requestor__SetPalette( CoreSurface *obj,
                    CorePalette                               *palette );

DFBResult ISurface_Requestor__SetAlphaRamp( CoreSurface *obj,
                    u8                                         a0,
                    u8                                         a1,
                    u8                                         a2,
                    u8                                         a3 );

DFBResult ISurface_Requestor__SetField( CoreSurface *obj,
                    s32                                        field );

DFBResult ISurface_Requestor__PreLockBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Requestor__PreLockBuffer2( CoreSurface *obj,
                    CoreSurfaceBufferRole                      role,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    bool                                       lock,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Requestor__PreReadBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation );

DFBResult ISurface_Requestor__PreWriteBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation );


DFBResult CoreSurfaceDispatch__Dispatch( CoreSurface *obj,
                    FusionID      caller,
                    int           method,
                    void         *ptr,
                    unsigned int  length,
                    void         *ret_ptr,
                    unsigned int  ret_size,
                    unsigned int *ret_length );


#endif
