/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
void config_delete(Config * config)
{
	config_reset(config);
	hash_delete(config);
}


/* accessors */
/* config_get */
char const * config_get(Config * config, char const * section,
		char const * variable)
{
	Hash * h;
	char const * value;

	if(section == NULL)
		section = "";
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
	char * v = NULL;

	if(section == NULL)
		section = "";
	if((hash = hash_get(config, section)) == NULL)
	{
		/* create a new section */
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
	else
		/* to free the current value if already set */
		v = hash_get(hash, variable);
	if((p = string_new(variable)) == NULL)
		return 1;
	if(value == NULL)
	{
		if(hash_set(hash, p, NULL) == 0)
		{
			string_delete(v);
			return 0;
		}
		string_delete(p);
		return 1;
	}
	if((q = string_new(value)) == NULL)
	{
		string_delete(p);
		return 1;
	}
	if(hash_set(hash, p, q) == 0)
	{
		string_delete(v);
		return 0;
	}
	string_delete(p);
	string_delete(q);
	return 1;
}


/* useful */
/* config_load */
static int _load_isprint(int c);
static String * _load_section(FILE * fp);
static String * _load_variable(FILE * fp, int c);
static String * _load_value(FILE * fp);

int config_load(Config * config, char const * filename)
{
	int ret = 0;
	size_t line;
	FILE * fp;
	String * section;
	String * variable = NULL;
	int c;
	String * str;

	if((section = string_new("")) == NULL)
		return 1;
	if((fp = fopen(filename, "r")) == NULL)
	{
		string_delete(section);
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	}
	for(line = 0; (c = fgetc(fp)) != EOF; line++)
		if(c == '#')
			while((c = fgetc(fp)) != EOF && c != '\n');
		else if(c == '[')
		{
			if((str = _load_section(fp)) == NULL)
				break;
			string_delete(section);
			section = str;
		}
		else if(_load_isprint(c))
		{
			if((str = _load_variable(fp, c)) == NULL)
				break;
			string_delete(variable);
			variable = str;
			if((str = _load_value(fp)) == NULL)
				break;
			/* XXX optimize string alloc/free, and may fail */
			config_set(config, section, variable, str);
			string_delete(str);
		}
		else if(c != '\n')
			break;
	string_delete(section);
	string_delete(variable);
	if(!feof(fp))
		ret = error_set_code(1, "%s: %s%zd", filename, "Syntax error"
				" at line ", line);
	if(fclose(fp) != 0)
		ret = error_set_code(1, "%s: %s", filename, strerror(errno));
	return ret;
}

static int _load_isprint(int c)
{
	if(c == '\n' || c == '\0')
		return 0;
	return 1;
}

static String * _load_section(FILE * fp)
{
	int c;
	char * str = NULL;
	size_t len = 0;
	char * p;

	while((c = fgetc(fp)) != EOF && c != ']' && _load_isprint(c))
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
		return string_new("");
	str[len] = '\0';
	return str;
}

static String * _load_variable(FILE * fp, int c)
{
	String * str;
	String buf[2] = "\0";

	buf[0] = c;
	if((str = string_new(buf)) == NULL)
		return NULL;
	while((c = fgetc(fp)) != EOF && c != '=' && _load_isprint(c))
	{
		buf[0] = c;
		if(string_append(&str, buf) != 0)
		{
			string_delete(str);
			return NULL;
		}
	}
	if(c != '=')
	{
		string_delete(str);
		return NULL;
	}
	return str;
}

static String * _load_value(FILE * fp)
{
	int c;
	String * str = NULL;
	size_t len = 0;
	char * p;

	while((c = fgetc(fp)) != EOF && _load_isprint(c))
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
static void _delete_foreach(void const * key, void * value, void * data);
static void _delete_foreach_section(void const * key, void * value,
		void * data);

int config_reset(Config * config)
{
	hash_foreach(config, _delete_foreach, NULL);
	return hash_reset(config);
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


/* config_save */
void _save_foreach_default(void const * key, void * value, void * data);
void _save_foreach(void const * key, void * value, void * data);
void _save_foreach_section(void const * key, void * value, void * data);

int config_save(Config * config, char const * filename)
{
	FILE * fp;

	if((fp = fopen(filename, "w")) == NULL)
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	hash_foreach(config, _save_foreach_default, &fp);
	hash_foreach(config, _save_foreach, &fp);
	if(fp == NULL || fclose(fp) != 0)
		return error_set_code(1, "%s: %s", filename, strerror(errno));
	return 0;
}

void _save_foreach_default(void const * key, void * value, void * data)
{
	FILE ** fp = data;
	char const * section = key;
	Hash * hash = value;

	if(*fp == NULL)
		return;
	if(section[0] != '\0')
		return;
	hash_foreach(hash, _save_foreach_section, fp);
}

void _save_foreach(void const * key, void * value, void * data)
{
	FILE ** fp = data;
	char const * section = key;
	Hash * hash = value;

	if(*fp == NULL)
		return;
	if(section[0] == '\0')
		return;
	if(fprintf(*fp, "\n[%s]\n", section) < 0)
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

	if(*fp == NULL)
		return;
	if(val == NULL || fprintf(*fp, "%s=%s\n", var, val) >= 0)
		return;
	fclose(*fp);
	*fp = NULL;
}
