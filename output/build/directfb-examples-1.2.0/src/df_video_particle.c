/*
   (c) Copyright 2000-2002  convergence integrated media GmbH.
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <neo@directfb.org>.
              
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

#define SELFRUNNING

#include <directfb.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define PI 3.1415926536f

IDirectFB *dfb;
IDirectFBSurface *primary;

IDirectFBSurface *video;
IDirectFBVideoProvider *videoprovider;

IDirectFBSurface *convergence;

IDirectFBInputDevice *keyboard;

typedef struct _Particle
{
     float w;
     int sw, sh;
     int size;
     int launch;
     struct _Particle *next;
     struct _Particle *prev;
} Particle;

Particle *particles = NULL;

static float f = 0, fc = 0;

int sx, sy;

void spawn_particle()
{
     Particle *new_particle = (Particle*)malloc( sizeof(Particle) );

     new_particle->w = 0.05f;
     new_particle->sw = rand()%(int)(sx/3.2f) + (int)(sx/3.2f)*sin(f)
                                              + (int)(sx/3.2f);// + 40*sin(f*5);
     new_particle->sh = rand()%100 + sy-130;// + 40*cos(f*5);
     new_particle->size = rand()%(sx/20) +2;
     new_particle->launch = rand()%(sx/70);
     new_particle->next = NULL;

     if (!particles) {
          particles = new_particle;
     }
     else {
          new_particle->next = particles;
          particles->prev = new_particle;
          particles = new_particle;
     }
}

void draw_particles()
{
     Particle *p = particles;

     primary->SetBlittingFlags( primary, DSBLIT_BLEND_COLORALPHA |
                                         DSBLIT_COLORIZE );
     
     while (p) {
          DFBRectangle rect = { p->launch + sin(p->w/2)*(p->sw),
                                sy - sin(p->w)*p->sh, p->w*p->size+1,
                                p->w*p->size+1 };
          primary->SetColor( primary, 0xA0+rand()%0x50, 0xA0+rand()%0x50, 0xFF, 0x30 );
/*          primary->FillRectangle( primary, p->launch + sin(p->w/2)*(p->sw),
                                  sy - sin(p->w)*p->sh, p->w*p->size+1,
                                  p->w*p->size+1 );*/

          primary->SetColor( primary, sin(p->w+fc)*127+128, sin(p->w+PI/3+fc)*127+128, sin(p->w+PI+fc)*127+128, 0x40 );
          primary->StretchBlit( primary, video, NULL, &rect );

          p->w += PI/1000 * sqrt(p->w) * sx/640.0f;

          if (p->w > PI) {
               Particle *tp = p;

               if (p->prev)
                    p->prev->next = p->next;
               else
                    particles = p->next;

               if (p->next)
                    p->next->prev = p->prev;

               p = p->next;

               free(tp);
          }
          else {
               p = p->next;
          }
     }

     fc -= 0.5f;
}

void destroy_particles()
{
     Particle *p = particles;

     while (p) {
          particles = p->next;
          free(p);
          p = particles;
     }
}

int main( int argc, char *argv[] )
{
     int i;
     DFBInputDeviceKeyState quit = DIKS_UP;
     DFBInputDeviceKeyState spawn = 0;
     DFBInputDeviceKeyState right = 0;
     DFBInputDeviceKeyState left = 0;
     DFBResult err;

     srand( (long)time( 0 ) );

     if (DirectFBInit( &argc, &argv )  !=  DFB_OK)
          return 1;

     if (argc < 2)
     {
          fprintf(stderr, "%s: you must specify a video source\n", argv[0]);
          return 1;
     }

     if (DirectFBCreate( &dfb )  !=  DFB_OK)
          return 1;

     err = dfb->GetInputDevice( dfb, DIDID_KEYBOARD, &keyboard );
     if (err != DFB_OK) {
          DirectFBError( "CreateInputDevice for keyboard failed", err );
          dfb->Release( dfb );
          return 1;
     }

     err = dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
     if (err != DFB_OK) {
          DirectFBError( "Failed requesting exclusive access", err );
     }

#if 0
     err = dfb->SetVideoMode( dfb, sx, sy, 16 );
     if (err != DFB_OK) {
          DirectFBError( "Failed setting video mode", err );
          keyboard->Release( keyboard );
          dfb->Release( dfb );
          return 1;
     }
#endif

     {
          DFBSurfaceDescription dsc;

          dsc.flags = DSDESC_CAPS;
          dsc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;

          err = dfb->CreateSurface( dfb, &dsc, &primary );
          if (err != DFB_OK) {
               DirectFBError( "Failed creating primary surface", err );
               keyboard->Release( keyboard );
               dfb->Release( dfb );
               return 1;
          }

          primary->GetSize( primary, &sx, &sy );
     }

     {
          DFBSurfaceDescription dsc;
          IDirectFBImageProvider *provider;

          dfb->CreateImageProvider( dfb, DATADIR"/convergence.png",
                                             &provider );
          provider->GetSurfaceDescription( provider, &dsc );

          dfb->CreateSurface( dfb, &dsc, &convergence );

          provider->RenderTo( provider, convergence, NULL );
          provider->Release( provider );
     }
     
     {
          DFBSurfaceDescription desc;

          err = dfb->CreateVideoProvider( dfb, argv[1],
                                          &videoprovider );
          if (err != DFB_OK) {
               DirectFBError( "Failed creating video provider", err );
               primary->Release( primary );
               keyboard->Release( keyboard );
               dfb->Release( dfb );
               return 1;
          }

          videoprovider->GetSurfaceDescription( videoprovider, &desc );

          err = dfb->CreateSurface( dfb, &desc, &video );
          if (err != DFB_OK) {
               DirectFBError( "Failed creating video surface", err );
               videoprovider->Release( videoprovider );
               primary->Release( primary );
               keyboard->Release( keyboard );
               dfb->Release( dfb );
               return 1;
          }


          err = videoprovider->PlayTo( videoprovider, video, NULL, NULL, NULL );
          if (err != DFB_OK) {
               DirectFBError( "Failed creating video surface", err );
               videoprovider->Release( videoprovider );
               video->Release( video );
               primary->Release( primary );
               keyboard->Release( keyboard );
               dfb->Release( dfb );
               return 1;
          }
     }

     primary->Clear( primary, 0xFF, 0xFF, 0xFF, 0xFF );
     err = primary->Flip( primary, NULL, 0 );
     if (err != DFB_OK) {
          DirectFBError( "Failed flipping the primary surface", err );
          videoprovider->Release( videoprovider );
          video->Release( video );
          primary->Release( primary );
          keyboard->Release( keyboard );
          dfb->Release( dfb );
          return 1;
     }

     sleep(2);

     for (i=254; i>=0; i-=4) {
          primary->Clear( primary, i, i, i, 0xFF );
          err = primary->Flip( primary, NULL, DSFLIP_WAITFORSYNC );
          if (err != DFB_OK) {
               DirectFBError( "Failed flipping the primary surface", err );
               videoprovider->Release( videoprovider );
               video->Release( video );
               primary->Release( primary );
               keyboard->Release( keyboard );
               dfb->Release( dfb );
               return 1;
          }
     }

     primary->SetDrawingFlags( primary, DSDRAW_BLEND );

     while (!quit) {
          primary->SetBlittingFlags( primary, DSBLIT_BLEND_COLORALPHA |
                                              DSBLIT_COLORIZE );

//          primary->SetColor( primary, 0xFF, 0xFF, 0xFF, 0x30 );
          primary->SetColor( primary, sin(f*2)*100+155, sin(PI/3+f*2)*100+155, sin(PI+f*2)*100+155, 0x30 );
//          primary->SetColor( primary, 0xFF, 0xFF, 0xFF, 0xA );
          //primary->StretchBlit( primary, video, NULL, NULL );
          {
               DFBRectangle dest = { sx/2-75, sy/2-75, 150, 150 };

               primary->StretchBlit( primary, video, NULL, &dest );
          }
          //primary->FillRectangle( primary, 0, 0, sx, sy );

          {
               DFBRectangle dest = { -10, -10, sx+20, sy+20 };

               primary->SetColor( primary, 0xFF, 0xFF, 0xFF, 0x80 );
               primary->StretchBlit( primary, primary, NULL, &dest );
          }

          {
#ifdef SELFRUNNING
               if (!(rand()%50))
                    left = !left;

#else
               keyboard->GetKeyState( keyboard, DIKC_LEFT, &left );
#endif
               if (left)
                    f -= 0.02f;

               if (f < -PI/2)
                    f = -PI/2;
          }

          {
#ifdef SELFRUNNING
               if (!(rand()%50))
                    right = !right;
#else
               keyboard->GetKeyState( keyboard, DIKC_RIGHT, &right );
#endif
               if (right)
                    f += 0.02f;

               if (f > PI/2)
                    f = PI/2;
          }
#ifdef SELFRUNNING
          spawn = 100;
#else
          keyboard->GetKeyState( keyboard, DIKC_SPACE, &spawn );
#endif
          if (spawn) {
               spawn = rand()%4 == 0;
               while (spawn--)
                    spawn_particle();
          }

          if (rand()%100 == 0) {
               primary->SetBlittingFlags( primary, DSBLIT_BLEND_ALPHACHANNEL |
                                                   DSBLIT_BLEND_COLORALPHA |
                                                   DSBLIT_COLORIZE );
               primary->SetColor( primary, sin(f*2)*100+155, sin(PI/3+f*2)*100+155, sin(PI+f*2)*100+155, 0x30 );
               primary->Blit( primary, convergence, NULL, rand()%(sx/2) + sx/4, rand()%(sy/2) + sy/4 );
          }

          draw_particles();

          err = primary->Flip( primary, NULL, DSFLIP_BLIT /*| DSFLIP_WAITFORSYNC*/ );
          if (err != DFB_OK) {
               DirectFBError( "Failed flipping the primary surface", err );
               break;
          }

//          usleep(20000);

          keyboard->GetKeyState( keyboard, DIKI_ESCAPE, &quit );
     }

     destroy_particles();

     primary->SetColor( primary, 0, 0, 0, 10 );

     for (i=0; i<70; i++) {
          primary->FillRectangle( primary, 0, 0, sx, sy );

          err = primary->Flip( primary, NULL, DSFLIP_BLIT | DSFLIP_WAITFORSYNC );
          if (err != DFB_OK) {
               DirectFBError( "Failed flipping the primary surface", err );
               break;
          }
     }

     videoprovider->Release( videoprovider );
     video->Release( video );
     primary->Release( primary );
     keyboard->Release( keyboard );
     dfb->Release( dfb );

     return 42;
}

