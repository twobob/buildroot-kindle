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

#ifndef __VOODOO__MESSAGE_H__
#define __VOODOO__MESSAGE_H__

#include <voodoo/types.h>

#include <direct/debug.h>
#include <direct/memcpy.h>


#define VOODOO_MSG_ALIGN(i)     (((i) + 3) & ~3)

typedef enum {
     VMBT_NONE,
     VMBT_ID,
     VMBT_INT,
     VMBT_UINT,
     VMBT_DATA,
     VMBT_ODATA,
     VMBT_STRING
} VoodooMessageBlockType;

typedef enum {
     VREQ_NONE    = 0x00000000,
     VREQ_RESPOND = 0x00000001,
     VREQ_ASYNC   = 0x00000002,
     VREQ_QUEUE   = 0x00000004
} VoodooRequestFlags;

typedef enum {
     VMSG_SUPER,
     VMSG_REQUEST,
     VMSG_RESPONSE
} VoodooMessageType;


struct __V_VoodooMessageHeader {
     int                 size;
     VoodooMessageSerial serial;
     VoodooMessageType   type;
};


struct __V_VoodooSuperMessage {
     VoodooMessageHeader header;
};

struct __V_VoodooRequestMessage {
     VoodooMessageHeader header;

     VoodooInstanceID    instance;
     VoodooMethodID      method;

     VoodooRequestFlags  flags;
};

struct __V_VoodooResponseMessage {
     VoodooMessageHeader header;

     VoodooMessageSerial request;
     DirectResult        result;

     VoodooInstanceID    instance;
};


typedef struct {
     int         magic;

     const char *msg;
     const char *ptr;
} VoodooMessageParser;



#define __VOODOO_PARSER_PROLOG( parser, req_type )          \
     const char             *_vp_ptr;                       \
     VoodooMessageBlockType  _vp_type;                      \
     int                     _vp_length;                    \
     VoodooMessageParser    *_parser = &parser;             \
                                                            \
     D_MAGIC_ASSERT( _parser, VoodooMessageParser );        \
                                                            \
     _vp_ptr = _parser->ptr;                                \
                                                            \
     /* Read message block type. */                         \
     _vp_type = *(const VoodooMessageBlockType*) _vp_ptr;   \
                                                            \
     D_ASSERT( _vp_type == (req_type) );                    \
                                                            \
     /* Read data block length. */                          \
     _vp_length = *(const s32*) (_vp_ptr + 4)


#define __VOODOO_PARSER_EPILOG( parser )                    \
     /* Advance message data pointer. */                    \
     _parser->ptr += 8 + VOODOO_MSG_ALIGN(_vp_length)


#define VOODOO_PARSER_BEGIN( parser, message )                                                 \
     do {                                                                                      \
          const VoodooMessageHeader *_vp_header = (const VoodooMessageHeader *) (message);     \
          VoodooMessageParser       *_parser = &parser;                                        \
                                                                                               \
          D_ASSERT( (message) != NULL );                                                       \
          D_ASSERT( _vp_header->type == VMSG_REQUEST || _vp_header->type == VMSG_RESPONSE );   \
                                                                                               \
          _parser->msg = (const char*)(message);                                               \
          _parser->ptr = _parser->msg + (_vp_header->type == VMSG_REQUEST ?                    \
                              sizeof(VoodooRequestMessage) : sizeof(VoodooResponseMessage));   \
                                                                                               \
          D_MAGIC_SET_ONLY( _parser, VoodooMessageParser );                                    \
     } while (0)


#define VOODOO_PARSER_GET_ID( parser, ret_id )                        \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_ID );                  \
                                                                      \
          D_ASSERT( _vp_length == 4 );                                \
                                                                      \
          /* Read the ID. */                                          \
          (ret_id) = *(const u32*) (_vp_ptr + 8);                     \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_GET_INT( parser, ret_int )                      \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_INT );                 \
                                                                      \
          D_ASSERT( _vp_length == 4 );                                \
                                                                      \
          /* Read the integer. */                                     \
          (ret_int) = *(const s32*) (_vp_ptr + 8);                    \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_GET_UINT( parser, ret_uint )                    \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_UINT );                \
                                                                      \
          D_ASSERT( _vp_length == 4 );                                \
                                                                      \
          /* Read the unsigned integer. */                            \
          (ret_uint) = *(const u32*) (_vp_ptr + 8);                   \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_GET_DATA( parser, ret_data )                    \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_DATA );                \
                                                                      \
          D_ASSERT( _vp_length > 0 );                                 \
                                                                      \
          /* Return pointer to data. */                               \
          (ret_data) = (__typeof__(ret_data))(_vp_ptr + 8);           \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_READ_DATA( parser, dst, max_len )               \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_DATA );                \
                                                                      \
          D_ASSERT( _vp_length > 0 );                                 \
          D_ASSERT( _vp_length <= max_len );                          \
                                                                      \
          /* Copy data block. */                                      \
          direct_memcpy( (dst), _vp_ptr + 8, _vp_length );            \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_COPY_DATA( parser, ret_data )                   \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_DATA );                \
                                                                      \
          D_ASSERT( _vp_length > 0 );                                 \
                                                                      \
          /* Allocate memory on the stack. */                         \
          (ret_data) = alloca( _vp_length );                          \
                                                                      \
          /* Copy data block. */                                      \
          direct_memcpy( (ret_data), _vp_ptr + 8, _vp_length );       \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_GET_ODATA( parser, ret_data )                   \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_ODATA );               \
                                                                      \
          D_ASSERT( _vp_length >= 0 );                                \
                                                                      \
          /* Return pointer to data or NULL. */                       \
          if (_vp_length)                                             \
               (ret_data) = (__typeof__(ret_data))(_vp_ptr + 8);      \
          else                                                        \
               (ret_data) = NULL;                                     \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)

#define VOODOO_PARSER_GET_STRING( parser, ret_string )                \
     do {                                                             \
          __VOODOO_PARSER_PROLOG( parser, VMBT_STRING );              \
                                                                      \
          D_ASSERT( _vp_length > 0 );                                 \
                                                                      \
          /* Return pointer to string. */                             \
          (ret_string) = (const char*) (_vp_ptr + 8);                 \
                                                                      \
          __VOODOO_PARSER_EPILOG( parser );                           \
     } while (0)


#define VOODOO_PARSER_END( parser )                                   \
     do {                                                             \
          VoodooMessageParser *_parser = &parser;                     \
                                                                      \
          D_MAGIC_ASSERT( _parser, VoodooMessageParser );             \
                                                                      \
          D_ASSUME( *(const u32*) (_parser->ptr) == VMBT_NONE );      \
                                                                      \
          D_MAGIC_CLEAR( _parser );                                   \
     } while (0)


#endif
