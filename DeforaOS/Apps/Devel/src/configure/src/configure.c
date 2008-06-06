/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
/* configure is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * configure is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with configure; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "makefile.h"
#include "configure.h"
#include "../config.h"

#ifndef PACKAGE
# define PACKAGE	"configure"
#endif


/* configure */
/* variables */
const String * sHostArch[HA_COUNT] =
{
	"i386", "i486", "i586", "i686",
	"amd64",
	"sparc", "sparc64",
	"sun4u",
	"zaurus",
	"unknown"
};
const String * sHostOS[HO_COUNT] =
{
	"Linux",
	"FreeBSD", "NetBSD", "OpenBSD",
	"SunOS",
	"unknown"
};
const struct HostKernel sHostKernel[] =
{
	{ HO_GNU_LINUX,	"2.0"		},
	{ HO_GNU_LINUX,	"2.2"		},
	{ HO_GNU_LINUX,	"2.4"		},
	{ HO_GNU_LINUX,	"2.6"		},
	{ HO_NETBSD,	"2.0"		},
	{ HO_NETBSD,	"3.0"		},
	{ HO_NETBSD,	"4.0"		},
	{ HO_OPENBSD,	"4.0"		},
	{ HO_OPENBSD,	"4.1"		},
	{ HO_SUNOS,	"5.7",		},
	{ HO_SUNOS,	"5.8",		},
	{ HO_SUNOS,	"5.9",		},
	{ HO_SUNOS,	"5.10",		},
	{ HO_UNKNOWN,	"unknown"	}
};

const String * sTargetType[TT_COUNT] = { "binary", "library", "object", NULL };
const String * sObjectType[OT_COUNT] = { "c", "cc", "cpp", "S", NULL };

String const * _source_extension(String const * source)
{
	size_t len;

	for(len = string_length(source); len > 0; len--)
		if(source[len - 1] == '.')
			return &source[len];
	return NULL;
}


/* functions */
int configure_error(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


int enum_string(int last, const String * strings[], String const * str)
{
	int i;

	for(i = 0; i < last; i++)
		if(string_compare(strings[i], str) == 0)
			return i;
	return last;
}


int enum_string_short(int last, const String * strings[], String const * str)
{
	int i;

	for(i = 0; i < last; i++)
		if(string_compare_length(strings[i], str,
					string_length(strings[i])) == 0)
			return i;
	return last;
}


/* Configure */
static void _configure_detect(Configure * configure);
static int _configure_load(Prefs * prefs, char const * directory,
		configArray * ca);
static int _configure_do(Configure * configure, configArray * ca);
static int _configure(Prefs * prefs, char const * directory)
{
	int ret;
	Configure cfgr;
	configArray * ca;
	int flags = prefs->flags;
	int i;
	Config * p;

	if((ca = configarray_new()) == NULL)
		return error_print(PACKAGE);
	cfgr.prefs = prefs;
	_configure_detect(&cfgr);
	ret = _configure_load(prefs, directory, ca);
	if(ret == 0)
	{
		if(prefs->flags & PREFS_n)
			ret = _configure_do(&cfgr, ca);
		else
		{
			prefs->flags = PREFS_n;
			if(_configure_do(&cfgr, ca) == 0)
			{
				prefs->flags = flags;
				ret = _configure_do(&cfgr, ca);
			}
		}
	}
	for(i = array_count(ca); i > 0; i--)
	{
		array_get_copy(ca, i - 1, &p);
		config_delete(p);
	}
	array_delete(ca);
	return ret;
}


/* private */
static HostKernel _detect_kernel(HostOS os, char const * release);
static void _configure_detect(Configure * configure)
{
	struct utsname un;

	if(uname(&un) < 0)
	{
		configure_error("system detection failed", 0);
		configure->arch = HA_UNKNOWN;
		configure->os = HO_UNKNOWN;
		configure->kernel = HK_UNKNOWN;
		return;
	}
	configure->arch = enum_string(HA_LAST, sHostArch, un.machine);
	configure->os = enum_string(HO_LAST, sHostOS, un.sysname);
	configure->kernel = _detect_kernel(configure->os, un.release);
	if(configure->prefs->flags & PREFS_v)
		printf("Detected system %s version %s on %s\n",
				sHostOS[configure->os],
				sHostKernel[configure->kernel].version,
				sHostArch[configure->arch]);
}

static HostKernel _detect_kernel(HostOS os, char const * release)
{
	unsigned int i;

	for(i = 0; i != HK_LAST; i++)
	{
		if(sHostKernel[i].os < os)
			continue;
		if(sHostKernel[i].os > os)
			return HK_UNKNOWN;
		if(strncmp(release, sHostKernel[i].version,
					strlen(sHostKernel[i].version)) == 0)
			return i;
	}
	return i;
}


static int _load_subdirs(Prefs * prefs, char const * directory,
		configArray * ca, String const * subdirs);
static int _configure_load(Prefs * prefs, String const * directory,
		configArray * ca)
{
	int ret = 0;
	Config * config;
	String * path;
	String const * subdirs = NULL;

	if((path = string_new(directory)) == NULL)
		return configure_error(directory, 1);
	if(string_append(&path, "/") != 0
			|| string_append(&path, PROJECT_CONF) != 0
			|| (config = config_new()) == NULL)
	{
		string_delete(path);
		return configure_error(directory, 1);
	}
	config_set(config, "", "directory", directory);
	if(prefs->flags & PREFS_v)
		printf("%s%s%s", "Loading project file ", path, "\n");
	if(config_load(config, path) != 0)
		ret = error_print(PACKAGE);
	else
	{
		array_append(ca, &config);
		subdirs = config_get(config, "", "subdirs");
	}
	string_delete(path);
	if(subdirs == NULL)
		return ret;
	return _load_subdirs(prefs, directory, ca, subdirs);
}

static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
		configArray * ca, char const * subdir);
static int _load_subdirs(Prefs * prefs, char const * directory,
		configArray * ca, String const * subdirs)
{
	int ret = 0;
	int i;
	char c;
	String * subdir;
	String * p;
	
	if((subdir = string_new(subdirs)) == NULL)
		return 1;
	p = subdir;
	for(i = 0; ret == 0; i++)
	{
		if(subdir[i] != ',' && subdir[i] != '\0')
			continue;
		c = subdir[i];
		subdir[i] = '\0';
		ret = _load_subdirs_subdir(prefs, directory, ca, subdir);
		if(c == '\0')
			break;
		subdir += i + 1;
		i = 0;
	}
	string_delete(p);
	return ret;
}


static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
		configArray * ca, char const * subdir)
	/* FIXME error checking */
{
	int ret;
	String * p;

	p = string_new(directory);
	string_append(&p, "/");
	string_append(&p, subdir);
	ret = _configure_load(prefs, p, ca);
	string_delete(p);
	return ret;
}


static int _configure_do(Configure * configure, configArray * ca)
{
	size_t i;
	size_t cnt = array_count(ca);
	String const * di;
	size_t j;
	Config * cj;
	String const * dj;

	for(i = 0; i < cnt; i++)
	{
		array_get_copy(ca, i, &configure->config);
		di = config_get(configure->config, "", "directory");
		for(j = i; j < cnt; j++)
		{
			array_get_copy(ca, j, &cj);
			dj = config_get(cj, "", "directory");
			if(string_find(dj, di) == NULL)
				break;
		}
		if(makefile(configure, di, ca, i, j) != 0)
			break;
	}
	return (i == cnt) ? 0 : 1;
}


/* usage */
static void _prefs_init(Prefs * prefs);
static int _usage(void)
{
	Prefs prefs;

	_prefs_init(&prefs);
	fprintf(stderr, "%s%s%s%s%s%s%s%s%s",
"Usage: configure [-nv][options...][directory]\n\
  -n	Do not actually write Makefiles\n\
  -v	Verbose mode\n\
  -b	Binary files directory (default: \"", prefs.bindir, "\")\n\
  -d	Destination prefix (default: \"\")\n\
  -i	Include files directory (default: \"", prefs.includedir, "\")\n\
  -l	Library files directory (default: \"", prefs.libdir, "\")\n\
  -p	Installation directory prefix (default: \"", prefs.prefix, "\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	_prefs_init(&prefs);
	while((o = getopt(argc, argv, "d:i:l:np:v")) != -1)
		switch(o)
		{
			case 'b':
				prefs.bindir = optarg;
				break;
			case 'd':
				prefs.destdir = optarg;
				break;
			case 'i':
				prefs.includedir = optarg;
				break;
			case 'l':
				prefs.libdir = optarg;
				break;
			case 'n':
				prefs.flags |= PREFS_n;
				break;
			case 'p':
				prefs.prefix = optarg;
				break;
			case 'v':
				prefs.flags |= PREFS_v;
				break;
			case '?':
				return _usage();
		}
	if(argc - optind > 1)
		return _usage();
	return _configure(&prefs, argc - optind == 1 ? argv[argc - 1] : ".");
}

static void _prefs_init(Prefs * prefs)
{
	struct stat st;

	memset(prefs, 0, sizeof(Prefs));
	prefs->destdir = "";
	if(stat("/usr", &st) == 0) /* FIXME see below */
	{
		prefs->bindir = "bin";
		prefs->includedir = "include";
		prefs->libdir = "lib";
		prefs->prefix = "/usr/local";
		return;
	}
	prefs->bindir = "Binaries";
	prefs->includedir = "Includes";
	prefs->libdir = "Libraries";
	prefs->prefix = "/Apps"; /* FIXME detect System or Apps/x first */
}
