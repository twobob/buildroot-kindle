#ifdef SH7722_DEBUG_SCREEN
#define DIRECT_ENABLE_DEBUG
#endif


#include <config.h>

#include <stdio.h>

#include <sys/mman.h>

#include <asm/types.h>

#include <directfb.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>


#include "sh7722.h"
#include "sh7722_screen.h"


D_DEBUG_DOMAIN( SH7722_Screen, "SH772x/Screen", "Renesas SH772x Screen" );

/**********************************************************************************************************************/

static DFBResult
sh7722InitScreen( CoreScreen           *screen,
               CoreGraphicsDevice       *device,
               void                 *driver_data,
               void                 *screen_data,
               DFBScreenDescription *description )
{
     D_DEBUG_AT( SH7722_Screen, "%s()\n", __FUNCTION__ );

     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf( description->name, DFB_SCREEN_DESC_NAME_LENGTH, "SH772x Screen" );

     return DFB_OK;
}

static DFBResult
sh7722GetScreenSize( CoreScreen *screen,
                  void       *driver_data,
                  void       *screen_data,
                  int        *ret_width,
                  int        *ret_height )
{
     SH7722DriverData *sdrv = driver_data;
     SH7722DeviceData *sdev = sdrv->dev;
     D_DEBUG_AT( SH7722_Screen, "%s()\n", __FUNCTION__ );

     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );

     *ret_width  = sdev->lcd_width;
     *ret_height = sdev->lcd_height;

     return DFB_OK;
}

ScreenFuncs sh7722ScreenFuncs = {
     .InitScreen    = sh7722InitScreen,
     .GetScreenSize = sh7722GetScreenSize,
};

