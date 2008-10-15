/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "System.h"


/* Config */
/* public */
/* functions */
/* config_new */
Config * config_new(void)
{
	return hash_new(hash_func_string, hash_compare_string);
}


/* config_delete */
static void _delete_foreach(void const * key, void * value, void * data);
static void _delete_foreach_section(void const * key, void * value,
		void * data);

void config_delete(Config * config)
{
	hash_foreach(config, _delete_foreach, NULL);
	hash_delete(config);
}

static void _delete_foreach(void const * key, void * value, void * data)
{
	char * str = (char*)key;
	Hash * hash = value;

	free(str);
	hash_foreach(hash, _delete_foreach_section, data);
	hash_delete(hash);
}

static void _delete_foreach_section(void const * key, void * value, void * data)
{
	char * k = (char*)key;
	char * v = value;

	free(k);
	free(v);
}


/* accessors */
/* config_get */
char const * config_get(Config * config, char const * section,
		char const * variable)
{
	Hash * h;
	char const * value;

	if((h = hash_get(config, section)) != NULL) /* found section */
	{
		if((value = hash_get(h, variable)) != NULL) /* found value */
			return value;
		if(section[0] == '\0')
			error_set_code(1, "%s%s", variable, ": Not found in"
					" default section");
		else
			error_set_code(1, "%s%s%s", variable, ": Not found in"
					" section ", section);
		return NULL;
	}
	if(section[0] == '\0')
		error_set_code(1, "%s", "default section: Not found");
	else
		error_set_code(1, "%s%s%s", "section ", section, ": Not found");
	return NULL;
}


/* config_set */
int config_set(Config * config, char const * section, char const * variable,
		char const * value)
{
	Hash * hash;
	char * p;
	char * q;

	if((hash = hash_get(config, section)) == NULL)
	{
		if((hash = hash_new(hash_func_string, hash_compare_string))
				== NULL)
			return 1;
		if((p = string_new(section)) == NULL
				|| hash_set(config, p, hash) != 0)
		{
			free(p);
			hash_delete(hash);
			return 1;
		}
	}
	if((p = string_new(variable)) == NULL)
		return 1;
	if(value == NULL)
	{
		if(hash_set(hash, p, NULL) == 0)
			return 0;
		string_delete(p);
		return 1;
	}
	if((q = string_new(value)) == NULL)
	{
		string_delete(p);
		return 1;
	}
	if(hash_set(hash, p, q) == 0)
		return 0;
	string_delete(p);
	string_delete(q);
	return 1;
}


/* useful */
/* config_load */
static char * _load_section(FILE * fp);
static char * _load_variable(FILE * fp, int c);
static char * _load_value(FILE * fp);

int config_load(Config * config, char const * filename)
{
	int ret = 0;
	FILE * fp;
	char * section;
	char * variable = NULL;
	int c;
	char * str;

	if((section = string_new("")) == NULL)
		return 1;
	if((fp = fopen(filename, "r")) == NULL)
	{
		free(section);
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	}
	while((c = fgetc(fp)) != EOF)
		if(c == '#')
			while((c = fgetc(fp)) != EOF && c != '\n');
		else if(c == '[')
		{
			if((str = _load_section(fp)) == NULL)
				break;
			free(section);
			section = str;
		}
		else if(isprint(c))
		{
			if((str = _load_variable(fp, c)) == NULL)
				break;
			free(variable);
			variable = str;
			if((str = _load_value(fp)) == NULL)
				break;
			/* XXX optimize string alloc/free */
			config_set(config, section, variable, str);
			free(str);
		}
		else if(c != '\n')
			break;
	free(section);
	free(variable);
	if(!feof(fp))
		ret = error_set_code(1, "%s: %s", filename, "Syntax error");
	if(fclose(fp) != 0)
		ret = error_set_code(1, "%s: %s", filename, strerror(errno));
	return ret;
}

static char * _load_section(FILE * fp)
{
	int c;
	char * str = NULL;
	size_t len = 0;
	char * p;

	while((c = fgetc(fp)) != EOF && c != ']' && isprint(c))
	{
		if((p = realloc(str, sizeof(*str) * (len + 2))) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = c;
	}
	if(c != ']')
	{
		free(str);
		return NULL;
	}
	if(str == NULL)
		return strdup("");
	str[len] = '\0';
	return str;
}

static char * _load_variable(FILE * fp, int c)
{
	char * str;
	int len = 1;
	char * p;

	if((str = malloc(sizeof(char) * (len+1))) == NULL)
		return NULL;
	str[0] = c;
	while((c = fgetc(fp)) != EOF && c != '=' && isprint(c))
	{
		if((p = realloc(str, sizeof(*str) * (len + 2))) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = c;
	}
	if(c != '=')
	{
		free(str);
		return NULL;
	}
	str[len] = '\0';
	return str;
}

static char * _load_value(FILE * fp)
{
	int c;
	char * str = NULL;
	int len = 0;
	char * p;

	while((c = fgetc(fp)) != EOF && isprint(c))
	{
		if((p = realloc(str, sizeof(*str) * (len + 2))) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = c;
	}
	if(c != '\n')
	{
		free(str);
		return NULL;
	}
	if(str == NULL)
		return string_new("");
	str[len] = '\0';
	return str;
}


/* config_reset */
void config_reset(Config * config)
{
	/* FIXME untested */
	hash_foreach(config, _delete_foreach, NULL);
}


/* config_save */
void _save_foreach(void const * key, void * value, void * data);
void _save_foreach_section(void const * key, void * value, void * data);

int config_save(Config * config, char const * filename)
{
	FILE * fp;

	if((fp = fopen(filename, "w")) == NULL)
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	hash_foreach(config, _save_foreach, &fp);
	if(fp == NULL || fclose(fp) != 0)
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	return 0;
}

void _save_foreach(void const * key, void * value, void * data)
{
	FILE ** fp = data;
	char const * section = key;
	Hash * hash = value;

	if(*fp == NULL)
		return;
	if(section[0] != '\0' && fprintf(*fp, "[%s]\n", section) < 0)
	{
		fclose(*fp);
		*fp = NULL;
		return;
	}
	hash_foreach(hash, _save_foreach_section, fp);
}

void _save_foreach_section(void const * key, void * value, void * data)
{
	FILE ** fp = data;
	char const * var = key;
	char const * val = value;

	if(fprintf(*fp, "%s=%s\n", var, val) >= 0)
		return;
	fclose(*fp);
	*fp = NULL;
}
