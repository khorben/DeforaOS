/* config.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "config.h"


/* Config */
Config * config_new(void)
{
	Config * config;

	if((config = hash_new()) == NULL)
		return NULL;
	return config;
}

/* FIXME should be an API to avoid this */
typedef struct _HashEntry
{
	char * name;
	void * data;
} HashEntry;
void config_delete(Config * config)
{
	unsigned int i;
	HashEntry * hi;
	int j;
	HashEntry * hj;

	for(i = array_count(config); i > 0; i--)
	{
		hi = array_get(config, i - 1);
		for(j = array_count(hi->data); j > 0; j--)
		{
			hj = array_get(hi->data, j - 1);
			free(hj->data);
		}
		hash_delete(hi->data);
	}
	hash_delete(config);
}


/* useful */
char * config_get(Config * config, char const * section, char const * variable)
{
	void * p;

	if((p = hash_get(config, section)) == NULL)
		return NULL;
	return hash_get(p, variable);
}


int config_set(Config * config, char const * section, char const * variable,
		char const * value)
{
	Hash * h;
	char * p;

	if((h = hash_get(config, section)) == NULL)
	{
		if((h = hash_new()) == NULL)
			return 1;
		if(hash_set(config, section, h) == 1)
		{
			hash_delete(h);
			return 1;
		}
	}
	if((p = string_new(value)) == NULL)
		return 1;
	if(hash_set(h, variable, p) == 0)
		return 0;
	free(p);
	return 1;
}


static char * _load_section(FILE * fp);
static char * _load_variable(FILE * fp, int c);
static char * _load_value(FILE * fp);
int config_load(Config * config, char const * filename)
{
	FILE * fp;
	char * section;
	char * variable = NULL;
	int c;
	char * str;
	int ret = 0;

	if((section = string_new("")) == NULL
			|| (fp = fopen(filename, "r")) == NULL)
	{
		free(section);
		return 1;
	}
	while((c = fgetc(fp)) != EOF)
	{
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
			config_set(config, section, variable, str);
			free(str);
		}
		else if(c != '\n')
			break;
	}
	free(section);
	free(variable);
	if(!feof(fp))
	{
		errno = EINVAL;
		ret = 1;
	}
	fclose(fp);
	return ret;
}

static char * _load_section(FILE * fp)
{
	int c;
	char * str = NULL;
	int len = 0;
	char * p;

	while((c = fgetc(fp)) != EOF && c != ']' && isprint(c))
	{
		if((p = realloc(str, sizeof(char) * (len+2))) == NULL)
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
		if((p = realloc(str, sizeof(char) * (len+2))) == NULL)
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
		if((p = realloc(str, sizeof(char) * (len+2))) == NULL)
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


static int _save_section(Hash * h, unsigned int i, FILE * fp);
static int _save_variables(Hash * h, FILE * fp);
int config_save(Config * config, char const * filename)
{
	FILE * fp;
	unsigned int i;
	unsigned int j;
	int ret = 0;

	if((i = array_count(config)) == 0)
		return 1;
	if((fp = fopen(filename, "w")) == NULL)
	{
		fprintf(stderr, "%s", "libutils: ");
		perror(filename);
		return 1;
	}
	for(j = 0; j < i; j++)
		if((ret = _save_section(config, j, fp)) != 0)
			break;
	fclose(fp);
	return ret;
}

static int _save_section(Hash * h, unsigned int i, FILE * fp)
{
	HashEntry * he;

	he = array_get(h, i);
	if(he->name[0] != '\0')
	{
		if(fprintf(fp, "[%s]\n", he->name) < 0)
			return 1;
	}
	else if(i != 0)
		if(fwrite("[]\n", sizeof(char), 3, fp) != 3)
			return 1;
	if(_save_variables(he->data, fp) != 0)
		return 1;
	return fputc('\n', fp) == '\n' ? 0 : 1;
}

static int _save_variables(Hash * h, FILE * fp)
{
	unsigned int i;
	unsigned int j;
	HashEntry * he;

	i = array_count(h);
	for(j = 0; j < i; j++)
	{
		he = array_get(h, j);
		if(he->name == NULL || he->data == NULL)
			continue;
		if(fprintf(fp, "%s=%s\n", he->name, (char*)he->data) < 0)
			return 1;
	}
	return 0;
}
