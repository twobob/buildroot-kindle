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

#ifndef __VOODOO__MANAGER_H__
#define __VOODOO__MANAGER_H__

#include <voodoo/types.h>
#include <voodoo/message.h>


DirectResult voodoo_manager_super          ( VoodooManager           *manager,
                                             const char              *name,
                                             VoodooInstanceID        *ret_instance );

DirectResult voodoo_manager_request        ( VoodooManager           *manager,
                                             VoodooInstanceID         instance,
                                             VoodooMethodID           method,
                                             VoodooRequestFlags       flags,
                                             VoodooResponseMessage  **ret_response,
                                             VoodooMessageBlockType   block_type, ... );

DirectResult voodoo_manager_next_response  ( VoodooManager           *manager,
                                             VoodooResponseMessage   *response,
                                             VoodooResponseMessage  **ret_response );

DirectResult voodoo_manager_finish_request ( VoodooManager           *manager,
                                             VoodooResponseMessage   *response );

DirectResult voodoo_manager_respond        ( VoodooManager           *manager,
                                             bool                     flush,
                                             VoodooMessageSerial      request,
                                             DirectResult             result,
                                             VoodooInstanceID         instance,
                                             VoodooMessageBlockType   block_type, ... );

DirectResult voodoo_manager_register_local ( VoodooManager           *manager,
                                             bool                     super,
                                             void                    *dispatcher,
                                             void                    *real,
                                             VoodooDispatch           dispatch,
                                             VoodooInstanceID        *ret_instance_id );

DirectResult voodoo_manager_unregister_local( VoodooManager           *manager,
                                              VoodooInstanceID         instance_id );

DirectResult voodoo_manager_lookup_local   ( VoodooManager           *manager,
                                             VoodooInstanceID         instance,
                                             void                   **ret_dispatcher,
                                             void                   **ret_real );

DirectResult voodoo_manager_register_remote( VoodooManager           *manager,
                                             bool                     super,
                                             void                    *requestor,
                                             VoodooInstanceID         instance );

DirectResult voodoo_manager_lookup_remote  ( VoodooManager           *manager,
                                             VoodooInstanceID         instance,
                                             void                   **ret_requestor );

DirectResult voodoo_manager_quit           ( VoodooManager           *manager );

DirectResult voodoo_manager_check_allocation( VoodooManager           *manager,
                                              unsigned int             amount );


#endif
