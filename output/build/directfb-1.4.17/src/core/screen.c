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

#include <directfb.h>

#include <direct/debug.h>

#include <core/layers.h>
#include <core/screen.h>
#include <core/screens_internal.h>


DFBResult
dfb_screen_get_info( CoreScreen           *screen,
                     DFBScreenID          *ret_id,
                     DFBScreenDescription *ret_desc )
{
     CoreScreenShared *shared;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );

     shared = screen->shared;

     if (ret_id)
          *ret_id = shared->screen_id;

     if (ret_desc)
          *ret_desc = shared->description;

     return DFB_OK;
}

DFBResult
dfb_screen_suspend( CoreScreen *screen )
{
     D_ASSERT( screen != NULL );

     return DFB_OK;
}

DFBResult
dfb_screen_resume( CoreScreen *screen )
{
     D_ASSERT( screen != NULL );

     return DFB_OK;
}

DFBResult
dfb_screen_set_powermode( CoreScreen         *screen,
                          DFBScreenPowerMode  mode )
{
     ScreenFuncs *funcs;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->funcs != NULL );

     funcs = screen->funcs;

     if (funcs->SetPowerMode)
          return funcs->SetPowerMode( screen,
                                      screen->driver_data,
                                      screen->screen_data,
                                      mode );

     return DFB_UNSUPPORTED;
}

DFBResult
dfb_screen_wait_vsync( CoreScreen *screen )
{
     ScreenFuncs *funcs;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->funcs != NULL );

     funcs = screen->funcs;

     if (funcs->WaitVSync)
          return funcs->WaitVSync( screen,
                                   screen->driver_data, screen->screen_data );

     return DFB_UNSUPPORTED;
}

DFBResult
dfb_screen_get_vsync_count( CoreScreen *screen, unsigned long *ret_count )
{
     ScreenFuncs *funcs;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( ret_count != NULL );

     funcs = screen->funcs;

     if (funcs->GetVSyncCount)
          return funcs->GetVSyncCount( screen,
                                       screen->driver_data,
                                       screen->screen_data,
                                       ret_count );

     return DFB_UNSUPPORTED;
}


/*********************************** Mixers ***********************************/

DFBResult
dfb_screen_get_mixer_info( CoreScreen                *screen,
                           int                        mixer,
                           DFBScreenMixerDescription *ret_desc )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( mixer >= 0 );
     D_ASSERT( mixer < screen->shared->description.mixers );
     D_ASSERT( ret_desc != NULL );

     /* Return mixer description. */
     *ret_desc = screen->shared->mixers[mixer].description;

     return DFB_OK;
}

DFBResult
dfb_screen_get_mixer_config( CoreScreen           *screen,
                             int                   mixer,
                             DFBScreenMixerConfig *ret_config )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( mixer >= 0 );
     D_ASSERT( mixer < screen->shared->description.mixers );
     D_ASSERT( ret_config != NULL );

     /* Return current mixer configuration. */
     *ret_config = screen->shared->mixers[mixer].configuration;

     return DFB_OK;
}

DFBResult
dfb_screen_test_mixer_config( CoreScreen                 *screen,
                              int                         mixer,
                              const DFBScreenMixerConfig *config,
                              DFBScreenMixerConfigFlags  *ret_failed )
{
     DFBResult                 ret;
     DFBScreenMixerConfigFlags failed = DSMCONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestMixerConfig != NULL );
     D_ASSERT( mixer >= 0 );
     D_ASSERT( mixer < screen->shared->description.mixers );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->mixers[mixer].configuration.flags );

     /* Test the mixer configuration. */
     ret = screen->funcs->TestMixerConfig( screen,
                                           screen->driver_data,
                                           screen->screen_data,
                                           mixer, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK) );

     if (ret_failed)
          *ret_failed = failed;

     return ret;
}

DFBResult
dfb_screen_set_mixer_config( CoreScreen                 *screen,
                             int                         mixer,
                             const DFBScreenMixerConfig *config )
{
     DFBResult                 ret;
     DFBScreenMixerConfigFlags failed = DSMCONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestMixerConfig != NULL );
     D_ASSERT( screen->funcs->SetMixerConfig != NULL );
     D_ASSERT( mixer >= 0 );
     D_ASSERT( mixer < screen->shared->description.mixers );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->mixers[mixer].configuration.flags );

     /* Test configuration first. */
     ret = screen->funcs->TestMixerConfig( screen,
                                           screen->driver_data,
                                           screen->screen_data,
                                           mixer, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK && failed) );

     if (ret)
          return ret;

     /* Set configuration afterwards. */
     ret = screen->funcs->SetMixerConfig( screen,
                                          screen->driver_data,
                                          screen->screen_data,
                                          mixer, config );
     if (ret)
          return ret;

     /* Store current configuration. */
     screen->shared->mixers[mixer].configuration = *config;

     return DFB_OK;
}


/********************************** Encoders **********************************/

DFBResult
dfb_screen_get_encoder_info( CoreScreen                  *screen,
                             int                          encoder,
                             DFBScreenEncoderDescription *ret_desc )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( encoder >= 0 );
     D_ASSERT( encoder < screen->shared->description.encoders );
     D_ASSERT( ret_desc != NULL );

     /* Return encoder description. */
     *ret_desc = screen->shared->encoders[encoder].description;

     return DFB_OK;
}

DFBResult
dfb_screen_get_encoder_config( CoreScreen             *screen,
                               int                     encoder,
                               DFBScreenEncoderConfig *ret_config )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( encoder >= 0 );
     D_ASSERT( encoder < screen->shared->description.encoders );
     D_ASSERT( ret_config != NULL );

     /* Return current encoder configuration. */
     *ret_config = screen->shared->encoders[encoder].configuration;

     return DFB_OK;
}

DFBResult
dfb_screen_test_encoder_config( CoreScreen                   *screen,
                                int                           encoder,
                                const DFBScreenEncoderConfig *config,
                                DFBScreenEncoderConfigFlags  *ret_failed )
{
     DFBResult                   ret;
     DFBScreenEncoderConfigFlags failed = DSECONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestEncoderConfig != NULL );
     D_ASSERT( encoder >= 0 );
     D_ASSERT( encoder < screen->shared->description.encoders );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->encoders[encoder].configuration.flags );

     /* Test the encoder configuration. */
     ret = screen->funcs->TestEncoderConfig( screen,
                                             screen->driver_data,
                                             screen->screen_data,
                                             encoder, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK && failed) );

     if (ret_failed)
          *ret_failed = failed;

     return ret;
}

DFBResult
dfb_screen_set_encoder_config( CoreScreen                   *screen,
                               int                           encoder,
                               const DFBScreenEncoderConfig *config )
{
     DFBResult                   ret;
     DFBScreenEncoderConfigFlags failed = DSECONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestEncoderConfig != NULL );
     D_ASSERT( screen->funcs->SetEncoderConfig != NULL );
     D_ASSERT( encoder >= 0 );
     D_ASSERT( encoder < screen->shared->description.encoders );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->encoders[encoder].configuration.flags );

     /* Test configuration first. */
     ret = screen->funcs->TestEncoderConfig( screen,
                                             screen->driver_data,
                                             screen->screen_data,
                                             encoder, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK && failed) );

     if (ret)
          return ret;

     /* Set configuration afterwards. */
     ret = screen->funcs->SetEncoderConfig( screen,
                                            screen->driver_data,
                                            screen->screen_data,
                                            encoder, config );
     if (ret)
          return ret;

     /* Store current configuration. */
     screen->shared->encoders[encoder].configuration = *config;

     return DFB_OK;
}


/********************************** Outputs ***********************************/

DFBResult
dfb_screen_get_output_info( CoreScreen                 *screen,
                            int                         output,
                            DFBScreenOutputDescription *ret_desc )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( output >= 0 );
     D_ASSERT( output < screen->shared->description.outputs );
     D_ASSERT( ret_desc != NULL );

     /* Return output description. */
     *ret_desc = screen->shared->outputs[output].description;

     return DFB_OK;
}

DFBResult
dfb_screen_get_output_config( CoreScreen            *screen,
                              int                    output,
                              DFBScreenOutputConfig *ret_config )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( output >= 0 );
     D_ASSERT( output < screen->shared->description.outputs );
     D_ASSERT( ret_config != NULL );

     /* Return current output configuration. */
     *ret_config = screen->shared->outputs[output].configuration;

     return DFB_OK;
}

DFBResult
dfb_screen_test_output_config( CoreScreen                  *screen,
                               int                          output,
                               const DFBScreenOutputConfig *config,
                               DFBScreenOutputConfigFlags  *ret_failed )
{
     DFBResult                  ret;
     DFBScreenOutputConfigFlags failed = DSOCONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestOutputConfig != NULL );
     D_ASSERT( output >= 0 );
     D_ASSERT( output < screen->shared->description.outputs );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->outputs[output].configuration.flags );

     /* Test the output configuration. */
     ret = screen->funcs->TestOutputConfig( screen,
                                            screen->driver_data,
                                            screen->screen_data,
                                            output, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK && failed) );

     if (ret_failed)
          *ret_failed = failed;

     return ret;
}

DFBResult
dfb_screen_set_output_config( CoreScreen                  *screen,
                              int                          output,
                              const DFBScreenOutputConfig *config )
{
     DFBResult                  ret;
     DFBScreenOutputConfigFlags failed = DSOCONF_NONE;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->TestOutputConfig != NULL );
     D_ASSERT( screen->funcs->SetOutputConfig != NULL );
     D_ASSERT( output >= 0 );
     D_ASSERT( output < screen->shared->description.outputs );
     D_ASSERT( config != NULL );
     D_ASSERT( config->flags == screen->shared->outputs[output].configuration.flags );

     /* Test configuration first. */
     ret = screen->funcs->TestOutputConfig( screen,
                                            screen->driver_data,
                                            screen->screen_data,
                                            output, config, &failed );

     D_ASSUME( (ret == DFB_OK && !failed) || (ret != DFB_OK && failed) );

     if (ret)
          return ret;

     /* Set configuration afterwards. */
     ret = screen->funcs->SetOutputConfig( screen,
                                           screen->driver_data,
                                           screen->screen_data,
                                           output, config );
     if (ret)
          return ret;

     /* Store current configuration. */
     screen->shared->outputs[output].configuration = *config;

     return DFB_OK;
}

DFBResult
dfb_screen_get_screen_size( CoreScreen *screen,
                            int        *ret_width,
                            int        *ret_height )
{
     D_ASSERT( screen != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( screen->funcs->GetScreenSize != NULL );
     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );

     return screen->funcs->GetScreenSize( screen,
                                          screen->driver_data, screen->screen_data,
                                          ret_width, ret_height );
}

DFBResult
dfb_screen_get_layer_dimension( CoreScreen *screen,
                                CoreLayer  *layer,
                                int        *ret_width,
                                int        *ret_height )
{
     int               i;
     DFBResult         ret = DFB_UNSUPPORTED;
     CoreScreenShared *shared;
     ScreenFuncs      *funcs;

     D_ASSERT( screen != NULL );
     D_ASSERT( screen->shared != NULL );
     D_ASSERT( screen->funcs != NULL );
     D_ASSERT( layer != NULL );
     D_ASSERT( ret_width != NULL );
     D_ASSERT( ret_height != NULL );

     shared = screen->shared;
     funcs  = screen->funcs;

     if (funcs->GetMixerState) {
          for (i=0; i<shared->description.mixers; i++) {
               const DFBScreenMixerConfig *config = &shared->mixers[i].configuration;

               if ((config->flags & DSMCONF_LAYERS) &&
                   DFB_DISPLAYLAYER_IDS_HAVE( config->layers, dfb_layer_id(layer) ))
               {
                    CoreMixerState state;

                    ret = funcs->GetMixerState( screen, screen->driver_data, screen->screen_data, i, &state );
                    if (ret == DFB_OK) {
                         if (state.flags & CMSF_DIMENSION) {
                              *ret_width  = state.dimension.w;
                              *ret_height = state.dimension.h;

                              return DFB_OK;
                         }

                         ret = DFB_UNSUPPORTED;
                    }
               }
          }

          for (i=0; i<shared->description.mixers; i++) {
               const DFBScreenMixerDescription *desc = &shared->mixers[i].description;

               if ((desc->caps & DSMCAPS_SUB_LAYERS) &&
                   DFB_DISPLAYLAYER_IDS_HAVE( desc->sub_layers, dfb_layer_id(layer) ))
               {
                    CoreMixerState state;

                    ret = funcs->GetMixerState( screen, screen->driver_data, screen->screen_data, i, &state );
                    if (ret == DFB_OK) {
                         if (state.flags & CMSF_DIMENSION) {
                              *ret_width  = state.dimension.w;
                              *ret_height = state.dimension.h;

                              return DFB_OK;
                         }

                         ret = DFB_UNSUPPORTED;
                    }
               }
          }
     }

     if (funcs->GetScreenSize)
          ret = funcs->GetScreenSize( screen,
                                      screen->driver_data, screen->screen_data,
                                      ret_width, ret_height );

     return ret;
}

