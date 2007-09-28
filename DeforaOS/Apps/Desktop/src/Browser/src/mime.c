/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include "mime.h"
#include "../config.h"


/* Mime */
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
		perror("Error while loading MIME globs");
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
		if((p = realloc(mime->types, sizeof(*(mime->types))
						*(mime->types_cnt+1))) == NULL)
			break;
		mime->types = p;
		p[mime->types_cnt].type = strdup(buf);
		p[mime->types_cnt].glob = strdup(glob);
		p[mime->types_cnt].icon_24 = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
		p[mime->types_cnt].icon_48 = NULL;
		p[mime->types_cnt].icon_96 = NULL;
#endif
/*		p[mime->types_cnt].open = mime->config != NULL
			? config_get(mime->config, buf, "open") : NULL;
		p[mime->types_cnt].edit = mime->config != NULL
			? config_get(mime->config, buf, "edit") : NULL; */
		mime->types_cnt++;
		if(p[mime->types_cnt-1].type == NULL
				|| p[mime->types_cnt-1].glob == NULL)
			break;
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
	char * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return;
	if((mime->config = config_new()) == NULL)
		return;
	if((filename = malloc(strlen(homedir) + 1 + strlen(MIME_CONFIG_FILE)
					+ 1)) == NULL)
		return;
	sprintf(filename, "%s/%s", homedir, MIME_CONFIG_FILE);
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
		return NULL;
	if((program = config_get(mime->config, type, action)) != NULL)
		return program;
	if((p = strchr(type, '/')) == NULL || *(++p) == '\0'
			|| (p = strdup(type)) == NULL)
		return NULL;
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
	unsigned int i;

	for(i = 0; i < mime->types_cnt; i++)
		if(fnmatch(mime->types[i].glob, path, FNM_NOESCAPE) == 0)
			break;
#ifdef FNM_CASEFOLD
	if(i < mime->types_cnt)
		return mime->types[i].type;
	for(i = 0; i < mime->types_cnt; i++)
		if(fnmatch(mime->types[i].glob, path,
					FNM_NOESCAPE | FNM_CASEFOLD) == 0)
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
	/* FIXME report errors */
{
	char const * program;
	pid_t pid;

	if((program = mime_get_handler(mime, type, action)) == NULL)
		return 2;
	if((pid = fork()) == -1)
	{
		perror("fork");
		return 3;
	}
	if(pid != 0)
		return 0;
	execlp(program, program, path, NULL);
	fprintf(stderr, "%s%s%s%s\n", "browser: ", program, ": ",
			strerror(errno));
	exit(2);
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
			continue;
		}
#if GTK_CHECK_VERSION(2, 6, 0)
		if(size == 48)
		{
			if(mime->types[i].icon_48 == NULL)
				mime->types[i].icon_48 = _icons_size(theme,
						type, 48);
			*icon = mime->types[i].icon_48;
			continue;
		}
		if(size == 96)
		{
			if(mime->types[i].icon_96 == NULL)
				mime->types[i].icon_96 = _icons_size(theme,
						type, 96);
			*icon = mime->types[i].icon_96;
			continue;
		}
#endif
		*icon = _icons_size(theme, type, size);
	}
	va_end(arg);
}

static GdkPixbuf * _icons_size(GtkIconTheme * theme, char const * type,
		int size)
{
	static char buf[256] = "gnome-mime-";
	char * p;
	GdkPixbuf * icon;

	strncpy(&buf[11], type, sizeof(buf)-11);
	for(buf[sizeof(buf)-1] = '\0'; (p = strchr(&buf[11], '/')) != NULL;
			*p = '-');
	if((icon = gtk_icon_theme_load_icon(theme, buf, size, 0, NULL)) != NULL)
		return icon;
	if((p = strchr(&buf[11], '-')) != NULL)
	{
		*p = '\0';
		return gtk_icon_theme_load_icon(theme, buf, size, 0, NULL);
	}
	return icon;
}
