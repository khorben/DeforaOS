/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* Copyright (c) 2012 Baptiste Daroussin <bapt@FreeBSD.org> */
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



#include <sys/param.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "../config.h"

#ifndef __unused
# define __unused
#endif
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif

#define PKG_CFLAGS		(1 << 0)
#define PKG_CFLAGS_ONLY_I	(1 << 1)
#define PKG_CFLAGS_ONLY_OTHERS	(1 << 2)
#define PKG_LIBS		(1 << 3)
#define PKG_LIBS_ONLY_L		(1 << 4)
#define PKG_LIBS_ONLY_l		(1 << 5)
#define PKG_LIBS_ONLY_OTHERS	(1 << 6)
#define PKG_DESCRIPTION		(1 << 7)
#define PKG_URL			(1 << 8)
#define PKG_STATIC		(1 << 9)
#define PKG_VERSION		(1 << 10)
#define PKG_EXISTS		(1 << 11)
#define PKG_MODVERSION		(1 << 12)
#define PKG_PRINT_REQUIRES	(1 << 13)
#define PKG_PRINT_REQUIRES_PRIV	(1 << 14)


/* pkg-config without glib without pkg-config without glib without pkg-config */
/* private */
/* types */
typedef struct _ListData {
	void *data;
	void (*freefn)(void *free);
} ListData;

typedef struct _PkgList {
	size_t cap;
	size_t len;
	ListData **data;
} PkgList;

typedef struct _PkgConfigVariable
{
	char * name;
	char * value;
} PkgConfigVariable;

typedef enum {
	GT,
	LT,
	GE,
	LE,
	EQ
} operator;

typedef struct _Pkg
{
	char *pkgname;
	char *name;
	char *version;
	char *description;
	char *url;
	PkgList *cflags;
	PkgList *cflags_private;
	PkgList *libs;
	PkgList *libs_private;
	PkgList *requires;
	PkgList *requires_private;
	PkgList *variables;
} Pkg;

typedef struct _PkgRequires
{
	char *name;
	char *version;
	operator op;
	Pkg *pkg;
} PkgRequires;

typedef struct _PkgConfig
{
	/* variables */
	unsigned int flags;

	PkgList *pc_dirs;
	PkgList *pkgs;
} PkgConfig;


/* prototypes */
static int _pkgconfig(PkgConfig * pc, int pkgc, char * pkgv[]);
static int _pkgconfig_error(int ret, char const * format, ...);

/* lists */
static PkgList * _pkglist_new(PkgList **list);
static void _pkglist_delete(PkgList *list);
static void * _pkglist_get(PkgList *list, size_t i);
static void _pkglist_append(PkgList *list, void *data, void (*freefn)(void *));

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
static Pkg * _pkg_new(Pkg **pkg, char const * pkgname);
static void _pkgconfig_variable_delete(PkgConfigVariable *p);
static FILE * _pkgconfig_open(PkgConfig * pc, char const * pkg);
static int _pkgconfig_parse(PkgConfig * p, FILE * fp);
static int _pkgconfig_parse_directive(PkgConfig * pc, Pkg * p, char const * directive,
		char const * value);
static char * _pkgconfig_parse_substitute(PkgList * v, char const * value);
static int _pkgconfig_parse_variable(PkgConfig * pc, Pkg * p, char const * name,
		char const * value);
static int _pkgconfig_parse_name(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_description(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_version(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_cflags(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_cflags_private(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_libs(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_libs_private(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_requires(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_requires_private(PkgConfig *pc, Pkg *p, char *data);
static int _pkgconfig_parse_requires_generic(PkgConfig *pc, Pkg *p, char *data, PkgList *lst);

struct keys {
	const char *key;
	int (*parse)(PkgConfig *, Pkg *, char *);
} keys [] = {
	{ "Name", _pkgconfig_parse_name  },
	{ "Description", _pkgconfig_parse_description },
	{ "Version", _pkgconfig_parse_version },
	{ "Cflags", _pkgconfig_parse_cflags },
	{ "Cflags.private", _pkgconfig_parse_cflags_private },
	{ "Libs", _pkgconfig_parse_libs },
	{ "Libs.private", _pkgconfig_parse_libs_private },
	{ "Requires", _pkgconfig_parse_requires },
	{ "Requires.private", _pkgconfig_parse_requires_private },
	{ NULL, NULL },
};

static PkgRequires * _pkgrequires_new(PkgRequires **p, char *name)
{
	if (*p != NULL)
		return *p;

	if ((*p = malloc(sizeof(PkgRequires))) == NULL) {
		_pkgconfig_error(1, "%s", strerror(errno));
		return NULL;
	}

	(*p)->name = strdup(name);
	(*p)->op = EQ;
	(*p)->version = NULL;
	(*p)->pkg = NULL;

	return *p;
}

static void _pkgrequires_delete(void *p1)
{
	PkgRequires *p = (PkgRequires *)p1;

	if (p == NULL)
		return;

	if (p->name != NULL)
		free(p->name);

	if (p->version != NULL)
		free(p->version);
}

static Pkg * _pkg_new(Pkg **pkg, char const * pkgname)
{
	if (*pkg != NULL)
		return *pkg;

	if ((*pkg = malloc(sizeof(Pkg))) == NULL) {
		_pkgconfig_error(1, "%s", strerror(errno));
		return NULL;
	}

	(*pkg)->name = NULL;
	(*pkg)->version = NULL;
	(*pkg)->description = NULL;
	(*pkg)->url = NULL;
	(*pkg)->pkgname = strdup(pkgname);
	(*pkg)->cflags = NULL;
	(*pkg)->cflags_private = NULL;
	(*pkg)->libs = NULL;
	(*pkg)->libs_private = NULL;
	(*pkg)->variables = NULL;
	(*pkg)->requires = NULL;
	(*pkg)->requires_private = NULL;

	return *pkg;
}

static void _pkg_delete(void *pkg)
{
	Pkg *p = (Pkg *)pkg;

	if (p->pkgname != NULL)
		free(p->pkgname);
	if (p->name != NULL)
		free(p->name);
	if (p->version != NULL)
		free(p->version);
	if (p->description != NULL)
		free(p->description);
	if (p->url != NULL)
		free(p->url);

	_pkglist_delete(p->cflags);
	_pkglist_delete(p->cflags_private);
	_pkglist_delete(p->libs);
	_pkglist_delete(p->libs_private);
	_pkglist_delete(p->variables);
	_pkglist_delete(p->requires);
	_pkglist_delete(p->requires_private);
}

static PkgConfigVariable *_pkgconfig_variable_new(PkgConfigVariable **p)
{
	if (*p != NULL)
		return *p;

	if ((*p = malloc(sizeof(PkgConfigVariable))) == NULL)
	{
		_pkgconfig_error(1, "%s", strerror(errno));
		return NULL;
	}

	(*p)->name = NULL;
	(*p)->value = NULL;

	return *p;
}

static void _pkgconfig_variable_delete(PkgConfigVariable *p)
{
	free(p->name);
	free(p->value);
	free(p);
}
static int _pkgconfig_parse_name(__unused PkgConfig *pc, Pkg *p, char *data)
{
	int ret = 0;

	if (p->name != NULL)
		free(p->name);

	p->name = strdup(data);

	return ret;
}

static int _pkgconfig_parse_description(PkgConfig *pc, Pkg *p, char *data)
{
	int ret = 0;

	if ((pc->flags & PKG_DESCRIPTION) == 0)
		return ret;

	if (p->description != NULL)
		free(p->description);

	p->description = strdup(data);

	return ret;
}

static int _pkgconfig_parse_version(PkgConfig *pc, Pkg *p, char *data)
{
	int ret = 0;

	if ((pc->flags & PKG_MODVERSION) == 0)
		return ret;

	if (p->version != NULL)
		free(p->version);

	p->version = strdup(data);

	return ret;
}

static int split_chr(char *str, char sep)
{
	char *next;
	char *buf = str;
	int nbel = 0;

	while ((next = strchr(buf, sep)) != NULL) {
		nbel++;
		buf = next;
		buf[0] = '\0';
		buf++;
	}

	return nbel;
}

static void _pkgconfig_add_to_list(PkgList *lst, char *data)
{
	size_t k;
	/* do not append something already appended */
	for (k = 0; k < lst->len; k++)
		if (strcmp((char *)_pkglist_get(lst, k), data) == 0)
			break;
	if (k == lst->len)
		_pkglist_append(lst, strdup(data), free);
}

static int _pkgconfig_parse_generic(PkgList *lst, char *data, unsigned int flags, unsigned int type)
{
	int ret = 0;
	int i;
	int nbel = 0;
	size_t next;
	char *walk;

	nbel = split_chr(data, ' ');

	next = strlen(data);
	walk = data;
	for (i = 0; i <= nbel; i++) {
		if (next != 0) {
			if (type == PKG_CFLAGS) {
				if (flags & PKG_CFLAGS_ONLY_I) {
					if (strncmp(walk, "-I", 2) == 0)
						_pkgconfig_add_to_list(lst, walk);
				} else if ((flags & PKG_CFLAGS_ONLY_OTHERS) == PKG_CFLAGS_ONLY_OTHERS) {
					if (strncmp(walk, "-I", 2) != 0)
						_pkgconfig_add_to_list(lst, walk);
				} else {
					_pkgconfig_add_to_list(lst, walk);
				}
			}

			if (type == PKG_LIBS) {
				if (flags & PKG_LIBS_ONLY_l) {
					if (strncmp(walk, "-l", 2) == 0)
						_pkgconfig_add_to_list(lst, walk);
				} else if (flags & PKG_LIBS_ONLY_L) {
					if (strncmp(walk, "-L", 2) == 0)
						_pkgconfig_add_to_list(lst, walk);
				} else if (flags & PKG_LIBS_ONLY_OTHERS) {
					if (strncmp(walk, "-L", 2) != 0 && strncmp(walk, "-l", 2) != 0)
						_pkgconfig_add_to_list(lst, walk);
				} else {
					_pkgconfig_add_to_list(lst, walk);
				}
			}
		}
		if (i != nbel) {
			walk += next + 1;
			next = strlen(walk);
		}
	}
	
	return ret;
}

static int _pkgconfig_parse_cflags(PkgConfig *pc, Pkg *p, char *data)
{
	if ((pc->flags & (PKG_CFLAGS|PKG_CFLAGS_ONLY_I|PKG_CFLAGS_ONLY_OTHERS)) == 0)
		return 0;

	_pkglist_new(&p->cflags);
	return _pkgconfig_parse_generic(p->cflags, data, pc->flags, PKG_CFLAGS);
}

static int _pkgconfig_parse_cflags_private(PkgConfig *pc, Pkg *p, char *data)
{
	if ((pc->flags & (PKG_CFLAGS|PKG_CFLAGS_ONLY_I|PKG_CFLAGS_ONLY_OTHERS)) == 0)
		return 0;

	_pkglist_new(&p->cflags_private);
	return _pkgconfig_parse_generic(p->cflags_private, data, pc->flags, PKG_CFLAGS);
}

static int _pkgconfig_parse_libs(PkgConfig *pc, Pkg *p, char *data)
{
	if ((pc->flags & (PKG_LIBS|PKG_LIBS_ONLY_L|PKG_LIBS_ONLY_l|PKG_LIBS_ONLY_OTHERS)) == 0)
		return 0;

	_pkglist_new(&p->libs);
	return _pkgconfig_parse_generic(p->libs, data, pc->flags, PKG_LIBS);
}

static int _pkgconfig_parse_libs_private(PkgConfig *pc, Pkg *p, char *data)
{
	if ((pc->flags & (PKG_LIBS|PKG_LIBS_ONLY_L|PKG_LIBS_ONLY_l|PKG_LIBS_ONLY_OTHERS)) == 0)
		return 0;

	_pkglist_new(&p->libs_private);
	return _pkgconfig_parse_generic(p->libs_private, data, pc->flags, PKG_LIBS);
}

static int _pkgconfig_parse_requires_private(PkgConfig *pc ,Pkg *p, char *data)
{
	if (pc->flags & PKG_MODVERSION)
		return 0;

	_pkglist_new(&p->requires_private);
	return _pkgconfig_parse_requires_generic(pc, p, data, p->requires_private);
}

static int _pkgconfig_parse_requires(PkgConfig *pc, Pkg *p, char *data)
{
	if (pc->flags & PKG_MODVERSION)
		return 0;

	_pkglist_new(&p->requires);
	return _pkgconfig_parse_requires_generic(pc, p, data, p->requires);
}

static void * _pkglist_lookup(PkgList *list, void *data, int (*compar)(const char *, const char *))
{
	size_t i;

	for (i = 0; i < list->len; i++)
		if (compar(_pkglist_get(list, i), data) == 0)
			return _pkglist_get(list, i);

	return NULL;
}

static void printout(PkgList *list, char * toprint)
{
	if (_pkglist_lookup(list, toprint, strcmp) != NULL)
		return;

	printf("%s ", toprint);
	_pkglist_append(list, toprint, NULL);
}

static int _pkgconfig(PkgConfig * pc, int pkgc, char * pkgv[])
{
	int ret = 0;
	PkgList *printed = NULL;
	char out = '\0';

	char const format[] = "Package %s was not found in the pkg-config"
		" search path.\n"
		"Perhaps you should add the directory containing `%s.pc'\n"
		"to the PKG_CONFIG_PATH environment variable\n"
		"No package '%s' found\n";
	char const * libdir, * libpath;
	int i;
	size_t j, k;
	FILE * fp;
	Pkg *p;

	/* default values */
	_pkglist_append(pc->pc_dirs, "/usr/lib/pkgconfig", NULL);
	_pkglist_append(pc->pc_dirs, "/usr/libdata/pkgconfig", NULL);
	_pkglist_append(pc->pc_dirs, PREFIX"/libdata/pkgconfig", NULL);
	_pkglist_append(pc->pc_dirs, PREFIX"/lib/pkgconfig", NULL);
	_pkglist_append(pc->pc_dirs, PREFIX"/share/pkgconfig", NULL);

	/* environment variables */
	libdir = getenv("PKG_CONFIG_LIBDIR");
	libpath = getenv("PKG_CONFIG_PATH");
	_pkglist_new(&printed);

	/* TODO parse libpath and libdir */
	if (libpath != NULL) {
		_pkglist_delete(pc->pc_dirs);
	} else if (libdir != NULL) {
		_pkglist_delete(pc->pc_dirs);
	}
	/* packages */
	for(i = 0; i < pkgc; i++)
	{
		/* Do not try to load already loaded pkgs */
		for (j = 0; j < pc->pkgs->len; j++)
			if (strcmp(((Pkg *) _pkglist_get(pc->pkgs, j))->pkgname, pkgv[i]) == 0)
				break;

		if (j < pc->pkgs->len)
			continue;

		if ((fp = _pkgconfig_open(pc, pkgv[i])) != NULL)
		{
			if ((pc->flags & PKG_EXISTS) == 0)
				ret |= _pkgconfig_parse(pc, fp);
			fclose(fp);
		} else {
			if ((pc->flags & PKG_EXISTS) == 0)
				return _pkgconfig_error(1, format, pkgv[i], pkgv[i], pkgv[i]);
			else
				return 1;
		}
	}
	if (pc->flags & PKG_MODVERSION) {
		for (j = 0; j < pc->pkgs->len; j++) {
			p = (Pkg*)_pkglist_get(pc->pkgs, j);
			printf("%s\n",p->version);
		}
		_pkglist_delete(printed);
		return 0;
	}
	if (pc->flags & (PKG_PRINT_REQUIRES|PKG_PRINT_REQUIRES_PRIV)) {
		for (j = 0; j < pc->pkgs->len; j++) {
			p = (Pkg *)_pkglist_get(pc->pkgs, j);
			if (pc->flags & PKG_PRINT_REQUIRES && p->requires != NULL)
				for (k = 0; k < p->requires->len; k++)
					printf("%s\n", ((PkgRequires *)_pkglist_get(p->requires, k))->name);
			if (pc->flags & PKG_PRINT_REQUIRES_PRIV && p->requires_private != NULL)
				for (k = 0; k < p->requires_private->len; k++)
					printf("%s\n", ((PkgRequires *)_pkglist_get(p->requires_private, k))->name);
		}
		_pkglist_delete(printed);
		return 0;
	}
	if (pc->flags & (PKG_CFLAGS|PKG_CFLAGS_ONLY_OTHERS|PKG_CFLAGS_ONLY_I)) {
		for (j = 0; j < pc->pkgs->len; j++) {
			p = (Pkg*)_pkglist_get(pc->pkgs, j);
			if (p->cflags == NULL)
				continue;
			for (k = 0; k < p->cflags->len; k++) {
				printout(printed, (char *)_pkglist_get(p->cflags, k));
				out='\n';
			}
		}
	}
	if (pc->flags & (PKG_LIBS|PKG_LIBS_ONLY_L|PKG_LIBS_ONLY_l|PKG_LIBS_ONLY_OTHERS)) {
		for (j = 0; j < pc->pkgs->len; j++) {
			p = (Pkg *)_pkglist_get(pc->pkgs, j);
			if (p->libs == NULL)
				continue;
			for (k = 0; k < p->libs->len; k++) {
				printout(printed, (char *)_pkglist_get(p->libs, k));
				out='\n';
			}
			if (pc->flags & PKG_STATIC && p->libs_private != NULL) {
				for (k = 0; k < p->libs_private->len; k++) {
					printout(printed, (char *)_pkglist_get(p->libs_private, k));
					out='\n';
				}
			}
		}
	}
	printf("%c", out);
	_pkglist_delete(printed);
	return ret;
}

/* see http://fedoraproject.org/wiki/Tools/RPM/VersionComparison */ 
static int version_cmp(char const *v1, char const *v2)
{
	const char *ver1, *ver2;
	int v1t = 0;
	int v2t = 0;
	int ret = 0;
	int c;

	if (strcmp(v1, v2) == 0)
		return 0;

	/* start as the first alpha numeric value */
	while (*v1 != '\0' && *v2 != '\0') {
		v1t = 0;
		v2t = 0;
		while(*v1 != '\0' && !isalnum((c = *v1)))
			v1++;
		while(*v2 != '\0' && !isalnum((c = *v2)))
			v2++;

		if (v1 == '\0' && v2 == '\0')
			return 0;

		if (v1 == '\0')
			return -1;

		if (v2 == '\0')
			return 1;

		ver1 = v1;
		ver2 = v2;

		if (isdigit((c = *ver1))) {
			v1t = 1;
			while(*v1 != '\0' && isdigit((c = *v1)))
				v1++;
		} else {
			v1t = 2;
			while(*v1 != '\0' && isalpha((c = *v1)))
				v1++;
		}

		if (isdigit((c = *ver2))) {
			v2t = 1;
			while(*v2 != '\0' && isdigit((c = *v2)))
				v2++;
		} else {
			v2t = 2;
			while(*v2 != '\0' && isalpha((c = *v2)))
				v2++;

		}
		/* v1 is alpha and v2 is num then v2 GT v1) */
		if (v1t < v2t)
			return -1;

		/* v1 is num and v2 is alpha then v1 GT v2) */
		if (v2t > v1t)
			return 1;

		if (v1t == 1) {
			ret = strtol(ver1, NULL, 10) - strtol(ver2, NULL, 10);
			if (ret != 0)
				return ret;
		} else {
			while (*ver1 != *v1 && *ver2 != *v2) {
				if (*ver1 == *ver2) {
					ver1++;
					ver2++;
					continue;
				}
				break;
			}
			if (*ver1 == *v1 && *ver2 != *v2)
				return -1;
			if (*ver1 != *v1 && *ver2 == *v2)
				return 1;
			if (*ver1 > *ver2)
				return 1;
			if (*ver1 < *ver2)
				return -1;
		}
		v1++;
		v2++;
	}
	return 0;
}

static FILE * _pkgconfig_open(PkgConfig *pc, char const * pkg)
{
	FILE * fp;
	size_t i;
	char path[MAXPATHLEN];
	Pkg *p = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, pkg);
#endif
	for (i = 0; i < pc->pc_dirs->len; i++)
	{
		snprintf(path, MAXPATHLEN, "%s/%s.pc", (char *)_pkglist_get(pc->pc_dirs, i), pkg);
		if ((fp = fopen(path, "r")) != NULL) {
			p = _pkg_new(&p, pkg);
			_pkglist_append(pc->pkgs, p, _pkg_delete);
			return fp;
		}
	}
	return NULL;
}

static int _pkgconfig_parse(PkgConfig * pc, FILE * fp)
{
	Pkg *p;
	char * line;
	size_t len = 256;
	size_t i;
	int c;

	p = _pkglist_get(pc->pkgs, pc->pkgs->len - 1);

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
			if (line[i] == '\0')
				continue;
			if(_pkgconfig_parse_variable(pc, p, line, &line[i]) != 0)
				return -1;
		}
		else if(line[i] == ':')
		{
			line[i] = '\0';
			for(i += 1; (c = line[i]) != '\0' && isspace(c); i++);
			if (line[i] == '\0')
				continue;
			if(_pkgconfig_parse_directive(pc, p, line, &line[i]) != 0)
				return -1;
		}
#ifdef DEBUG
		else
			fprintf(stderr, "DEBUG: %s\n", line);
#endif
	}
	free(line);
	return 0;
}

static int _pkgconfig_parse_directive(PkgConfig * pc, Pkg *pkg, char const * directive,
		char const * value)
{
	int ret = 0;
	char * p;
	int i = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, directive,
			value);
#endif
	if((p = _pkgconfig_parse_substitute(pkg->variables, value)) == NULL)
		return -1;

	for (i = 0; keys[i].key != NULL; i++) {
		if (strcmp(directive, keys[i].key) == 0) {
			keys[i].parse(pc, pkg, p);
		}
	}
	free(p);
	return ret;
}

static int _pkgconfig_parse_requires_generic(PkgConfig *pc, Pkg * pkg, char * requires, PkgList *lst)
{
	int ret = 0;
	int i;
	size_t j;
	char * p;
	Pkg *reqp;
	FILE * fp;
	int expect_version = 0;
	operator op;
	int nbel;
	char *walk;
	size_t next;
	PkgRequires *req = NULL;

	if (pc->flags & PKG_MODVERSION)
		return 0;

	if (pc->flags & (PKG_LIBS|PKG_LIBS_ONLY_L|PKG_LIBS_ONLY_l|PKG_LIBS_ONLY_L) && (pc->flags & PKG_STATIC) == 0)
		return 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, requires);
#endif

	if((p = _pkgconfig_parse_substitute(pkg->variables, requires)) == NULL)
		return -1;

	nbel = split_chr(p, ' ');
	nbel += split_chr(p, '\t');
	nbel += split_chr(p, ',');

	next = strlen(p);
	walk = p;
	for (i = 0; i <= nbel; i++) {
		if (next == 0) {
			walk += next + 1;
			next = strlen(walk);
			continue;
		}

		if (walk[0] == '>') {
			if (req == NULL)
				return -_pkgconfig_error(1, "malformed entry");
			op = GT;
			if (walk[0] == '=')
				op = GE;
			expect_version = 1;
		} else if (walk[0] == '<') {
			if (req == NULL)
				return -_pkgconfig_error(1, "malformed entry");
			op = LT;
			if (walk[0] == '=')
				op = LE;
			expect_version = 1;
		} else if (walk[0] == '=') {
			if (req == NULL)
				return -_pkgconfig_error(1, "malformed entry");
			op = EQ;
			expect_version = 1;
		} else {
			if (!expect_version) {
				req = NULL;
				_pkgrequires_new(&req, walk);
				for (j = 0; j < pc->pkgs->len; j++) {
					reqp = (Pkg*)_pkglist_get(pc->pkgs, j);
					if (strcmp(reqp->pkgname, walk) == 0) {
						req->pkg = reqp;
						break;
					}
				}
				_pkglist_append(lst, req, _pkgrequires_delete);
				if (req->pkg == NULL) {
					if ((fp = _pkgconfig_open(pc, req->name)) != NULL) {
						if ((pc->flags & (PKG_PRINT_REQUIRES|PKG_PRINT_REQUIRES_PRIV)) == 0) {
							ret |= _pkgconfig_parse(pc, fp);
							reqp = (Pkg *)_pkglist_get(pc->pkgs, pc->pkgs->len -1);
							req->pkg = reqp;
						}
						fclose(fp);
					} else {
						ret |= _pkgconfig_error(1, "%s: %s", req->name, strerror(errno));
					}
				}
			} else {
				req->version = strdup(walk);
				expect_version = 0;
			}
		}
		walk += next + 1;
		next = strlen(walk);
	}
	_string_delete(p);
	return ret;
}

static char * _pkgconfig_parse_substitute(PkgList * vars, char const * value)
{
	char * ret = NULL;
	PkgConfigVariable *var = NULL;
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
		for(k = 0; k < vars->len; k++) {
			var = (PkgConfigVariable *)_pkglist_get(vars, k);
			if(strncmp(var->name, &value[i + 2],
						j - i - 2) == 0)
				break;
			var = NULL;
		}
		if(var == NULL)
		{
			/* FIXME report error */
			free(ret);
			return NULL;
		}
		if(_string_append(&ret, var->value) != 0)
		{
			/* FIXME report error */
			free(ret);
			return NULL;
		}
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s => %s\n", var->name, var->value);
#endif
		i = j;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => \"%s\"\n", __func__, ret);
#endif
	return ret;
}

static int _pkgconfig_parse_variable(__unused PkgConfig * pc, Pkg *pkg, char const * name,
		char const * value)
{
	PkgConfigVariable * p = NULL;
	char * q;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, name, value);
#endif

	if((q = _pkgconfig_parse_substitute(pkg->variables, value)) == NULL)
		return -1;

	_pkglist_new(&pkg->variables);

	/* check if the variable already exists */
	for(i = 0; i < pkg->variables->len; i++)
	{
		p = (PkgConfigVariable *)_pkglist_get(pkg->variables, i);
		if(strcmp(p->name, name) == 0)
		{
			free(p->value);
			p->value = q;
			return 0;
		}
	}

	/* allocate a new variable */
	p = NULL;
	p = _pkgconfig_variable_new(&p);

	if((p->name = strdup(name)) == NULL)
	{
		_pkgconfig_variable_delete(p);
		return -_pkgconfig_error(1, "%s", strerror(errno));
	}
	p->value = q;
	/* TODO cleanup function */
	_pkglist_append(pkg->variables, p, NULL);
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

/* lists */
static PkgList * _pkglist_new(PkgList ** pkglist)
{
	if (*pkglist != NULL)
		return *pkglist;

	*pkglist = malloc(sizeof(PkgList));
	(*pkglist)->len = 0;
	(*pkglist)->cap = 0;
	(*pkglist)->data = NULL;

	return *pkglist;
}

static void * _pkglist_get(PkgList *pkglist, size_t i)
{
	if (i >= pkglist->len)
		return NULL;

	return pkglist->data[i]->data;
}

static void _pkglist_delete(PkgList *pkglist)
{
	size_t i;

	if (pkglist == NULL)
		return;

	for (i = 0; i < pkglist->len; i++) {
		if (pkglist->data[i]->freefn != NULL)
			pkglist->data[i]->freefn(_pkglist_get(pkglist,i));
	}

	free(pkglist);
}

static void  _pkglist_append(PkgList *pkglist, void *data, void (*freefn)(void *))
{
	ListData *ldata;
	if (pkglist->cap <= pkglist->len) {
		pkglist->cap |= 1;
		pkglist->cap *= 2;
		if ((pkglist->data = realloc(pkglist->data, pkglist->cap * sizeof (ListData))) == NULL) {
			_pkgconfig_error(1, "%s", strerror(errno));
			return;
		}
	}
	if ((ldata = malloc(sizeof(ListData))) == NULL)
	{
		_pkgconfig_error(1, "%s", strerror(errno));
		return;
	}

	ldata->data = data;
	ldata->freefn = freefn;
	pkglist->data[pkglist->len++] = ldata;
}


/* usage */
static int _usage(int brief)
{
	if(brief)
		fputs("Usage: pkg-config [-?] [--version] [--modversion]\n",
				stderr);
	else
		fputs("Usage: pkg-config [OPTIONS...] [PACKAGES...]\n"
"  --cflags			Output all pre-processor and compiler flags\n"
"  --cflags-only-I		Output -I flags\n"
"  --cflags-only-other		Output non -I flags\n"
"  --libs			Output all linker flags\n"
"  --libs_only_L		Ouput -L flags\n"
"  --libs_only_l		Ouput -l flags\n"
"  --libs_only_other		other libs (e.g. -pthread)\n"
"  --modversion			Output version for package\n"
"  --print-requires		Output the requires packages\n"
"  --print-requires_private	Output the requires private packages\n"
"  --static			Output linker flags for static linking\n"
"  --version			Output version of pkg-config\n"
"\n"
"Help options:\n"
"  -?, --help		Show this help message\n"
"  --usage		Display brief usage message\n", stderr);
	return 1;
}


/* main */
static int _main_option(PkgConfig * pc, char const * option);
static int _main_option_cflags(PkgConfig * pc);
static int _main_option_cflags_only_I(PkgConfig * pc);
static int _main_option_cflags_only_other(PkgConfig * pc);
static int _main_option_exists(PkgConfig * pc);
static int _main_option_libs(PkgConfig * pc);
static int _main_option_libs_only_l(PkgConfig * pc);
static int _main_option_libs_only_L(PkgConfig * pc);
static int _main_option_libs_only_other(PkgConfig * pc);
static int _main_option_static(PkgConfig * pc);
static int _main_option_usage(PkgConfig * pc);
static int _main_option_version(PkgConfig * pc);
static int _main_option_modversion(PkgConfig * pc);
static int _main_option_print_requires(PkgConfig * pc);
static int _main_option_print_requires_private(PkgConfig * pc);
static struct
{
	char const * option;
	int (*callback)(PkgConfig * pc);
} _main_options[] = {
	{ "cflags",			_main_option_cflags			},
	{ "cflags-only-I",		_main_option_cflags_only_I		},
	{ "cflags-only-other",		_main_option_cflags_only_other		},
	{ "exists",			_main_option_exists			},
	{ "libs",			_main_option_libs			},
	{ "libs-only-l",		_main_option_libs_only_l		},
	{ "libs-only-L",		_main_option_libs_only_L		},
	{ "libs-only-other",		_main_option_libs_only_other		},
	{ "modversion",			_main_option_modversion			},
	{ "print-requires",		_main_option_print_requires		},
	{ "print-requires-private",	_main_option_print_requires_private	},
	{ "static",			_main_option_static			},
	{ "usage",			_main_option_usage			},
	{ "version",			_main_option_version			}
};

int main(int argc, char * argv[])
{
	PkgConfig pc;
	int optind;

	memset(&pc, 0, sizeof(pc));
	_pkglist_new(&pc.pkgs);
	_pkglist_new(&pc.pc_dirs);
	pc.flags = '\0';
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
		if(_main_option(&pc, argv[optind]) != 0)
			return 1;
	}

	/* check if any package was specified */
	if(optind == argc)
	{
		fputs("Must specify package names on the command line\n",
				stderr);
		return 1;
	}
	return _pkgconfig(&pc, argc - optind, &argv[optind]);
}

static int _main_option(PkgConfig * pc, char const * option)
{
	size_t i;

	for(i = 0; i < sizeof(_main_options) / sizeof(*_main_options); i++)
		if(strcmp(_main_options[i].option, &option[2]) == 0)
			return _main_options[i].callback(pc);
	fprintf(stderr, "%s: Unknown option\n", option);
	return 1;
}

static int _main_option_cflags(PkgConfig * pc)
{
	pc->flags |= PKG_CFLAGS;
	return 0;
}

static int _main_option_cflags_only_I(PkgConfig * pc)
{
	pc->flags |= PKG_CFLAGS_ONLY_I;
	return 0;
}

static int _main_option_cflags_only_other(PkgConfig * pc)
{
	pc->flags |= PKG_CFLAGS_ONLY_OTHERS;
	return 0;
}

static int _main_option_exists(PkgConfig * pc)
{
	pc->flags |= PKG_EXISTS;
	return 0;
}

static int _main_option_libs(PkgConfig * pc)
{
	pc->flags |= PKG_LIBS;
	return 0;
}

static int _main_option_libs_only_L(PkgConfig * pc)
{
	pc->flags |= PKG_LIBS_ONLY_L;
	return 0;
}

static int _main_option_libs_only_l(PkgConfig * pc)
{
	pc->flags |= PKG_LIBS_ONLY_l;
	return 0;
}

static int _main_option_libs_only_other(PkgConfig * pc)
{
	pc->flags |= PKG_LIBS_ONLY_OTHERS;
	return 0;
}

static int _main_option_modversion(PkgConfig * pc)
{
	pc->flags |= PKG_MODVERSION;
	return 0;
}

static int _main_option_print_requires(PkgConfig * pc)
{
	pc->flags |= PKG_PRINT_REQUIRES;
	return 0;
}

static int _main_option_print_requires_private(PkgConfig * pc)
{
	pc->flags |= PKG_PRINT_REQUIRES_PRIV;
	return 0;
}

static int _main_option_static(PkgConfig * pc)
{
	pc->flags |= PKG_STATIC;
	return 0;
}

static int _main_option_usage(__unused PkgConfig * pc)
{
	return _usage(1);
}

static int _main_option_version(__unused PkgConfig * pc)
{
	puts("0.26");
	exit(0);
}
