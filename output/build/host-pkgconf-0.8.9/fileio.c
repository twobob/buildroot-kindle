/*
 * fileio.c
 * File reading utilities
 *
 * Copyright (c) 2012 pkgconf authors (see AUTHORS).
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

char *
pkg_fgetline(char *line, size_t size, FILE *stream)
{
	char *s = line;
	char *end = line + size - 1;
	bool quoted = false;
	int c = '\0', c2;

	if (s == NULL)
		return NULL;

	while (s < end && (c = getc(stream)) != EOF)
	{
		if (c == '\\')
		{
			quoted = true;
			continue;
		}

		else if (c == '\n')
		{
			*s++ = c;

			if (quoted)
			{
				quoted = false;
				continue;
			}

			break;
		}
		else if (c == '\r')
		{
			*s++ = '\n';

			if ((c2 = getc(stream)) == '\n')
			{
				if (quoted)
				{
					quoted = false;
					continue;
				}

				break;
			}

			ungetc(c2, stream);

			if (quoted)
			{
				quoted = false;
				continue;
			}

			break;
		}
		else
		{
			if (quoted) {
				*s++ = '\\';
				quoted = false;
			}
			*s++ = c;
		}

	}

	*s = '\0';

	if (c == EOF && (s == line || ferror(stream)))
		return NULL;

	return line;
}
