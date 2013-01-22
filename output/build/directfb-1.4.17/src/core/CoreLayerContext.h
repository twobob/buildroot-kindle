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

#ifndef ___CoreLayerContext__H___
#define ___CoreLayerContext__H___

#include <core/CoreLayerContext_includes.h>

/**********************************************************************************************************************
 * CoreLayerContext
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreLayerContext_GetPrimaryRegion(
                    CoreLayerContext                          *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region);

DFBResult CoreLayerContext_TestConfiguration(
                    CoreLayerContext                          *obj,
                    const DFBDisplayLayerConfig               *config,
                    DFBDisplayLayerConfigFlags                *ret_failed);

DFBResult CoreLayerContext_SetConfiguration(
                    CoreLayerContext                          *obj,
                    const DFBDisplayLayerConfig               *config);

DFBResult CoreLayerContext_SetSrcColorKey(
                    CoreLayerContext                          *obj,
                    const DFBColorKey                         *key);

DFBResult CoreLayerContext_SetDstColorKey(
                    CoreLayerContext                          *obj,
                    const DFBColorKey                         *key);

DFBResult CoreLayerContext_SetSourceRectangle(
                    CoreLayerContext                          *obj,
                    const DFBRectangle                        *rectangle);

DFBResult CoreLayerContext_SetScreenLocation(
                    CoreLayerContext                          *obj,
                    const DFBLocation                         *location);

DFBResult CoreLayerContext_SetScreenRectangle(
                    CoreLayerContext                          *obj,
                    const DFBRectangle                        *rectangle);

DFBResult CoreLayerContext_SetScreenPosition(
                    CoreLayerContext                          *obj,
                    const DFBPoint                            *position);

DFBResult CoreLayerContext_SetOpacity(
                    CoreLayerContext                          *obj,
                    u8                                         opacity);

DFBResult CoreLayerContext_SetRotation(
                    CoreLayerContext                          *obj,
                    s32                                        rotation);

DFBResult CoreLayerContext_SetColorAdjustment(
                    CoreLayerContext                          *obj,
                    const DFBColorAdjustment                  *adjustment);

DFBResult CoreLayerContext_SetFieldParity(
                    CoreLayerContext                          *obj,
                    u32                                        field);

DFBResult CoreLayerContext_SetClipRegions(
                    CoreLayerContext                          *obj,
                    const DFBRegion                           *regions,
                    u32                                        num,
                    bool                                       positive);

DFBResult CoreLayerContext_CreateWindow(
                    CoreLayerContext                          *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window);

DFBResult CoreLayerContext_FindWindow(
                    CoreLayerContext                          *obj,
                    DFBWindowID                                window_id,
                    CoreWindow                               **ret_window);

DFBResult CoreLayerContext_FindWindowByResourceID(
                    CoreLayerContext                          *obj,
                    u64                                        resource_id,
                    CoreWindow                               **ret_window);


void CoreLayerContext_Init_Dispatch(
                    CoreDFB              *core,
                    CoreLayerContext     *obj,
                    FusionCall           *call
);

void  CoreLayerContext_Deinit_Dispatch(
                    FusionCall           *call
);


#ifdef __cplusplus
}
#endif




/*
 * CoreLayerContext Calls
 */
typedef enum {
    _CoreLayerContext_GetPrimaryRegion = 1,
    _CoreLayerContext_TestConfiguration = 2,
    _CoreLayerContext_SetConfiguration = 3,
    _CoreLayerContext_SetSrcColorKey = 4,
    _CoreLayerContext_SetDstColorKey = 5,
    _CoreLayerContext_SetSourceRectangle = 6,
    _CoreLayerContext_SetScreenLocation = 7,
    _CoreLayerContext_SetScreenRectangle = 8,
    _CoreLayerContext_SetScreenPosition = 9,
    _CoreLayerContext_SetOpacity = 10,
    _CoreLayerContext_SetRotation = 11,
    _CoreLayerContext_SetColorAdjustment = 12,
    _CoreLayerContext_SetFieldParity = 13,
    _CoreLayerContext_SetClipRegions = 14,
    _CoreLayerContext_CreateWindow = 15,
    _CoreLayerContext_FindWindow = 16,
    _CoreLayerContext_FindWindowByResourceID = 17,
} CoreLayerContextCall;

/*
 * CoreLayerContext_GetPrimaryRegion
 */
typedef struct {
    bool                                       create;
} CoreLayerContextGetPrimaryRegion;

typedef struct {
    DFBResult                                  result;
    u32                                        region_id;
} CoreLayerContextGetPrimaryRegionReturn;


/*
 * CoreLayerContext_TestConfiguration
 */
typedef struct {
    DFBDisplayLayerConfig                      config;
} CoreLayerContextTestConfiguration;

typedef struct {
    DFBResult                                  result;
    DFBDisplayLayerConfigFlags                 failed;
} CoreLayerContextTestConfigurationReturn;


/*
 * CoreLayerContext_SetConfiguration
 */
typedef struct {
    DFBDisplayLayerConfig                      config;
} CoreLayerContextSetConfiguration;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetConfigurationReturn;


/*
 * CoreLayerContext_SetSrcColorKey
 */
typedef struct {
    DFBColorKey                                key;
} CoreLayerContextSetSrcColorKey;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetSrcColorKeyReturn;


/*
 * CoreLayerContext_SetDstColorKey
 */
typedef struct {
    DFBColorKey                                key;
} CoreLayerContextSetDstColorKey;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetDstColorKeyReturn;


/*
 * CoreLayerContext_SetSourceRectangle
 */
typedef struct {
    DFBRectangle                               rectangle;
} CoreLayerContextSetSourceRectangle;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetSourceRectangleReturn;


/*
 * CoreLayerContext_SetScreenLocation
 */
typedef struct {
    DFBLocation                                location;
} CoreLayerContextSetScreenLocation;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetScreenLocationReturn;


/*
 * CoreLayerContext_SetScreenRectangle
 */
typedef struct {
    DFBRectangle                               rectangle;
} CoreLayerContextSetScreenRectangle;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetScreenRectangleReturn;


/*
 * CoreLayerContext_SetScreenPosition
 */
typedef struct {
    DFBPoint                                   position;
} CoreLayerContextSetScreenPosition;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetScreenPositionReturn;


/*
 * CoreLayerContext_SetOpacity
 */
typedef struct {
    u8                                         opacity;
} CoreLayerContextSetOpacity;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetOpacityReturn;


/*
 * CoreLayerContext_SetRotation
 */
typedef struct {
    s32                                        rotation;
} CoreLayerContextSetRotation;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetRotationReturn;


/*
 * CoreLayerContext_SetColorAdjustment
 */
typedef struct {
    DFBColorAdjustment                         adjustment;
} CoreLayerContextSetColorAdjustment;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetColorAdjustmentReturn;


/*
 * CoreLayerContext_SetFieldParity
 */
typedef struct {
    u32                                        field;
} CoreLayerContextSetFieldParity;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetFieldParityReturn;


/*
 * CoreLayerContext_SetClipRegions
 */
typedef struct {
    u32                                        num;
    bool                                       positive;
    /* 'num' DFBRegion follow (regions) */
} CoreLayerContextSetClipRegions;

typedef struct {
    DFBResult                                  result;
} CoreLayerContextSetClipRegionsReturn;


/*
 * CoreLayerContext_CreateWindow
 */
typedef struct {
    DFBWindowDescription                       description;
    bool                                       parent_set;
    u32                                        parent_id;
    bool                                       toplevel_set;
    u32                                        toplevel_id;
} CoreLayerContextCreateWindow;

typedef struct {
    DFBResult                                  result;
    u32                                        window_id;
} CoreLayerContextCreateWindowReturn;


/*
 * CoreLayerContext_FindWindow
 */
typedef struct {
    DFBWindowID                                window_id;
} CoreLayerContextFindWindow;

typedef struct {
    DFBResult                                  result;
    u32                                        window_id;
} CoreLayerContextFindWindowReturn;


/*
 * CoreLayerContext_FindWindowByResourceID
 */
typedef struct {
    u64                                        resource_id;
} CoreLayerContextFindWindowByResourceID;

typedef struct {
    DFBResult                                  result;
    u32                                        window_id;
} CoreLayerContextFindWindowByResourceIDReturn;


DFBResult ILayerContext_Real__GetPrimaryRegion( CoreLayerContext *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region );

DFBResult ILayerContext_Real__TestConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config,
                    DFBDisplayLayerConfigFlags                *ret_failed );

DFBResult ILayerContext_Real__SetConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config );

DFBResult ILayerContext_Real__SetSrcColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key );

DFBResult ILayerContext_Real__SetDstColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key );

DFBResult ILayerContext_Real__SetSourceRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle );

DFBResult ILayerContext_Real__SetScreenLocation( CoreLayerContext *obj,
                    const DFBLocation                         *location );

DFBResult ILayerContext_Real__SetScreenRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle );

DFBResult ILayerContext_Real__SetScreenPosition( CoreLayerContext *obj,
                    const DFBPoint                            *position );

DFBResult ILayerContext_Real__SetOpacity( CoreLayerContext *obj,
                    u8                                         opacity );

DFBResult ILayerContext_Real__SetRotation( CoreLayerContext *obj,
                    s32                                        rotation );

DFBResult ILayerContext_Real__SetColorAdjustment( CoreLayerContext *obj,
                    const DFBColorAdjustment                  *adjustment );

DFBResult ILayerContext_Real__SetFieldParity( CoreLayerContext *obj,
                    u32                                        field );

DFBResult ILayerContext_Real__SetClipRegions( CoreLayerContext *obj,
                    const DFBRegion                           *regions,
                    u32                                        num,
                    bool                                       positive );

DFBResult ILayerContext_Real__CreateWindow( CoreLayerContext *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window );

DFBResult ILayerContext_Real__FindWindow( CoreLayerContext *obj,
                    DFBWindowID                                window_id,
                    CoreWindow                               **ret_window );

DFBResult ILayerContext_Real__FindWindowByResourceID( CoreLayerContext *obj,
                    u64                                        resource_id,
                    CoreWindow                               **ret_window );

DFBResult ILayerContext_Requestor__GetPrimaryRegion( CoreLayerContext *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region );

DFBResult ILayerContext_Requestor__TestConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config,
                    DFBDisplayLayerConfigFlags                *ret_failed );

DFBResult ILayerContext_Requestor__SetConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config );

DFBResult ILayerContext_Requestor__SetSrcColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key );

DFBResult ILayerContext_Requestor__SetDstColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key );

DFBResult ILayerContext_Requestor__SetSourceRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle );

DFBResult ILayerContext_Requestor__SetScreenLocation( CoreLayerContext *obj,
                    const DFBLocation                         *location );

DFBResult ILayerContext_Requestor__SetScreenRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle );

DFBResult ILayerContext_Requestor__SetScreenPosition( CoreLayerContext *obj,
                    const DFBPoint                            *position );

DFBResult ILayerContext_Requestor__SetOpacity( CoreLayerContext *obj,
                    u8                                         opacity );

DFBResult ILayerContext_Requestor__SetRotation( CoreLayerContext *obj,
                    s32                                        rotation );

DFBResult ILayerContext_Requestor__SetColorAdjustment( CoreLayerContext *obj,
                    const DFBColorAdjustment                  *adjustment );

DFBResult ILayerContext_Requestor__SetFieldParity( CoreLayerContext *obj,
                    u32                                        field );

DFBResult ILayerContext_Requestor__SetClipRegions( CoreLayerContext *obj,
                    const DFBRegion                           *regions,
                    u32                                        num,
                    bool                                       positive );

DFBResult ILayerContext_Requestor__CreateWindow( CoreLayerContext *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window );

DFBResult ILayerContext_Requestor__FindWindow( CoreLayerContext *obj,
                    DFBWindowID                                window_id,
                    CoreWindow                               **ret_window );

DFBResult ILayerContext_Requestor__FindWindowByResourceID( CoreLayerContext *obj,
                    u64                                        resource_id,
                    CoreWindow                               **ret_window );


DFBResult CoreLayerContextDispatch__Dispatch( CoreLayerContext *obj,
                    FusionID      caller,
                    int           method,
                    void         *ptr,
                    unsigned int  length,
                    void         *ret_ptr,
                    unsigned int  ret_size,
                    unsigned int *ret_length );


#endif
