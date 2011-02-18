/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <string.h>
#include <ctype.h>
#include <glib.h>


/* ghtml_make_url */
static gchar * _ghtml_make_url(gchar const * base, gchar const * url)
{
	char * b;
	char * p;

	if(url == NULL)
		return NULL;
	for(; isspace(*url); url++);
	/* XXX use a more generic protocol finder (strchr(':')) */
	if(strncmp("about:", url, 6) == 0)
		return g_strdup(url);
	if(strncmp("ftp://", url, 6) == 0)
		return g_strdup(url);
	if(strncmp("http://", url, 7) == 0)
		return g_strdup(url);
	if(strncmp("https://", url, 8) == 0)
		return g_strdup(url);
	if(strncmp("mailto:", url, 7) == 0)
		return g_strdup(url);
	if(base != NULL)
	{
		if(url[0] == '/')
		{
			if(strncmp("http://", base, 7) == 0)
			{
				if((b = g_strdup(base)) == NULL)
					return NULL;
				if((p = strchr(&b[7], '/')) != NULL)
				{
					*p = '\0';
					p = g_strdup_printf("%s%s", b, url);
					g_free(b);
					return p;
				}
				g_free(b);
			}
			/* FIXME implement other protocols */
			return g_strdup_printf("%s%s", base, url);
		}
		/* construct from basename */
		if((b = g_strdup(base)) == NULL)
			return NULL;
		p = b;
		/* FIXME implement other protocols */
		if(strncmp("http://", p, 7) == 0)
			p += 7;
		if((p = strrchr(p, '/')) != NULL)
			*p = '\0';
		p = g_strdup_printf("%s/%s", b, url);
		g_free(b);
		return p;
	}
	/* base is NULL, url is not NULL */
	if(url[0] == '/')
		return g_strdup(url);
	/* guess protocol */
	if(strncmp("ftp", url, 3) == 0)
		return g_strdup_printf("%s%s", "ftp://", url);
	/* FIXME guess http only for "www.*"? we're already in GNet...? */
	return g_strdup_printf("%s%s", "http://", url);
}
