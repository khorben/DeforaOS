/* mime.c */



#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include "browser.h"
#include "mime.h"


/* Mime */
Mime * mime_new(void)
{
	Mime * mime;
	char * globs[] = { /* ideally taken from Gtk+ but seems impossible */
	       	"/usr/pkg/share/mime/globs",
	       	"/usr/local/share/mime/globs",
	       	"/usr/share/mime/globs",
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
						* (mime->types_cnt+1))) == NULL)
			break;
		mime->types = p;
		p[mime->types_cnt].type = strdup(buf);
		p[mime->types_cnt].glob = strdup(glob);
		p[mime->types_cnt++].icon = NULL;
		if(p[mime->types_cnt-1].type == NULL
				|| p[mime->types_cnt-1].glob == NULL)
			break;
	}
	if(!feof(fp))
	{
		perror("/usr/share/mime/globs");
		mime_delete(mime);
		mime = NULL;
	}
	fclose(fp);
	return mime;
}


void mime_delete(Mime * mime)
{
	unsigned int i;

	for(i = 0; i < mime->types_cnt; i++)
	{
		free(mime->types[i].type);
		free(mime->types[i].glob);
		free(mime->types[i].icon);
	}
	free(mime->types);
	free(mime);
}


/* useful */
char const * mime_type(Mime * mime, char const * path)
{
	unsigned int i;

	for(i = 0; i < mime->types_cnt; i++)
		if(fnmatch(mime->types[i].glob, path, FNM_NOESCAPE) == 0)
			break;
	return i < mime->types_cnt ? mime->types[i].type : NULL;
}


GdkPixbuf * mime_icon(Mime * mime, GtkIconTheme * theme, char const * type)
{
	unsigned int i;
	static char buf[256] = "gnome-mime-";
	char * p;

	for(i = 0; i < mime->types_cnt; i++)
		if(strcmp(type, mime->types[i].type) == 0)
			break;
	if(i == mime->types_cnt)
		return NULL;
	if(mime->types[i].icon != NULL)
		return mime->types[i].icon;
	strncpy(&buf[11], type, sizeof(buf)-11);
	for(; (p = strchr(&buf[11], '/')) != NULL; *p = '-');
	if((mime->types[i].icon = gtk_icon_theme_load_icon(theme, buf, 48, 0,
					NULL)) == NULL)
	{
		if((p = strchr(&buf[11], '-')) != NULL)
		{
			*p = '\0';
			mime->types[i].icon = gtk_icon_theme_load_icon(theme,
					buf, 48, 0, NULL);
		}
	}
	return mime->types[i].icon;
}
