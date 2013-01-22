/*
 * tuple.c
 * management of key->value tuples
 *
 * Copyright (c) 2011, 2012 pkgconf authors (see AUTHORS).
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This software is provided 'as is' and without any warranty, express or
 * implied.  In no event shall the authors be liable for any damages arising
 * from the use of this software.
 */

#include "pkg.h"
#include "bsdstubs.h"

static pkg_tuple_t *pkg_global_var;

void
pkg_tuple_add_global(const char *key, const char *value)
{
	pkg_global_var = pkg_tuple_add(pkg_global_var, key, value);
}

char *
pkg_tuple_find_global(const char *key)
{
	pkg_tuple_t *node;

	PKG_FOREACH_LIST_ENTRY(pkg_global_var, node)
	{
		if (!strcasecmp(node->key, key))
			return node->value;
	}

	return NULL;
}

void
pkg_tuple_free_global(void)
{
	pkg_tuple_free(pkg_global_var);
}

void
pkg_tuple_define_global(const char *kv)
{
	char *workbuf = strdup(kv);
	char *value;

	value = strchr(workbuf, '=');
	if (value == NULL)
		goto out;

	*value++ = '\0';
	pkg_tuple_add_global(workbuf, value);
out:
	free(workbuf);
}

pkg_tuple_t *
pkg_tuple_add(pkg_tuple_t *parent, const char *key, const char *value)
{
	pkg_tuple_t *tuple = calloc(sizeof(pkg_tuple_t), 1);

	tuple->key = strdup(key);
	tuple->value = pkg_tuple_parse(parent, value);

	tuple->next = parent;
	if (tuple->next != NULL)
		tuple->next->prev = tuple;

	return tuple;
}

char *
pkg_tuple_find(pkg_tuple_t *head, const char *key)
{
	pkg_tuple_t *node;
	char *res;

	if ((res = pkg_tuple_find_global(key)) != NULL)
		return res;

	PKG_FOREACH_LIST_ENTRY(head, node)
	{
		if (!strcasecmp(node->key, key))
			return node->value;
	}

	return NULL;
}

char *
pkg_tuple_parse(pkg_tuple_t *vars, const char *value)
{
	char buf[PKG_BUFSIZE];
	const char *ptr;
	char *bptr = buf;

	for (ptr = value; *ptr != '\0' && bptr - buf < PKG_BUFSIZE; ptr++)
	{
		if (*ptr != '$')
			*bptr++ = *ptr;
		else if (*(ptr + 1) == '{')
		{
			static char varname[PKG_BUFSIZE];
			char *vptr = varname;
			const char *pptr;
			char *kv, *parsekv;

			*vptr = '\0';

			for (pptr = ptr + 2; *pptr != '\0'; pptr++)
			{
				if (*pptr != '}')
					*vptr++ = *pptr;
				else
				{
					*vptr = '\0';
					break;
				}
			}

			ptr += (pptr - ptr);
			kv = pkg_tuple_find(vars, varname);

			if (kv != NULL)
			{
				parsekv = pkg_tuple_parse(vars, kv);

				strncpy(bptr, parsekv, PKG_BUFSIZE - (bptr - buf));
				bptr += strlen(parsekv);

				free(parsekv);
			}
		}
	}

	*bptr = '\0';

	return strdup(buf);
}

void
pkg_tuple_free(pkg_tuple_t *head)
{
	pkg_tuple_t *node, *next;

	PKG_FOREACH_LIST_ENTRY_SAFE(head, next, node)
	{
		free(node->key);
		free(node->value);
		free(node);
	}
}
