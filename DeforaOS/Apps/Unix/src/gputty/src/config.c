/* config.c */
/* Copyright (C) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "config.h"


/* Config */
Config * config_new(void)
{
	Config * config;

	if((config = hash_new()) == NULL)
		return NULL;
	return config;
}

void config_delete(Config * config)
{
	free(config);
}


/* useful */
char * config_get(Config * config, char * section, char * variable)
{
	void * p;

	if((p = hash_get(config, section)) == NULL)
		return NULL;
	return hash_get(p, variable);
}

int config_set(Config * config, char * section, char * variable, char * value)
{
	Hash * h;

	if((h = hash_get(config, section)) != NULL)
		return hash_set(h, variable, value);
	if((h = hash_new()) == NULL)
		return 1;
	if(hash_set(config, section, h) == 1)
	{
		hash_delete(h);
		return 1;
	}
	return hash_set(h, variable, value);
}


static char * _load_section(FILE * fp);
static char * _load_variable(FILE * fp, int c);
static char * _load_value(FILE * fp);
int config_load(Config * config, char * filename)
{
	FILE * fp;
	char * section;
	char * variable = NULL;
	int c;
	char * str;
	int ret;

	if((section = strdup("")) == NULL)
	{
		fprintf(stderr, "%s", "gputty: ");
		perror("strdup");
		return 1;
	}
	if((fp = fopen(filename, "r")) == NULL)
		return 1;
	while((c = fgetc(fp)) != EOF)
	{
		if(c == '#')
			while((c = fgetc(fp)) != EOF && 'c' != '\n');
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
#ifdef DEBUG
			fprintf(stderr, "%s%p, %s, %s, %s%s",
					"config_set(", config,
					section, variable, str, ");\n");
#endif
			config_set(config, section, variable, str);
		}
		else if(c != '\n')
			break;
	}
	free(section);
	free(variable);
	if(!feof(fp))
	{
		ret = 1;
		fprintf(stderr, "%s", "gputty: error in configuration file\n");
	}
	fclose(fp);
	return ret;
}

static char * _load_section(FILE * fp)
{
	int c;
	char * str = NULL;
	int len = 1;
	char * p;

	while((c = fgetc(fp)) != EOF && c != ']' && isprint(c))
	{
		if((p = realloc(str, len + 1)) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++ - 1] = c;
	}
	if(c != ']')
	{
		free(str);
		return NULL;
	}
	str[len - 1] = '\0';
	return str;
}

static char * _load_variable(FILE * fp, int c)
{
	char * str;
	int len = 2;
	char * p;

	if((str = malloc(len)) == NULL)
		return NULL;
	str[0] = c;
	while((c = fgetc(fp)) != EOF && c != '=' && isprint(c))
	{
		if((p = realloc(str, len + 1)) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++ - 1] = c;
	}
	if(c != '=')
	{
		fprintf(stderr, "%d ici\n", c);
		free(str);
		return NULL;
	}
	str[len - 1] = '\0';
	return str;
}

static char * _load_value(FILE * fp)
{
	int c;
	char * str = NULL;
	int len = 1;
	char * p;

	while((c = fgetc(fp)) != EOF && isprint(c))
	{
		if((p = realloc(str, len + 1)) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len++ - 1] = c;
	}
	if(c != '\n')
	{
		free(str);
		return NULL;
	}
	str[len - 1] = '\0';
	return str;
}

/* FIXME */
typedef struct _HashEntry {
	char * name;
	void * data;
} HashEntry;
static void _save_section(Hash * h, unsigned int i, FILE * fp);
static void _save_variables(Hash * h, FILE * fp);
int config_save(Config * config, char * filename)
{
	FILE * fp;
	unsigned int i;
	unsigned int j;

	if((i = array_get_size(config)) == 0)
		return 1;
	if((fp = fopen(filename, "w")) == NULL)
	{
		fprintf(stderr, "%s", "gputty: ");
		perror(filename);
		return 1;
	}
	for(j = 0; j < i; j++)
		_save_section(config, j, fp);
	fclose(fp);
	return 0;
}

static void _save_section(Hash * h, unsigned int i, FILE * fp)
{
	HashEntry * he;

	he = array_get(h, i);
	if(he->name[0] != '\0')
		fprintf(fp, "[%s]\n", he->name);
	else if(i != 0)
		fwrite("[]\n", sizeof(char), 3, fp);
	_save_variables(he->data, fp);
	fwrite("\n", sizeof(char), 1, fp);
}

static void _save_variables(Hash * h, FILE * fp)
{
	unsigned int i;
	unsigned int j;
	HashEntry * he;

	i = array_get_size(h);
	for(j = 0; j < i; j++)
	{
		he = array_get(h, j);
		fprintf(fp, "%s=%s\n", he->name, (char*)he->data);
	}
}
