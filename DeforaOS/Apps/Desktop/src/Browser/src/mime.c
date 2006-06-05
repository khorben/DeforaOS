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
	FILE * fp;
	char buf[256];
	size_t len;
	char * glob;
	MimeType * p;

	if((mime = malloc(sizeof(*mime))) == NULL)
		return NULL;
	if((fp = fopen("/usr/share/mime/globs", "r")) == NULL)
	{
		perror("/usr/share/mime/globs"); /* FIXME */
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
		p[mime->types_cnt++].glob = strdup(glob);
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
