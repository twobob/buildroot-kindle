#ifndef _MB_HAVE_MB_CONFIG
#define _MB_HAVE_MB_CONFIG

/* 
 *
 *
 *
 */


#if 1
#define MB_HAVE_PNG
#endif

#if 0
#define MB_HAVE_JPEG
#endif

#if 1
#define MB_HAVE_XFT
#endif

#if 0
#define MB_HAVE_PANGO
#endif


#define mb_supports_png() (1)

#define mb_supports_jpeg() (0)

#define mb_supports_xft() (1)

#define mb_supports_pango() (0)

#ifdef MB_HAVE_PNG
#ifndef USE_PNG  		/* config.h redefines  */
#define USE_PNG 
#endif
#endif

#ifdef MB_HAVE_JPEG
#ifndef USE_JPG 
#define USE_JPG 
#endif
#endif

#ifdef MB_HAVE_XFT
#ifndef USE_XFT  
#define USE_XFT
#endif 
#endif

#ifdef MB_HAVE_PANGO
#ifndef USE_PANGO  
#define USE_PANGO 
#endif
#endif

#endif

