/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "cpp.h"
#include "common.h"


/* Cpp */
/* private */
/* types */
struct _CppDefine /* FIXME use a hash table */
{
	char * name;
	char * value;
};


/* public */
/* functions */
/* cpp_new */
Cpp * cpp_new(CppPrefs * prefs)
{
	Cpp * cpp;
	String * p;
	int r = 0;

	if((cpp = object_new(sizeof(*cpp))) == NULL)
		return NULL;
	memset(cpp, 0, sizeof(*cpp));
	cpp->options = prefs->options;
	cpp->parser = cppparser_new(cpp, NULL, prefs->filename,
			prefs->filters);
	if((p = string_new(prefs->filename)) != NULL)
	{
		r |= cpp_path_add(cpp, dirname(p)); /* FIXME inclusion order */
		string_delete(p);
	}
	if((p = getcwd(NULL, 0)) != NULL)
	{
		r |= cpp_path_add(cpp, p);
		free(p);
	}
	else
		error_set("%s%s", "getcwd: ", strerror(errno));
	if(r != 0 || cpp->parser == NULL || cpp->paths_cnt != 2)
	{
		cpp_delete(cpp);
		return NULL;
	}
	return cpp;
}


/* cpp_delete */
void cpp_delete(Cpp * cpp)
{
	size_t i;

	for(i = 0; i < cpp->defines_cnt; i++)
	{
		free(cpp->defines[i].name);
		free(cpp->defines[i].value);
	}
	free(cpp->defines);
	for(i = 0; i < cpp->paths_cnt; i++)
		free(cpp->paths[i]);
	free(cpp->paths);
	if(cpp->parser != NULL)
		cppparser_delete(cpp->parser);
	if(cpp->scopes != NULL)
		free(cpp->scopes);
	object_delete(cpp);
}


/* accessors */
/* cpp_get_filename */
char const * cpp_get_filename(Cpp * cpp)
{
	return cppparser_get_filename(cpp->parser);
}


/* useful */
/* cpp_define_add */
int cpp_define_add(Cpp * cpp, char const * name, char const * value)
{
	size_t i;
	CppDefine * p;
	char const * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\", \"%s\")\n", __func__, name,
			value);
#endif
	if(name == NULL || name[0] == '\0')
		return error_set_code(1, "%s", strerror(EINVAL));
	if(value == NULL)
		value = "";
	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			break;
	if(i != cpp->defines_cnt)
		return error_set_code(1, "%s is already defined", name);
	if(value != NULL) /* XXX not sure if this should be done here */
		for(q = cpp_define_get(cpp, value); q != NULL;
				q = cpp_define_get(cpp, value))
			value = q; /* XXX may deadloop */
	if((p = realloc(cpp->defines, sizeof(*p) * (cpp->defines_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->defines = p;
	p = &p[cpp->defines_cnt];
	p->name = string_new(name);
	p->value = string_new(value);
	if(p->name == NULL || p->value == NULL)
	{
		string_delete(p->name);
		string_delete(p->value);
		return 1;
	}
	cpp->defines_cnt++;
	return 0;
}


/* cpp_define_get */
char const * cpp_define_get(Cpp * cpp, char const * name)
{
	size_t i;

	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			return cpp->defines[i].value;
	return NULL;
}


/* cpp_define_remove */
int cpp_define_remove(Cpp * cpp, char const * name)
	/* FIXME should verify validity of name */
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\")\n", __func__, name);
#endif
	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			break;
	if(i == cpp->defines_cnt)
		return error_set_code(1, "%s is not defined", name);
	free(cpp->defines[i].name);
	free(cpp->defines[i].value);
	cpp->defines_cnt--;
	for(; i < cpp->defines_cnt; i++)
	{
		cpp->defines[i].name = cpp->defines[i + 1].name;
		cpp->defines[i].value = cpp->defines[i + 1].value;
	}
	return 0;
}


/* cpp_path_add */
int cpp_path_add(Cpp * cpp, char const * path)
{
	char ** p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\")\n", __func__, path);
#endif
	if((p = realloc(cpp->paths, sizeof(*p) * (cpp->paths_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths = p;
	if((p[cpp->paths_cnt] = strdup(path)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths_cnt++;
	return 0;
}


/* cpp_path_lookup */
static char * _lookup_error(char const * path);

String * cpp_path_lookup(Cpp * cpp, char const * filename)
{
	size_t len = strlen(filename);
	size_t i;
	char * buf = NULL;
	char * p;
	struct stat st;

	for(i = 0; i < cpp->paths_cnt; i++)
	{
		if((p = realloc(buf, strlen(cpp->paths[i]) + len + 2)) == NULL)
		{
			error_set("%s", strerror(errno));
			free(buf);
			return NULL;
		}
		buf = p;
		sprintf(buf, "%s/%s", cpp->paths[i], filename);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: stat(\"%s\", %p)\n", buf, &st);
#endif
		if(stat(buf, &st) == 0)
			return buf;
		if(errno != ENOENT)
			break;
	}
	free(buf);
	return _lookup_error(filename);
}

static char * _lookup_error(char const * filename)
{
	error_set("%s%s%s%s", "Cannot include <", filename, ">: ",
			strerror(errno));
	return NULL;
}
