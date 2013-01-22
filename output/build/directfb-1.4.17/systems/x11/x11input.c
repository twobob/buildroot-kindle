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

#include <fusion/types.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <directfb.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/input.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layers_internal.h>
#include <core/system.h>

#include <direct/mem.h>
#include <direct/thread.h>

#include "xwindow.h"

#include "primary.h"
#include "x11.h"

#include <core/input_driver.h>

D_DEBUG_DOMAIN( X11_Input, "X11/Input", "X11 Input/Key/Mouse handling" );

DFB_INPUT_DRIVER( x11input )


/*
 * declaration of private data
 */
typedef struct {
     CoreInputDevice*    device;
     DirectThread*       thread;
     DFBX11*             x11;
     bool                stop;
} X11InputData;


static DFBInputEvent motionX = {
     .type    = DIET_UNKNOWN,
     .axisabs = 0,
};

static DFBInputEvent motionY = {
     .type    = DIET_UNKNOWN,
     .axisabs = 0,
};

static void
motion_compress( int x, int y, const XEvent *xEvent )
{
     if (motionX.axisabs != x) {
          motionX.type    = DIET_AXISMOTION;
          motionX.flags   = DIEF_AXISABS | DIEF_TIMESTAMP;
          motionX.axis    = DIAI_X;
          motionX.axisabs = x;

          motionX.timestamp.tv_sec  =  xEvent->xmotion.time / 1000;
          motionX.timestamp.tv_usec = (xEvent->xmotion.time % 1000) * 1000;
     }

     if (motionY.axisabs != y) {
          motionY.type    = DIET_AXISMOTION;
          motionY.flags   = DIEF_AXISABS | DIEF_TIMESTAMP;
          motionY.axis    = DIAI_Y;
          motionY.axisabs = y;

          motionY.timestamp.tv_sec  =  xEvent->xmotion.time / 1000;
          motionY.timestamp.tv_usec = (xEvent->xmotion.time % 1000) * 1000;
     }
}

static void
motion_realize( X11InputData *data )
{
     if (motionX.type != DIET_UNKNOWN) {
          if (motionY.type != DIET_UNKNOWN)
               motionX.flags |= DIEF_FOLLOW;

          dfb_input_dispatch( data->device, &motionX );

          motionX.type = DIET_UNKNOWN;
     }

     if (motionY.type != DIET_UNKNOWN) {
          dfb_input_dispatch( data->device, &motionY );

          motionY.type = DIET_UNKNOWN;
     }
}

static DFBInputDeviceKeyIdentifier
xsymbol_to_id( KeySym xKeySymbol )
{
     switch (xKeySymbol) {
          case XK_a                : return DIKI_A;     
          case XK_b                : return DIKI_B;     
          case XK_c                : return DIKI_C;     
          case XK_d                : return DIKI_D;     
          case XK_e                : return DIKI_E;     
          case XK_f                : return DIKI_F;     
          case XK_g                : return DIKI_G;     
          case XK_h                : return DIKI_H;     
          case XK_i                : return DIKI_I;     
          case XK_j                : return DIKI_J;     
          case XK_k                : return DIKI_K;     
          case XK_l                : return DIKI_L;     
          case XK_m                : return DIKI_M;     
          case XK_n                : return DIKI_N;     
          case XK_o                : return DIKI_O;     
          case XK_p                : return DIKI_P;     
          case XK_q                : return DIKI_Q;     
          case XK_r                : return DIKI_R;     
          case XK_s                : return DIKI_S;     
          case XK_t                : return DIKI_T;     
          case XK_u                : return DIKI_U;     
          case XK_v                : return DIKI_V;     
          case XK_w                : return DIKI_W;     
          case XK_x                : return DIKI_X;     
          case XK_y                : return DIKI_Y;     
          case XK_z                : return DIKI_Z;     
          case XK_0                : return DIKI_0;     
          case XK_1                : return DIKI_1;     
          case XK_2                : return DIKI_2;     
          case XK_3                : return DIKI_3;     
          case XK_4                : return DIKI_4;     
          case XK_5                : return DIKI_5;     
          case XK_6                : return DIKI_6;     
          case XK_7                : return DIKI_7;     
          case XK_8                : return DIKI_8;     
          case XK_9                : return DIKI_9;     
          case XK_F1               : return DIKI_F1;    
          case XK_F2               : return DIKI_F2;    
          case XK_F3               : return DIKI_F3;    
          case XK_F4               : return DIKI_F4;    
          case XK_F5               : return DIKI_F5;    
          case XK_F6               : return DIKI_F6;    
          case XK_F7               : return DIKI_F7;    
          case XK_F8               : return DIKI_F8;    
          case XK_F9               : return DIKI_F9;    
          case XK_F10              : return DIKI_F10;   
          case XK_F11              : return DIKI_F11;   
          case XK_F12              : return DIKI_F12;   

          case XK_Shift_L          : return DIKI_SHIFT_L;     
          case XK_Shift_R          : return DIKI_SHIFT_R;     
          case XK_Control_L        : return DIKI_CONTROL_L;   
          case XK_Control_R        : return DIKI_CONTROL_R;   
          case XK_Alt_L            : return DIKI_ALT_L;       
          case XK_Alt_R            : return DIKI_ALT_R;       
          case XK_Meta_L           : return DIKI_META_L;      
          case XK_Meta_R           : return DIKI_META_R;      
          case XK_Super_L          : return DIKI_SUPER_L;     
          case XK_Super_R          : return DIKI_SUPER_R;     
          case XK_Hyper_L          : return DIKI_HYPER_L;     
          case XK_Hyper_R          : return DIKI_HYPER_R;     
          case XK_Mode_switch      : return DIKI_ALT_R;

          case XK_Caps_Lock        : return DIKI_CAPS_LOCK;   
          case XK_Num_Lock         : return DIKI_NUM_LOCK;    
          case XK_Scroll_Lock      : return DIKI_SCROLL_LOCK; 

          case XK_Escape           : return DIKI_ESCAPE;      
          case XK_Left             : return DIKI_LEFT;        
          case XK_Right            : return DIKI_RIGHT;       
          case XK_Up               : return DIKI_UP;          
          case XK_Down             : return DIKI_DOWN;        

          case XK_Tab              : return DIKI_TAB;
          case XK_ISO_Left_Tab     : return DIKI_TAB;
          case XK_Return           : return DIKI_ENTER;       
          case XK_space            : return DIKI_SPACE;       
          case XK_BackSpace        : return DIKI_BACKSPACE;   
          case XK_Insert           : return DIKI_INSERT;      
          case XK_Delete           : return DIKI_DELETE;      
          case XK_Home             : return DIKI_HOME;       
          case XK_End              : return DIKI_END;        
          case XK_Page_Up          : return DIKI_PAGE_UP;    
          case XK_Page_Down        : return DIKI_PAGE_DOWN;  
          case XK_Print            : return DIKI_PRINT;
          case XK_Pause            : return DIKI_PAUSE;       

          /*  The labels on these keys depend on the type of keyboard.
           *  We've choosen the names from a US keyboard layout. The
           *  comments refer to the ISO 9995 terminology.
           */
          case XK_quoteleft        : return DIKI_QUOTE_LEFT;   /*  TLDE  */
          case XK_minus            : return DIKI_MINUS_SIGN;   /*  AE11  */
          case XK_equal            : return DIKI_EQUALS_SIGN;  /*  AE12  */
          case XK_bracketleft      : return DIKI_BRACKET_LEFT; /*  AD11  */
          case XK_bracketright     : return DIKI_BRACKET_RIGHT;/*  AD12  */
          case XK_backslash        : return DIKI_BACKSLASH;    /*  BKSL  */
          case XK_semicolon        : return DIKI_SEMICOLON;    /*  AC10  */
          case XK_quoteright       : return DIKI_QUOTE_RIGHT;  /*  AC11  */
          case XK_comma            : return DIKI_COMMA;        /*  AB08  */
          case XK_period           : return DIKI_PERIOD;       /*  AB09  */
          case XK_slash            : return DIKI_SLASH;        /*  AB10  */
          case XK_less             : return DIKI_LESS_SIGN;    /*  103rd  */

          case XK_KP_Divide        : return DIKI_KP_DIV;      
          case XK_KP_Multiply      : return DIKI_KP_MULT;     
          case XK_KP_Subtract      : return DIKI_KP_MINUS;    
          case XK_KP_Add           : return DIKI_KP_PLUS;     
          case XK_KP_Enter         : return DIKI_KP_ENTER;    
          case XK_KP_Space         : return DIKI_KP_SPACE;    
          case XK_KP_Tab           : return DIKI_KP_TAB;      
          case XK_KP_F1            : return DIKI_KP_F1;       
          case XK_KP_F2            : return DIKI_KP_F2;       
          case XK_KP_F3            : return DIKI_KP_F3;       
          case XK_KP_F4            : return DIKI_KP_F4;       
          case XK_KP_Equal         : return DIKI_KP_EQUAL;    
          case XK_KP_Separator     : return DIKI_KP_SEPARATOR;
                                   
          case XK_KP_Delete        : return DIKI_KP_DECIMAL;
          case XK_KP_Insert        : return DIKI_KP_0;      
          case XK_KP_End           : return DIKI_KP_1;      
          case XK_KP_Down          : return DIKI_KP_2;      
          case XK_KP_Page_Down     : return DIKI_KP_3;      
          case XK_KP_Left          : return DIKI_KP_4;      
          case XK_KP_Begin         : return DIKI_KP_5;      
          case XK_KP_Right         : return DIKI_KP_6;      
          case XK_KP_Home          : return DIKI_KP_7;      
          case XK_KP_Up            : return DIKI_KP_8;      
          case XK_KP_Page_Up       : return DIKI_KP_9;

          case XK_KP_Decimal       : return DIKI_KP_DECIMAL;
          case XK_KP_0             : return DIKI_KP_0;      
          case XK_KP_1             : return DIKI_KP_1;      
          case XK_KP_2             : return DIKI_KP_2;      
          case XK_KP_3             : return DIKI_KP_3;      
          case XK_KP_4             : return DIKI_KP_4;      
          case XK_KP_5             : return DIKI_KP_5;      
          case XK_KP_6             : return DIKI_KP_6;      
          case XK_KP_7             : return DIKI_KP_7;      
          case XK_KP_8             : return DIKI_KP_8;      
          case XK_KP_9             : return DIKI_KP_9;

          case 0                   : break;

          default:
               D_DEBUG_AT( X11_Input, "Unknown key symbol 0x%lx\n", xKeySymbol);
     }    

     return DIKI_UNKNOWN;
}

static DFBInputDeviceKeySymbol
xsymbol_to_symbol( KeySym xKeySymbol )
{
     if (xKeySymbol >= 0x20 && xKeySymbol <= 0xff)
          return xKeySymbol;

     if (xKeySymbol >= XK_F1 && xKeySymbol <= XK_F35)
          return DFB_FUNCTION_KEY( xKeySymbol - XK_F1 + 1 );

     switch (xKeySymbol) {
          case XK_Shift_L          : return DIKS_SHIFT;
          case XK_Shift_R          : return DIKS_SHIFT;     
          case XK_Control_L        : return DIKS_CONTROL;   
          case XK_Control_R        : return DIKS_CONTROL;   
          case XK_Alt_L            : return DIKS_ALT;       
          case XK_Alt_R            : return DIKS_ALT;       
          case XK_Meta_L           : return DIKS_META;      
          case XK_Meta_R           : return DIKS_META;      
          case XK_Super_L          : return DIKS_SUPER;     
          case XK_Super_R          : return DIKS_SUPER;     
          case XK_Hyper_L          : return DIKS_HYPER;     
          case XK_Hyper_R          : return DIKS_HYPER;     
          case XK_Mode_switch      : return DIKS_ALTGR;

          case XK_Caps_Lock        : return DIKS_CAPS_LOCK;   
          case XK_Num_Lock         : return DIKS_NUM_LOCK;    
          case XK_Scroll_Lock      : return DIKS_SCROLL_LOCK; 

          case XK_Escape           : return DIKS_ESCAPE;      
          case XK_Left             : return DIKS_CURSOR_LEFT;
          case XK_Right            : return DIKS_CURSOR_RIGHT;       
          case XK_Up               : return DIKS_CURSOR_UP;          
          case XK_Down             : return DIKS_CURSOR_DOWN;        

          case XK_Tab              : return DIKS_TAB;
          case XK_ISO_Left_Tab     : return DIKS_TAB;
          case XK_Return           : return DIKS_ENTER;       
          case XK_space            : return DIKS_SPACE;       
          case XK_BackSpace        : return DIKS_BACKSPACE;   
          case XK_Insert           : return DIKS_INSERT;      
          case XK_Delete           : return DIKS_DELETE;      
          case XK_Home             : return DIKS_HOME;       
          case XK_End              : return DIKS_END;        
          case XK_Page_Up          : return DIKS_PAGE_UP;    
          case XK_Page_Down        : return DIKS_PAGE_DOWN;  
          case XK_Print            : return DIKS_PRINT;
          case XK_Pause            : return DIKS_PAUSE;       

          case XK_KP_Divide        : return DIKS_SLASH;
          case XK_KP_Multiply      : return DIKS_ASTERISK;     
          case XK_KP_Subtract      : return DIKS_MINUS_SIGN;    
          case XK_KP_Add           : return DIKS_PLUS_SIGN;     
          case XK_KP_Enter         : return DIKS_ENTER;    
          case XK_KP_Space         : return DIKS_SPACE;    
          case XK_KP_Tab           : return DIKS_TAB;      
          case XK_KP_F1            : return DIKS_F1;       
          case XK_KP_F2            : return DIKS_F2;       
          case XK_KP_F3            : return DIKS_F3;       
          case XK_KP_F4            : return DIKS_F4;       
          case XK_KP_Equal         : return DIKS_EQUALS_SIGN;    
          case XK_KP_Separator     : return DIKS_COLON; /* FIXME: what is a separator */
                                   
          case XK_KP_Delete        : return DIKS_DELETE;
          case XK_KP_Insert        : return DIKS_INSERT;      
          case XK_KP_End           : return DIKS_END;      
          case XK_KP_Down          : return DIKS_CURSOR_DOWN;      
          case XK_KP_Page_Down     : return DIKS_PAGE_DOWN;      
          case XK_KP_Left          : return DIKS_CURSOR_LEFT;      
          case XK_KP_Begin         : return DIKS_BEGIN;      
          case XK_KP_Right         : return DIKS_CURSOR_RIGHT;      
          case XK_KP_Home          : return DIKS_HOME;      
          case XK_KP_Up            : return DIKS_CURSOR_UP;      
          case XK_KP_Page_Up       : return DIKS_PAGE_UP;

          case XK_KP_Decimal       : return DIKS_PERIOD;
          case XK_KP_0             : return DIKS_0;      
          case XK_KP_1             : return DIKS_1;      
          case XK_KP_2             : return DIKS_2;      
          case XK_KP_3             : return DIKS_3;      
          case XK_KP_4             : return DIKS_4;      
          case XK_KP_5             : return DIKS_5;      
          case XK_KP_6             : return DIKS_6;      
          case XK_KP_7             : return DIKS_7;      
          case XK_KP_8             : return DIKS_8;      
          case XK_KP_9             : return DIKS_9;

          case 0                   : break;

          default:
               D_DEBUG("X11: Unknown key symbol 0x%lx\n", xKeySymbol);
     }    

     return DIKS_NULL;
}



static void handleMouseEvent(XEvent* pXEvent, X11InputData* pData)
{
     static int          iMouseEventCount = 0;
     DFBInputEvent  dfbEvent;
     if (pXEvent->type == MotionNotify) {
          motion_compress( pXEvent->xmotion.x, pXEvent->xmotion.y, pXEvent );
          ++iMouseEventCount;
     }

     if ( pXEvent->type == ButtonPress || pXEvent->type == ButtonRelease ) {
          if ( pXEvent->type == ButtonPress )
               dfbEvent.type = DIET_BUTTONPRESS;
          else
               dfbEvent.type = DIET_BUTTONRELEASE;

          dfbEvent.flags = DIEF_TIMESTAMP;

          /* Get pressed button */
          switch ( pXEvent->xbutton.button ) {
               case 1:
                    dfbEvent.button = DIBI_LEFT;
                    break;
               case 2:
                    dfbEvent.button = DIBI_MIDDLE;
                    break;
               case 3:
                    dfbEvent.button = DIBI_RIGHT;
                    break;
                    //Wheel events
               case 4: /*up*/
               case 5: /*down*/
               case 6: /*left*/
               case 7: /*right*/
                    if (pXEvent->type == ButtonPress) {
                         dfbEvent.type = DIET_AXISMOTION;
                         dfbEvent.flags = DIEF_AXISREL;
                         dfbEvent.axis = DIAI_Z;
                         /*SCROLL UP*/
                         if ( pXEvent->xbutton.button == 4 ) {
                              dfbEvent.axisrel = -1;
                         }
                         /*SCROLL DOWN */
                         else if (pXEvent->xbutton.button == 5) {
                              dfbEvent.axisrel = 1;
                         }
                         /*SCROLL LEFT*/
                         else if (pXEvent->xbutton.button == 6) {
                              dfbEvent.axis = DIAI_X;
                              dfbEvent.axisrel = -1;
                         }
                         /*SCROLL RIGHT*/
                         else if (pXEvent->xbutton.button == 7 ) {
                              dfbEvent.axis = DIAI_X;
                              dfbEvent.axisrel = 1;
                         }
                    }
                    else
                         return;
                    break;
               default:
                    break;
          }

          dfbEvent.timestamp.tv_sec  =  pXEvent->xbutton.time / 1000;
          dfbEvent.timestamp.tv_usec = (pXEvent->xbutton.time % 1000) * 1000;

          dfb_input_dispatch( pData->device, &dfbEvent );
          ++iMouseEventCount;
     }
}

static void
handle_expose( const XExposeEvent *expose )
{
     CoreLayer               *layer = 0;
     const DisplayLayerFuncs *funcs = 0;
     CoreLayerContext        *context;
     int                      i;

     /* find the correct layer */
     for( i=0; i<dfb_layer_num(); i++ ) {
          X11LayerData *lds;

          layer = dfb_layer_at( i );
          lds   = (X11LayerData*)(layer->layer_data);
          if( lds->xw && (lds->xw->window == expose->window) )
               break;
     }

     /* layer not found? */
     if( i==dfb_layer_num() )
          return;

     funcs = layer->funcs;

     D_ASSERT( funcs != NULL );
     D_ASSERT( funcs->UpdateRegion != NULL );

     /* Get the currently active context. */
     if (dfb_layer_get_active_context( layer, &context ) == DFB_OK) {
          CoreLayerRegion *region;

          /* Get the first region. */
          if (dfb_layer_context_get_primary_region( context,
                                                    false, &region ) == DFB_OK)
          {
               /* Lock the region to avoid tearing due to concurrent updates. */
               dfb_layer_region_lock( region );

               /* Get the surface of the region. */
               if (region->surface) {
                    dfb_surface_lock_buffer( region->surface, CSBR_FRONT, region->surface_accessor, CSAF_READ, &region->surface_lock );

                    if (region->surface_lock.buffer) {
                         DFBRegion update = { expose->x, expose->y,
                                              expose->x + expose->width  - 1,
                                              expose->y + expose->height - 1 };

                         funcs->UpdateRegion( layer, layer->driver_data, layer->layer_data,
                                              region->region_data, region->surface, &update, &region->surface_lock );
                    }

                    dfb_surface_unlock_buffer(region->surface, &region->surface_lock );
               }

               /* Unlock the region. */
               dfb_layer_region_unlock( region );

               /* Release the region. */
               dfb_layer_region_unref( region );
          }

          /* Release the context. */
          dfb_layer_context_unref( context );
     }
}

/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void*
x11EventThread( DirectThread *thread, void *driver_data )
{
     X11InputData *data   = driver_data;
     DFBX11       *x11    = data->x11;
     DFBX11Shared *shared = x11->shared;

     while (!data->stop) {
          unsigned int  pull = 23;
          XEvent        xEvent; 
          DFBInputEvent dfbEvent;
          static int    nextKeyIsRepeat = false;

          /* FIXME: Detect key repeats, we're receiving KeyPress, KeyRelease, KeyPress, KeyRelease... !!?? */

          if (shared->window_count == 0) {
               /* no window, so no event */
               usleep( 50000 );
               continue;
          }

          usleep( 10000 );

          XLockDisplay( x11->display );

          while (!data->stop && pull-- && XPending( x11->display )) {
               XNextEvent( x11->display, &xEvent );

               /* is this key repeat? idea from GII */
               if ( (xEvent.type == KeyRelease) && (XPending( x11->display )) ) {
                    XEvent peekEvent;
                    XPeekEvent( x11->display, &peekEvent );
                    if ( (peekEvent.type == KeyPress) &&
                         (peekEvent.xkey.keycode == xEvent.xkey.keycode) &&
                         (peekEvent.xkey.time - xEvent.xkey.time < 2) ) {
                              nextKeyIsRepeat = true;
                    }
               }

               XUnlockDisplay( x11->display );

               D_DEBUG_AT( X11_Input, "Event received: %d\n", xEvent.type );

               switch (xEvent.type) {
                    case ButtonPress:
                    case ButtonRelease:
                         motion_realize( data );
                    case MotionNotify:
                         handleMouseEvent( &xEvent, data ); // crash ???
                         break;

                    case KeyPress:
                    case KeyRelease: {
                         motion_realize( data );

                         dfbEvent.type     = (xEvent.type == KeyPress) ? DIET_KEYPRESS : DIET_KEYRELEASE;
                         dfbEvent.flags    = DIEF_KEYCODE | DIEF_TIMESTAMP;
                         dfbEvent.key_code = xEvent.xkey.keycode;

                         dfbEvent.timestamp.tv_sec  =  xEvent.xkey.time / 1000;
                         dfbEvent.timestamp.tv_usec = (xEvent.xkey.time % 1000) * 1000;

                         if ( (xEvent.type == KeyPress) && nextKeyIsRepeat ) {
                              nextKeyIsRepeat = false;
                              dfbEvent.flags |= DIEF_REPEAT;
                         }

                         dfb_input_dispatch( data->device, &dfbEvent );
                         break;
                    }

                    case Expose:
                         handle_expose( &xEvent.xexpose );
                         break;

                    case DestroyNotify:
                         /* this event is mainly to unblock XNextEvent. */
                         break;

                    default:
                         break;
               }

               XLockDisplay( x11->display );
          }

          XUnlockDisplay( x11->display );

          if (!data->stop)
               motion_realize( data );
     }

     return NULL;
}

/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int
driver_get_available( void )
{
     return dfb_system_type() == CORE_X11;
}

/*
 * Fill out general information about this driver.
 * Called once during initialization of DirectFB.
 */
static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     snprintf ( info->name,   DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "X11 Input Driver" );
     snprintf ( info->vendor, DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "directfb.org" );

     info->version.major = 0;
     info->version.minor = 1;
}

/*
 * Open the device, fill out information about it,
 * allocate and fill private data, start input thread.
 * Called during initialization, resuming or taking over mastership.
 */
static DFBResult
driver_open_device( CoreInputDevice  *device,
                    unsigned int      number,
                    InputDeviceInfo  *info,
                    void            **driver_data )
{
     X11InputData *data;
     DFBX11       *x11    = dfb_system_data();
     DFBX11Shared *shared = x11->shared;

     D_DEBUG_AT( X11_Input, "%s()\n", __FUNCTION__ );

     fusion_skirmish_prevail( &shared->lock );

     fusion_skirmish_dismiss( &shared->lock );

     /* set device vendor and name */
     snprintf( info->desc.vendor, DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "XServer" );
     snprintf( info->desc.name,   DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "X11 Input" );

     /* set one of the primary input device IDs */
     info->prefered_id = DIDID_KEYBOARD;

     /* set type flags */
     info->desc.type   = DIDTF_JOYSTICK | DIDTF_KEYBOARD | DIDTF_MOUSE;

     /* set capabilities */
     info->desc.caps   = DICAPS_ALL;

     /* enable translation of fake raw hardware keycodes */
     info->desc.min_keycode = 8;
     info->desc.max_keycode = 255;


     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(X11InputData) );

     data->device = device;
     data->x11    = x11;

     /* start input thread */
     data->thread = direct_thread_create( DTT_INPUT, x11EventThread, data, "X11 Input" );

     /* set private data pointer */
     *driver_data = data;

     return DFB_OK;
}

/*
 * Fetch one entry from the device's keymap if supported.
 * this does a fake mapping based on the orginal DFB code
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice           *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
     int           i;
     X11InputData *data = driver_data;
     DFBX11       *x11  = data->x11;

     XLockDisplay( x11->display );

     for (i=0; i<4; i++) {
          KeySym xSymbol = XKeycodeToKeysym( x11->display, entry->code, i );

          if (i == 0)
               entry->identifier = xsymbol_to_id( xSymbol );

          entry->symbols[i] = xsymbol_to_symbol( xSymbol );
     }

     XUnlockDisplay( x11->display );

     /* is CapsLock effective? */
     if (entry->identifier >= DIKI_A && entry->identifier <= DIKI_Z)
          entry->locks |= DILS_CAPS;

     /* is NumLock effective? */
     if (entry->identifier >= DIKI_KP_DECIMAL && entry->identifier <= DIKI_KP_9)
          entry->locks |= DILS_NUM;

     return DFB_OK;
}

/*
 * End thread, close device and free private data.
 */
static void
driver_close_device( void *driver_data )
{
     X11InputData *data   = driver_data;
     DFBX11       *x11    = data->x11;
     DFBX11Shared *shared = x11->shared;

     D_DEBUG_AT( X11_Input, "%s()\n", __FUNCTION__ );

     /* stop input thread */
     data->stop = true;

     if (!shared->x_error) {
          XLockDisplay( x11->display );
          XSync( x11->display, False );
          XUnlockDisplay( x11->display );
     }

     /* it is possible that this "close" function is called from the same
      * thread that the input device is actually running on.
      * This happens when you e.g. click the close box with your mouse.
      * As a fix, we check if we are this thread. */
     if (data->thread != direct_thread_self()) {
          direct_thread_join( data->thread );
          direct_thread_destroy( data->thread );
     }

     /* free private data */
     D_FREE ( data );
}

