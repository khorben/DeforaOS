/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <errno.h>
#include <libintl.h>
#include <System.h>
#include "mime.h"
#include "../config.h"
#define _(string) gettext(string)


/* Mime */
/* private */
/* types */
typedef struct _MimeType
{
	char * type;
	char * glob;
	GdkPixbuf * icon_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48;
	GdkPixbuf * icon_96;
#endif
} MimeType;

struct _Mime
{
	MimeType * types;
	size_t types_cnt;
	Config * config;
};


/* constants */
#define MIME_CONFIG_FILE ".mime"


/* public */
/* functions */
/* mime_new */
static void _new_config(Mime * mime);

Mime * mime_new(void)
{
	Mime * mime;
	char * globs[] = {
		PREFIX "/share/mime/globs",
	       	"/usr/share/mime/globs",
	       	"/usr/local/share/mime/globs",
	       	"/usr/pkg/share/mime/globs",
		NULL };
	char ** g = globs;
	FILE * fp = NULL;
	char buf[256];
	size_t len;
	char * glob;
	MimeType * p;

	if((mime = malloc(sizeof(*mime))) == NULL)
		return NULL;
	for(g = globs; *g != NULL; g++)
		if((fp = fopen(*g, "r")) != NULL)
			break;
	if(fp == NULL)
	{
		perror(_("Could not load MIME globs"));
		free(mime);
		return NULL;
	}
	mime->types = NULL;
	mime->types_cnt = 0;
	_new_config(mime);
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		errno = EINVAL;
		len = strlen(buf);
		if(buf[--len] != '\n')
			break;
		if(buf[0] == '#')
			continue;
		buf[len] = '\0';
		glob = strchr(buf, ':');
		*(glob++) = '\0';
		if((p = realloc(mime->types, sizeof(*p) * (mime->types_cnt
							+ 1))) == NULL)
			break;
		mime->types = p;
		p = &p[mime->types_cnt];
		p->type = strdup(buf);
		p->glob = strdup(glob);
		if(p->type == NULL || p->glob == NULL)
		{
			free(p->type);
			free(p->glob);
			break;
		}
		p->icon_24 = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
		p->icon_48 = NULL;
		p->icon_96 = NULL;
#endif
#if 0
		p->open = mime->config != NULL
			? config_get(mime->config, buf, "open") : NULL;
		p->edit = mime->config != NULL
			? config_get(mime->config, buf, "edit") : NULL;
#endif
		mime->types_cnt++;
	}
	if(!feof(fp))
	{
		perror(*g);
		mime_delete(mime);
		mime = NULL;
	}
	fclose(fp);
	return mime;
}

static void _new_config(Mime * mime)
{
	size_t len;
	char * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return;
	if((mime->config = config_new()) == NULL)
		return;
	len = strlen(homedir) + 1 + strlen(MIME_CONFIG_FILE) + 1;
	if((filename = malloc(len)) == NULL)
		return;
	snprintf(filename, len, "%s/%s", homedir, MIME_CONFIG_FILE);
	config_load(mime->config, filename);
	free(filename);
}


/* mime_delete */
void mime_delete(Mime * mime)
{
	unsigned int i;

	for(i = 0; i < mime->types_cnt; i++)
	{
		free(mime->types[i].type);
		free(mime->types[i].glob);
		free(mime->types[i].icon_24);
#if GTK_CHECK_VERSION(2, 6, 0)
		free(mime->types[i].icon_48);
		free(mime->types[i].icon_96);
#endif
	}
	free(mime->types);
	if(mime->config != NULL)
		config_delete(mime->config);
	free(mime);
}


/* accessors */
/* mime_get_handler */
char const * mime_get_handler(Mime * mime, char const * type,
		char const * action)
{
	char const * program;
	char * p;
	char * q;

	if(type == NULL || action == NULL)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if((program = config_get(mime->config, type, action)) != NULL)
		return program;
	if((p = strchr(type, '/')) == NULL || *(++p) == '\0'
			|| (p = strdup(type)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno)); /* XXX may be wrong */
		return NULL;
	}
	q = strchr(p, '/');
	q[1] = '*';
	q[2] = '\0';
	program = config_get(mime->config, p, action);
	free(p);
	return program;
}


/* useful */
/* mime_type */
char const * mime_type(Mime * mime, char const * path)
{
	char const * p;
	unsigned int i;

	p = strrchr(path, '/');
	p = (p != NULL) ? p + 1 : path;
	for(i = 0; i < mime->types_cnt; i++)
		if(fnmatch(mime->types[i].glob, p, FNM_NOESCAPE) == 0)
			break;
#ifdef FNM_CASEFOLD
	if(i < mime->types_cnt)
		return mime->types[i].type;
	for(i = 0; i < mime->types_cnt; i++)
		if(fnmatch(mime->types[i].glob, p, FNM_NOESCAPE | FNM_CASEFOLD)
				== 0)
			break;
#endif
	return i < mime->types_cnt ? mime->types[i].type : NULL;
}


/* mime_action */
int mime_action(Mime * mime, char const * action, char const * path)
{
	char const * type;

	if((type = mime_type(mime, path)) == NULL)
		return 1;
	return mime_action_type(mime, action, path, type);
}


/* mime_action_type */
int mime_action_type(Mime * mime, char const * action, char const * path,
		char const * type)
{
	int ret = 0;
	char const * program;
	char * argv[3];
	GError * error = NULL;

	if((program = mime_get_handler(mime, type, action)) == NULL)
		return -1;
	argv[0] = strdup(program);
	argv[1] = strdup(path);
	argv[2] = NULL;
	if(argv[0] == NULL || argv[1] == NULL)
		ret = -error_set_code(2, "%s", strerror(errno));
	else if(g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
				NULL, &error) == FALSE)
		ret = -error_set_code(2, "%s: %s", argv[0], error->message);
	free(argv[0]);
	free(argv[1]);
	return ret;
}


/* mime_icons */
static GdkPixbuf * _icons_size(GtkIconTheme * theme, char const * type,
		int size);

void mime_icons(Mime * mime, GtkIconTheme * theme, char const * type, ...)
{
	unsigned int i;
	va_list arg;
	int size;
	GdkPixbuf ** icon;

	for(i = 0; i < mime->types_cnt; i++)
		if(strcmp(type, mime->types[i].type) == 0)
			break;
	if(i == mime->types_cnt)
		return;
	va_start(arg, type);
	while((size = va_arg(arg, int)) > 0)
	{
		icon = va_arg(arg, GdkPixbuf **);
		if(size == 24)
		{
			if(mime->types[i].icon_24 == NULL)
				mime->types[i].icon_24 = _icons_size(theme,
						type, 24);
			*icon = mime->types[i].icon_24;
		}
#if GTK_CHECK_VERSION(2, 6, 0)
		else if(size == 48)
		{
			if(mime->types[i].icon_48 == NULL)
				mime->types[i].icon_48 = _icons_size(theme,
						type, 48);
			*icon = mime->types[i].icon_48;
		}
		else if(size == 96)
		{
			if(mime->types[i].icon_96 == NULL)
				mime->types[i].icon_96 = _icons_size(theme,
						type, 96);
			*icon = mime->types[i].icon_96;
		}
#endif
		else
			*icon = _icons_size(theme, type, size);
	}
	va_end(arg);
}

static GdkPixbuf * _icons_size(GtkIconTheme * theme, char const * type,
		int size)
{
	static char buf[256] = "gnome-mime-";
	char * p;
	GtkIconLookupFlags flags = GTK_ICON_LOOKUP_USE_BUILTIN
		| GTK_ICON_LOOKUP_GENERIC_FALLBACK;

	strncpy(&buf[11], type, sizeof(buf) - 11);
	buf[sizeof(buf) - 1] = '\0';
	for(; (p = strchr(&buf[11], '/')) != NULL; *p = '-');
	return gtk_icon_theme_load_icon(theme, buf, size, flags, NULL);
}
