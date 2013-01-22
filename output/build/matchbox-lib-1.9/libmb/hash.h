/* libmb
 * Copyright (C) 2002 Matthew Allum
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE 101

struct hash {
  struct nlist **hashtab;
  int size;
};

struct nlist {
   struct nlist *next;
   char *key;
   unsigned char *value;
};

struct hash* hash_new(int size);
unsigned int hashfunc(struct hash *h, char *s);
struct nlist *hash_lookup(struct hash *h, char *s);
struct nlist *hash_add(struct hash *h, char *key, char *val);
void hash_empty();
void hash_destroy(struct hash *h);
