/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY ITS AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE. */



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* pkg-config without glib without pkg-config without glib without pkg-config */
/* private */
/* types */
typedef struct _PkgConfigPrefs
{
	int cflags;
	int exists;
	int libs;
	int _static;
	int version;
} PkgConfigPrefs;

typedef struct _PkgConfigVariable
{
	char * name;
	char * value;
} PkgConfigVariable;

typedef struct _PkgConfig
{
	PkgConfigPrefs prefs;

	/* variables */
	char const * libdir;
	char const * path;

	/* parsing */
	PkgConfigVariable * variables;
	size_t variables_cnt;
} PkgConfig;


/* prototypes */
static int _pkgconfig(PkgConfigPrefs * prefs, int pkgc, char * pkgv[]);
static int _pkgconfig_error(int ret, char const * format, ...);

/* string */
static char * _string_new(char const * string);
static char * _string_new_append(char const * string, ...);
static void _string_delete(char * string);
static int _string_append(char ** string, char const * append);
static int _string_append_length(char ** string, char const * append,
		size_t length);

static int _usage(int brief);


/* functions */
/* pkgconfig */
static FILE * _pkgconfig_open(char const * path, char const * pkg);
static int _pkgconfig_parse(PkgConfig * pc, FILE * fp);
static int _pkgconfig_parse_directive(PkgConfig * pc, char const * directive,
		char const * value);
static int _pkgconfig_parse_requires(PkgConfig * pc, char const * requires);
static char * _pkgconfig_parse_substitute(PkgConfig * pc, char const * value);
static int _pkgconfig_parse_variable(PkgConfig * pc, char const * name,
		char const * value);

static int _pkgconfig(PkgConfigPrefs * prefs, int pkgc, char * pkgv[])
{
	int ret = 0;
	PkgConfig pc;
	char const format[] = "Package %s was not found in the pkg-config"
		" search path.\n"
		"Perhaps you should add the directory containing `%s.pc'\n"
		"to the PKG_CONFIG_PATH environment variable\n"
		"No package '%s' found\n";
	char const * p;
	int i;
	FILE * fp;

	memset(&pc, 0, sizeof(pc));
	/* default values */
	pc.libdir = "/usr/lib/pkgconfig"
		":" PREFIX "/lib/pkgconfig"
		":" PREFIX "/share/pkgconfig";
	/* preferences */
	memcpy(&pc.prefs, prefs, sizeof(pc.prefs));
	/* environment variables */
	if((p = getenv("PKG_CONFIG_LIBDIR")) != NULL)
		pc.libdir = p;
	if((p = getenv("PKG_CONFIG_PATH")) != NULL)
		pc.path = p;
	/* packages */
	for(i = 0; i < pkgc; i++)
	{
		if((fp = _pkgconfig_open(pc.libdir, pkgv[i])) == NULL)
			fp = _pkgconfig_open(pc.path, pkgv[i]);
		if(fp == NULL)		
		{
			fprintf(stderr, format, pkgv[i], pkgv[i], pkgv[i]);
			ret |= 1;
			continue;
		}
		ret |= _pkgconfig_parse(&pc, fp);
		fclose(fp);
	}
	if(!prefs->exists)
	{
		if(prefs->libs && prefs->_static)
			/* FIXME do not output -Wl,R -Wl,-rpath ... */
			fputs(" -static", stdout);
		putchar('\n');
	}
	return ret;
}

static FILE * _pkgconfig_open(char const * path, char const * pkg)
{
	FILE * fp;
	size_t i;
	char * p;
	char * q;
	char c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, path, pkg);
#endif
	if(path == NULL)
		return NULL;
	if((p = _string_new(path)) == NULL)
		return NULL;
	for(i = 0;; i++)
	{
		if(p[i] != '\0' && p[i] != ':')
			continue;
		c = p[i];
		p[i] = '\0';
		if((q = _string_new_append(p, "/", pkg, ".pc", NULL))
				== NULL)
			return NULL;
		fp = fopen(q, "r");
		_string_delete(q);
		if(fp != NULL)
		{
			_string_delete(p);
			return fp;
		}
		if(c == '\0')
			break;
		p += i + 1;
		i = 0;
	}
	_string_delete(p);
	return NULL;
}

static int _pkgconfig_parse(PkgConfig * pc, FILE * fp)
{
	PkgConfigVariable * v;
	size_t v_cnt;
	char * line;
	size_t len = 256;
	size_t i;
	int c;

	/* FIXME ugly hack */
	v = pc->variables;
	v_cnt = pc->variables_cnt;
	pc->variables = NULL;
	pc->variables_cnt = 0;
	if((line = malloc(len)) == NULL)
		return -1;
	while(fgets(line, len, fp) != NULL)
	{
		/* FIXME complete the line if necessary */
		i = strlen(line);
		if(line[i - 1] == '\n')
			line[i - 1] = '\0';
		/* detect empty lines or comments */
		for(i = 0; (c = line[i]) != '\0' && isspace(c); i++);
		if(line[i] == '\0' || line[i] == '#')
			continue;
		/* look for a '=' or a ':' in the line */
		for(i = 0; (c = line[i]) != '\0' && (isalnum(c) || c == '_'
					|| c == '.'); i++);
		if(line[i] == '=')
		{
			line[i] = '\0';
			for(i += 1; (c = line[i]) != '\0' && isspace(c); i++);
			if(_pkgconfig_parse_variable(pc, line, &line[i]) != 0)
				return -1;
		}
		else if(line[i] == ':')
		{
			line[i] = '\0';
			for(i += 1; (c = line[i]) != '\0' && isspace(c); i++);
			if(_pkgconfig_parse_directive(pc, line, &line[i]) != 0)
				return -1;
		}
#ifdef DEBUG
		else
			fprintf(stderr, "DEBUG: %s\n", line);
#endif
	}
	free(line);
	pc->variables = v;
	pc->variables_cnt = v_cnt;
	return 0;
}

static int _pkgconfig_parse_directive(PkgConfig * pc, char const * directive,
		char const * value)
{
	int ret = 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, directive,
			value);
#endif
	if((p = _pkgconfig_parse_substitute(pc, value)) == NULL)
		return -1;
	if(strcmp(directive, "Requires") == 0)
		ret = _pkgconfig_parse_requires(pc, p);
	else if(pc->prefs.exists)
		ret = 0;
	/* FIXME parse and store arguments for later instead */
	else if(strcmp(directive, "Cflags") == 0 && pc->prefs.cflags)
		printf(" %s", p);
	else if(strcmp(directive, "Libs") == 0 && pc->prefs.libs)
		printf(" %s", p);
	/* FIXME implement the rest */
#ifdef DEBUG
	else
		fprintf(stderr, "DEBUG: %s() \"%s\", \"%s\"\n", __func__,
				directive, value);
#endif
	free(p);
	return ret;
}

static int _pkgconfig_parse_requires(PkgConfig * pc, char const * requires)
{
	int ret = 0;
	char * p;
	size_t i;
	int c;
	FILE * fp;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, requires);
#endif
	if((p = _pkgconfig_parse_substitute(pc, requires)) == NULL)
		return -1;
	for(i = 0;; i++)
	{
		if((c = p[i]) != '\0' && !isspace(c) && c != ',')
			continue;
		p[i] = '\0';
		/* FIXME code duplication */
		fp = _pkgconfig_open(pc->libdir, p);
		if(fp == NULL)
			fp = _pkgconfig_open(pc->path, p);
		if(fp != NULL)
		{
			ret |= _pkgconfig_parse(pc, fp);
			fclose(fp);
		}
		else
			ret |= _pkgconfig_error(1, "%s: %s", p, strerror(
						errno));
		if(c == '\0')
			break;
		for(i++; (c = p[i]) != '\0' && isspace(c); i++);
		if(c == '\0')
			break;
		p += i;
		i = 0;
	}
	_string_delete(p);
	return ret;
}

static char * _pkgconfig_parse_substitute(PkgConfig * pc, char const * value)
{
	char * ret = NULL;
	size_t i;
	size_t j;
	size_t k;

	for(i = 0;; i++)
	{
		if(value[i] != '$' || value[i + 1] != '{')
		{
			if(value[i] == '\0')
				break;
			/* XXX not efficient */
			if(_string_append_length(&ret, &value[i], 1) != 0)
			{
				free(ret);
				return NULL;
			}
			continue;
		}
		for(j = i + 2; value[j] != '\0' && value[j] != '}'; j++);
		if(value[j] != '}')
		{
			free(ret);
			return NULL;
		}
		for(k = 0; k < pc->variables_cnt; k++)
			if(strncmp(pc->variables[k].name, &value[i + 2],
						j - i - 2) == 0)
				break;
		if(k == pc->variables_cnt)
		{
			/* FIXME report error */
			free(ret);
			return NULL;
		}
		if(_string_append(&ret, pc->variables[k].value) != 0)
		{
			/* FIXME report error */
			free(ret);
			return NULL;
		}
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s => %s\n", pc->variables[k].name,
				pc->variables[k].value);
#endif
		i = j;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => \"%s\"\n", __func__, ret);
#endif
	return ret;
}

static int _pkgconfig_parse_variable(PkgConfig * pc, char const * name,
		char const * value)
{
	PkgConfigVariable * p;
	char * q;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, name, value);
#endif
	if((q = _pkgconfig_parse_substitute(pc, value)) == NULL)
		return -1;
	/* check if the variable already exists */
	for(i = 0; i < pc->variables_cnt; i++)
		if(strcmp(pc->variables[i].name, name) == 0)
		{
			free(pc->variables[i].value);
			pc->variables[i].value = q;
			return 0;
		}
	/* allocate a new variable */
	if((p = realloc(pc->variables, sizeof(*p) * (pc->variables_cnt + 1)))
			== NULL)
		return -_pkgconfig_error(1, "%s", strerror(errno));
	pc->variables = p;
	p = &pc->variables[i];
	p->value = q;
	if((p->name = strdup(name)) == NULL)
	{
		free(p->name);
		free(p->value);
		return -_pkgconfig_error(1, "%s", strerror(errno));
	}
	pc->variables_cnt++;
	return 0;
}


/* pkgconfig_error */
static int _pkgconfig_error(int ret, char const * format, ...)
{
	va_list ap;

	fputs("pkg-config: ", stderr);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	return ret;
}


/* string */
/* string_new */
static char * _string_new(char const * string)
{
	return strdup(string);
}


/* string_new_append */
static char * _string_new_append(char const * string, ...)
{
	char * ret;
	va_list ap;
	char const * q;

	if(string == NULL || (ret = _string_new(string)) == NULL)
		return NULL;
	va_start(ap, string);
	while((q = va_arg(ap, char const *)) != NULL)
		if(_string_append(&ret, q) != 0)
		{
			_string_delete(ret);
			return NULL;
		}
	va_end(ap);
	return ret;
}


/* string_delete */
static void _string_delete(char * string)
{
	free(string);
}


/* string_append */
static int _string_append(char ** string, char const * append)
{
	size_t slen = (*string != NULL) ? strlen(*string) : 0;
	size_t alen = (append != NULL) ? strlen(append) : 0;
	char * p;

	if(alen == 0)
		return 0;
	if((p = realloc(*string, slen + alen + 1)) == NULL)
		return -1;
	*string = p;
	strcpy(*string + slen, append);
	return 0;
}


/* string_append_length */
static int _string_append_length(char ** string, char const * append,
		size_t length)
{
	size_t slen = (*string != NULL) ? strlen(*string) : 0;
	size_t alen = (append != NULL) ? length : 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", %zu)\n", __func__, *string,
			append, length);
#endif
	if(alen == 0)
		return 0;
	if((p = realloc(*string, slen + alen + 1)) == NULL)
		return -1;
	*string = p;
	strncpy(*string + slen, append, alen);
	(*string)[slen + alen] = '\0';
	return 0;
}


/* usage */
static int _usage(int brief)
{
	if(brief)
		fputs("Usage: pkg-config [-?] [--version] [--modversion]\n",
				stderr);
	else
		fputs("Usage: pkg-config [OPTIONS...] [PACKAGES...]\n"
"  --libs	Output all linker flags\n"
"  --static	Output linker flags for static linking\n"
"  --version	Output version of pkg-config\n"
"\n"
"Help options:\n"
"  -?, --help	Show this help message\n"
"  --usage	Display brief usage message\n", stderr);
	return 1;
}


/* main */
static int _main_option(PkgConfigPrefs * prefs, char const * option);
static int _main_option_cflags(PkgConfigPrefs * prefs);
static int _main_option_exists(PkgConfigPrefs * prefs);
static int _main_option_libs(PkgConfigPrefs * prefs);
static int _main_option_static(PkgConfigPrefs * prefs);
static int _main_option_usage(PkgConfigPrefs * prefs);
static int _main_option_version(PkgConfigPrefs * prefs);
static struct
{
	char const * option;
	int (*callback)(PkgConfigPrefs * prefs);
} _main_options[] = {
	{ "cflags",	_main_option_cflags	},
	{ "exists",	_main_option_exists	},
	{ "libs",	_main_option_libs	},
	{ "static",	_main_option_static	},
	{ "usage",	_main_option_usage	},
	{ "version",	_main_option_version	}
};

int main(int argc, char * argv[])
{
	PkgConfigPrefs prefs;
	int optind;

	memset(&prefs, 0, sizeof(prefs));
	/* getopt() is too complicated for GNU */
	/* FIXME stupid GNU accepts options even after actual arguments */
	/* -- khorben: no want fix it cause this meant as troll */
	for(optind = 1; optind < argc; optind++)
	{
		if(strcmp(argv[optind], "-?") == 0)
			return _usage(0);
		if(strncmp(argv[optind], "--", 2) != 0)
			break;
		if(argv[optind][2] == '\0')
		{
			optind++;
			break;
		}
		if(_main_option(&prefs, argv[optind]) != 0)
			return 1;
	}
	/* check if any package was specified */
	if(optind == argc)
	{
		fputs("Must specify package names on the command line\n",
				stderr);
		return 1;
	}
	return _pkgconfig(&prefs, argc - optind, &argv[optind]);
}

static int _main_option(PkgConfigPrefs * prefs, char const * option)
{
	size_t i;

	for(i = 0; i < sizeof(_main_options) / sizeof(*_main_options); i++)
		if(strcmp(_main_options[i].option, &option[2]) == 0)
			return _main_options[i].callback(prefs);
	fprintf(stderr, "%s: Unknown option\n", option);
	return 1;
}

static int _main_option_cflags(PkgConfigPrefs * prefs)
{
	prefs->cflags = 1;
	return 0;
}

static int _main_option_exists(PkgConfigPrefs * prefs)
{
	prefs->exists = 1;
	return 0;
}

static int _main_option_libs(PkgConfigPrefs * prefs)
{
	prefs->libs = 1;
	return 0;
}

static int _main_option_static(PkgConfigPrefs * prefs)
{
	prefs->_static = 1;
	return 0;
}

static int _main_option_usage(PkgConfigPrefs * prefs)
{
	return _usage(1);
}

static int _main_option_version(PkgConfigPrefs * prefs)
{
	prefs->version = 1;
	return 0;
}
