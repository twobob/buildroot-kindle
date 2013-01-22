/* Abstract ordered set data type.
   Copyright (C) 2006-2007, 2009-2011 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "gl_oset.h"

#if !HAVE_INLINE

/* Define all functions of this file as inline accesses to the
   struct gl_list_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

gl_oset_t
gl_oset_nx_create_empty (gl_oset_implementation_t implementation,
                         gl_setelement_compar_fn compar_fn,
                         gl_setelement_dispose_fn dispose_fn)
{
  return implementation->nx_create_empty (implementation, compar_fn,
                                          dispose_fn);
}

size_t
gl_oset_size (gl_oset_t set)
{
  return ((const struct gl_oset_impl_base *) set)->vtable->size (set);
}

bool
gl_oset_search (gl_oset_t set, const void *elt)
{
  return ((const struct gl_oset_impl_base *) set)->vtable->search (set, elt);
}

bool
gl_oset_search_atleast (gl_oset_t set,
                        gl_setelement_threshold_fn threshold_fn,
                        const void *threshold, const void **eltp)
{
  return ((const struct gl_oset_impl_base *) set)->vtable
         ->search_atleast (set, threshold_fn, threshold, eltp);
}

int
gl_oset_nx_add (gl_oset_t set, const void *elt)
{
  return ((const struct gl_oset_impl_base *) set)->vtable->nx_add (set, elt);
}

bool
gl_oset_remove (gl_oset_t set, const void *elt)
{
  return ((const struct gl_oset_impl_base *) set)->vtable
         ->remove_elt (set, elt);
}

void
gl_oset_free (gl_oset_t set)
{
  ((const struct gl_oset_impl_base *) set)->vtable->oset_free (set);
}

gl_oset_iterator_t
gl_oset_iterator (gl_oset_t set)
{
  return ((const struct gl_oset_impl_base *) set)->vtable->iterator (set);
}

bool
gl_oset_iterator_next (gl_oset_iterator_t *iterator, const void **eltp)
{
  return iterator->vtable->iterator_next (iterator, eltp);
}

void
gl_oset_iterator_free (gl_oset_iterator_t *iterator)
{
  iterator->vtable->iterator_free (iterator);
}

#endif
