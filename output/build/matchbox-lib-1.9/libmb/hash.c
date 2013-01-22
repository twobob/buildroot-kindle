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

#define _GNU_SOURCE

#include "hash.h"

// static struct nlist *hashtab[HASHSIZE];

struct hash*
hash_new(int size)
{
  struct hash *h = (struct hash *)(malloc(sizeof(struct hash)));
  h->size = size;
  h->hashtab = (struct nlist **)(malloc(sizeof(struct nlist)*size));
  memset((void *)h->hashtab, 0, sizeof(struct nlist)*size);
  return h;
}

unsigned int hashfunc(struct hash *h, char *s)
{
   unsigned int hashval;

   for(hashval = 0; *s != '\0'; s++)
      hashval = *s + 21 * hashval;
   return hashval % h->size;
}

struct nlist *hash_lookup(struct hash *h, char *s)
{
   struct nlist *np;

   for (np = h->hashtab[hashfunc(h, s)]; np != NULL; np = np->next)
      if (strcmp(s, np->key) == 0)
	 return np;
   return NULL;
}

struct nlist *hash_add(struct hash *h, char *key, char *val)
{
   struct nlist *np;
   unsigned int hashval;

   if ((np = hash_lookup(h, key)) == NULL)
   {
      np = (struct nlist * ) malloc(sizeof(*np));
      if ( np == NULL || (np->key = strdup(key)) == NULL)
	 return NULL;
      hashval = hashfunc(h, key);
      np->next = h->hashtab[hashval];
      h->hashtab[hashval] = np;
   } else {
      free((void *) np->value);
   }
   if ((np->value = (unsigned char*)strdup(val)) == NULL)
      return NULL;
   return np;
}

void hash_empty(struct hash *h)
{
   memset(h->hashtab, 0, sizeof(h->hashtab));
}

void 
hash_destroy(struct hash *h)
{
  struct nlist *np = NULL, *op = NULL;
  int    i;

  for (i=0;i<h->size;i++)
    if (h->hashtab[i])
      {
	np = h->hashtab[i];
	while (np != NULL) 
	  {
	    op = np->next;
	    if (np->key)   free(np->key);
	    if (np->value) free(np->value);
	    free(np);
	    np = op;
	  }
      }

  free(h->hashtab);
  free(h);
}
