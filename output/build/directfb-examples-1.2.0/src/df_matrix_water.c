/*
   (c) Copyright 2001-2008  The DirectFB Organization (directfb.org)
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

#include <directfb.h>

/* Include extended rendering interface (Water) */
#include <directfb_render.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <math.h>

//#define USE_FLOAT

/**********************************************************************************************************************/

static IDirectFB            *dfb     = NULL;
static IDirectFBSurface     *primary = NULL;
static IDirectFBEventBuffer *events  = NULL;
static IWater               *water   = NULL;

/**********************************************************************************************************************/

static void init_application( int *argc, char **argv[] );
static void exit_application( int status );

/**********************************************************************************************************************/

static inline void
matrix_rotate_16_16( IWater *water,
                     int     radians )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FIXED_16_16;
     transform.type        = WTT_ROTATE_FREE;
     transform.matrix[0].i = radians;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}

static inline void
matrix_translate_16_16( IWater *water,
                        int     translate_x,
                        int     translate_y )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FIXED_16_16;
     transform.type        = WTT_TRANSLATE_X | WTT_TRANSLATE_Y;
     transform.matrix[0].i = translate_x;
     transform.matrix[1].i = translate_y;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}

static inline void
matrix_scale_16_16( IWater *water,
                    int     scale_x,
                    int     scale_y )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FIXED_16_16;
     transform.type        = WTT_SCALE_X | WTT_SCALE_Y;
     transform.matrix[0].i = scale_x;
     transform.matrix[1].i = scale_y;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}


static inline void
matrix_rotate_float( IWater *water,
                     float   radians )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FLOAT;
     transform.type        = WTT_ROTATE_FREE;
     transform.matrix[0].f = radians;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}

static inline void
matrix_translate_float( IWater *water,
                        float   translate_x,
                        float   translate_y )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FLOAT;
     transform.type        = WTT_TRANSLATE_X | WTT_TRANSLATE_Y;
     transform.matrix[0].f = translate_x;
     transform.matrix[1].f = translate_y;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}

static inline void
matrix_scale_float( IWater *water,
                    float   scale_x,
                    float   scale_y )
{
     WaterTransform transform;

     transform.flags       = WTF_SCALAR | WTF_TYPE;
     transform.scalar      = WST_FLOAT;
     transform.type        = WTT_SCALE_X | WTT_SCALE_Y;
     transform.matrix[0].f = scale_x;
     transform.matrix[1].f = scale_y;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
}

/**********************************************************************************************************************/

/* Attributes for four rectangles and two lines */
static const WaterAttribute m_attributes[] = {{
     .type          = WAT_FILL_COLOR,
     .value.color   = { 0xff, 0xff, 0xff, 0xff }  /* [0] white - rect fill */
},{
     .type          = WAT_FILL_COLOR,
     .value.color   = { 0xff, 0x00, 0xff, 0x00 }  /* [1] green - rect fill */
},{
     .type          = WAT_FILL_COLOR,
     .value.color   = { 0xff, 0x00, 0x00, 0xff }  /* [2] blue - rect fill */
},{
     .type          = WAT_FILL_COLOR,
     .value.color   = { 0xff, 0xff, 0x00, 0x00 }  /* [3] red - rect fill/draw (including next attribute) */
},{
     .type          = WAT_DRAW_COLOR,
     .value.color   = { 0xff, 0xcc, 0xcc, 0xcc }  /* [4] gray - " */
},{
     .type          = WAT_DRAW_COLOR,
     .value.color   = { 0xff, 0x12, 0x34, 0x56 }  /* [5] bluish - line draw */
},{
     .type          = WAT_DRAW_COLOR,
     .value.color   = { 0xff, 0xff, 0xff, 0xff }  /* [6] white - line draw */
},{
     .type          = WAT_FILL_COLOR,
     .value.color   = { 0xff, 0x80, 0x90, 0x70 }  /* [7] greenish - triangle fill */
}};

/* Values for four rectangles */
static const WaterValue m_rect_values[] = {
     { .scalar.i =   -20 }, { .scalar.i =  -20 }, /* [ 0] white rectangle */
     { .scalar.i =    40 }, { .scalar.i =   40 },

     { .scalar.i =  -120 }, { .scalar.i =  -20 }, /* [ 4] green rectangle */
     { .scalar.i =    40 }, { .scalar.i =   40 },

     { .scalar.i =   -20 }, { .scalar.i = -120 }, /* [ 8] blue rectangle */
     { .scalar.i =    40 }, { .scalar.i =   40 },

     { .scalar.i =   100 }, { .scalar.i =  100 }, /* [12] red/gray rectangle (fill/stroke) */
     { .scalar.i =   100 }, { .scalar.i =  100 },
};

/* Values for two lines */
static const WaterValue m_line_values[] = {
     { .scalar.i =     0 }, { .scalar.i =    0 }, /* [0] bluish line */
     { .scalar.i =   300 }, { .scalar.i =  300 },

     { .scalar.i =   -20 }, { .scalar.i =  -20 }, /* [4] white line */
     { .scalar.i =  -300 }, { .scalar.i = -300 },
};

/* Values for a triangle */
static const WaterValue m_tri_values[] = {
     { .scalar.i =     0 }, { .scalar.i =    0 }, /* [0] greenish triangle */
     { .scalar.i =   200 }, { .scalar.i = -210 },
     { .scalar.i =  -200 }, { .scalar.i =  190 },
};

/* Elements for four rectangles and two lines */
static const WaterElement m_elements[] = {{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_RECTANGLE ), /* [0] */
     .flags         = WEF_FILL,
     .values        = &m_rect_values[0],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_RECTANGLE ), /* [1] */
     .flags         = WEF_FILL,
     .values        = &m_rect_values[4],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_RECTANGLE ), /* [2] */
     .flags         = WEF_FILL,
     .values        = &m_rect_values[8],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_RECTANGLE ), /* [3] */
     .flags         = WEF_FILL | WEF_DRAW,
     .values        = &m_rect_values[12],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_LINE ),      /* [4] */
     .flags         = WEF_DRAW,
     .values        = &m_line_values[0],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_LINE ),      /* [5] */
     .flags         = WEF_DRAW,
     .values        = &m_line_values[4],
     .num_values    = 4
},{
     .index         = WATER_ELEMENT_TYPE_INDEX( WET_TRIANGLE ),  /* [6] */
     .flags         = WEF_FILL,
     .values        = &m_tri_values[0],
     .num_values    = 6
}};

/* Joint Stream */
static const WaterJoint joint_stream[] = {{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [0] White */
     .buffer        = &m_attributes[0],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - rect fill [0] */
     .buffer        = &m_elements[0],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [1] Green */
     .buffer        = &m_attributes[1],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - rect fill [1] */
     .buffer        = &m_elements[1],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [2] Blue */
     .buffer        = &m_attributes[2],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - rect fill [2] */
     .buffer        = &m_elements[2],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [3] Red / Gray */
     .buffer        = &m_attributes[3],
     .count         = 2  /* also sets draw color */
},{
     .type          = WJT_ELEMENT_STREAM,    /* - rect draw+fill [3] */
     .buffer        = &m_elements[3],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [5] Bluish */
     .buffer        = &m_attributes[5],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - line draw [4] */
     .buffer        = &m_elements[4],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [6] White */
     .buffer        = &m_attributes[6],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - line draw [5] */
     .buffer        = &m_elements[5],
     .count         = 1
},{
     .type          = WJT_ATTRIBUTE_STREAM,  /* [7] Greenish */
     .buffer        = &m_attributes[7],
     .count         = 1
},{
     .type          = WJT_ELEMENT_STREAM,    /* - triangle fill [6] */
     .buffer        = &m_elements[6],
     .count         = 1
}};

/* Shape */
static const WaterShape m_shape = {
     .flags         = WSF_NONE,

     .root          = &joint_stream[0],
     .count         = D_ARRAY_SIZE(joint_stream)
};

int
main( int argc, char *argv[] )
{
     int            i = 0;
     int            width, height;
     WaterTransform transform;

     /* Initialize application. */
     init_application( &argc, &argv );

     /* Query size of output surface. */
     primary->GetSize( primary, &width, &height );

     /* Transform coordinates to have 0,0 in the center. */
     transform.flags       = WTF_TYPE | WTF_REPLACE;
     transform.type        = WTT_TRANSLATE_X | WTT_TRANSLATE_Y;
     transform.matrix[0].i = width  / 2;
     transform.matrix[1].i = height / 2;

     water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );

     /* Main loop. */
     while (1) {
          DFBInputEvent event;

          /* Clear the frame with black. */
          primary->Clear( primary, 0x00, 0x00, 0x00, 0x00 );

          /*
           * Render the whole scene via a Shape.
           *
           * The original version of the demo had 24 calls to IDirectFBSurface for this.
           */
          water->RenderShapes( water, &m_shape, 1 );

          /* Flip the output surface. */
          primary->Flip( primary, NULL, DSFLIP_WAITFORSYNC );

          /* Rotate and scale scene slightly. */
#ifdef USE_FLOAT
          matrix_rotate_float( water, 0.1 );
          matrix_scale_float( water, 0.99, 0.99 );
#else
          matrix_rotate_16_16( water, 0.1 * 0x10000 );
          matrix_scale_16_16( water, 0.99 * 0x10000, 0.99 * 0x10000 );
#endif

          /* Reset to initial transform after 500 frames. */
          if (++i == 500) {
               i = 0;

               water->SetAttribute( water, WAT_RENDER_TRANSFORM, WAF_NONE, (WaterValue){ .pointer = &transform } );
          }

          /* Check for new events. */
          while (events->GetEvent( events, DFB_EVENT(&event) ) == DFB_OK) {

               /* Handle key press events. */
               if (event.type == DIET_KEYPRESS) {
                    switch (event.key_symbol) {
                         case DIKS_ESCAPE:
                         case DIKS_POWER:
                         case DIKS_BACK:
                         case DIKS_SMALL_Q:
                         case DIKS_CAPITAL_Q:
                              exit_application( 0 );
                              break;

                         default:
                              break;
                    }
               }
          }
     }

     /* Shouldn't reach this. */
     return 0;
}

/**********************************************************************************************************************/

static void
init_application( int *argc, char **argv[] )
{
     DFBResult             ret;
     DFBSurfaceDescription desc;

     /* Initialize DirectFB including command line parsing. */
     ret = DirectFBInit( argc, argv );
     if (ret) {
          DirectFBError( "DirectFBInit() failed", ret );
          exit_application( 1 );
     }

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate() failed", ret );
          exit_application( 2 );
     }

     /* Request fullscreen mode. */
     dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );

     /* Fill the surface description. */
     desc.flags       = DSDESC_CAPS | DSDESC_PIXELFORMAT;
     desc.caps        = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
     desc.pixelformat = DSPF_ARGB;
     
     /* Create a primary surface. */
     ret = dfb->CreateSurface( dfb, &desc, &primary );
     if (ret) {
          DirectFBError( "IDirectFB::CreateSurface() failed", ret );
          exit_application( 3 );
     }
     
     /* Create an event buffer with key capable devices attached. */
     ret = dfb->CreateInputEventBuffer( dfb, DICAPS_KEYS, DFB_FALSE, &events );
     if (ret) {
          DirectFBError( "IDirectFB::CreateEventBuffer() failed", ret );
          exit_application( 4 );
     }
     
     /* Clear. */
     primary->Clear( primary, 0x00, 0x00, 0x00, 0x00 );
     primary->Flip( primary, NULL, 0 );

     /* Get the extended rendering interface. */
     ret = primary->GetWater( primary, &water );
     if (ret) {
          DirectFBError( "IDirectFBSurface::GetWater() failed", ret );
          exit_application( 5 );
     }
}

static void
exit_application( int status )
{
     /* Release the extended rendering interface. */
     if (water)
          water->Release( water );

     /* Release the event buffer. */
     if (events)
          events->Release( events );

     /* Release the primary surface. */
     if (primary)
          primary->Release( primary );

     /* Release the super interface. */
     if (dfb)
          dfb->Release( dfb );

     /* Terminate application. */
     exit( status );
}

