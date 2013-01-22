/*
   (c) Copyright 2009  Denis Oliver Kropp

   All rights reserved.

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

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <direct/messages.h>

#include <directfb.h>
#include <directfb_util.h>


static int
show_usage( const char *prg )
{
     fprintf( stderr, "Usage: %s <url>\n", prg );

     return -1;
}

int
main( int argc, char *argv[] )
{
     int                     i;
     DFBResult               ret;
     DFBSurfaceDescription   desc;
     IDirectFB              *dfb;
     IDirectFBImageProvider *provider = NULL;
     IDirectFBSurface       *source   = NULL;
     IDirectFBSurface       *dest     = NULL;
     const char             *url      = NULL;

     /* Parse arguments. */
     for (i=1; i<argc; i++) {
          if (!strcmp( argv[i], "-h" ))
               return show_usage( argv[0] );
          else if (!url)
               url = argv[i];
          else
               return show_usage( argv[0] );
     }

     /* Check if we got an URL. */
     if (!url)
          return show_usage( argv[0] );
          
     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: DirectFBInit() failed!\n" );
          return ret;
     }

     /* Create super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: DirectFBCreate() failed!\n" );
          return ret;
     }

     /* Create an image provider for the image to be loaded. */
     ret = dfb->CreateImageProvider( dfb, url, &provider );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: IDirectFB::CreateImageProvider( '%s' ) failed!\n", url );
          goto out;
     }

     /* Get the surface description. */
     ret = provider->GetSurfaceDescription( provider, &desc );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: IDirectFBImageProvider::GetSurfaceDescription() failed!\n" );
          goto out;
     }
     
     D_INFO( "DFBTest/Mirror: Source is %dx%d using %s\n",
             desc.width, desc.height, dfb_pixelformat_name(desc.pixelformat) );

     /* Create a surface for the image. */
     ret = dfb->CreateSurface( dfb, &desc, &source );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: IDirectFB::CreateSurface() failed!\n" );
          goto out;
     }
     
     ret = provider->RenderTo( provider, source, NULL );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: IDirectFBImageProvider::RenderTo() failed!\n" );
          goto out;
     }
     
     desc.width  = desc.width  * 4 / 3;
     desc.height = desc.height * 4 / 3;

     D_INFO( "DFBTest/Mirror: Destination is %dx%d using %s\n",
             desc.width, desc.height, dfb_pixelformat_name(desc.pixelformat) );

     /* Create a surface for the image. */
     ret = dfb->CreateSurface( dfb, &desc, &dest );
     if (ret) {
          D_DERROR( ret, "DFBTest/Mirror: IDirectFB::CreateSurface() failed!\n" );
          goto out;
     }

     dest->Clear( dest, 0, 0, 0, 0 );

     dest->SetColor( dest, 0xff, 0, 0, 0xff );
     dest->FillRectangle( dest, 0, desc.height / 3, desc.width, desc.height / 3 );

     dest->SetColor( dest, 0xff, 0xff, 0xff, 0xff );
     dest->FillRectangle( dest, 0, desc.height / 3 * 2, desc.width, desc.height / 3 );

     dest->SetDstColorKey( dest, 0xff, 0, 0 );

     dest->SetBlittingFlags( dest, DSBLIT_FLIP_VERTICAL | DSBLIT_DST_COLORKEY );
     //dest->Blit( dest, source, NULL, 0, 0 );
     dest->StretchBlit( dest, source, NULL, NULL );

     dest->Dump( dest, ".", "dfbtest_mirror" );


out:
     if (dest)
          dest->Release( dest );

     if (source)
          source->Release( source );

     if (provider)
          provider->Release( provider );

     /* Shutdown DirectFB. */
     dfb->Release( dfb );

     return ret;
}

