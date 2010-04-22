/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
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



#include <System.h>
#include <stdio.h>
#include <string.h>
#include "configure.h"
#include "settings.h"


/* types */
typedef enum _SETTINGS_TYPE
{
	ST_H = 0,
	ST_SH
} SettingsType;
#define ST_LAST ST_SH
String * sSettingsType[ST_LAST+1] =
{
	"h", "sh"
};


/* functions */
/* settings */
static int _settings_do(Prefs * prefs, Config * config,
		String const * directory, String const * package,
		String const * version, String const * extension);

int settings(Prefs * prefs, Config * config, String const * directory,
		String const * package, String const * version)
{
	int ret = 0;
	String const * p;
	String * q;
	String * r;
	unsigned long i;
	char c;

	if((p = config_get(config, "", "config")) == NULL)
		return 0;
	if((q = string_new(p)) == NULL)
		return 1;
	r = q;
	for(i = 0;; i++)
	{
		if(r[i] != ',' && r[i] != '\0')
			continue;
		c = r[i];
		r[i] = '\0';
		ret |= _settings_do(prefs, config, directory, package, version,
				r);
		if(c == '\0')
			break;
		r+=i+1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _do_h(Prefs * prefs, Config * config, FILE * fp,
		String const * package, String const * version);
static int _do_sh(Prefs * prefs, Config * config, FILE * fp,
		String const * package, String const * version);
static int _settings_do(Prefs * prefs, Config * config,
		String const * directory, String const * package,
		String const * version, String const * extension)
{
	int ret = 0;
	int i;
	String * filename;
	FILE * fp;

	for(i = 0; i <= ST_LAST; i++)
		if(strcmp(extension, sSettingsType[i]) == 0)
			break;
	if(i > ST_LAST)
	{
		fprintf(stderr, "%s%s%s", "configure: ", extension,
				": Unknown settings type\n");
		return 1;
	}
	if(prefs->flags & PREFS_n)
		return 0;
	if((filename = string_new(directory)) == NULL)
		return 1;
	if(string_append(&filename, "/config.") != 0
			|| string_append(&filename, extension) != 0)
	{
		string_delete(filename);
		return 1;
	}
	if((fp = fopen(filename, "w")) == NULL)
		configure_error(filename, 0);
	string_delete(filename);
	if(fp == NULL)
		return 1;
	if(prefs->flags & PREFS_v)
		printf("%s%s/%s%s\n", "Creating ", directory, "config.",
				extension);
	switch(i)
	{
		case ST_H:
			ret |= _do_h(prefs, config, fp, package, version);
			break;
		case ST_SH:
			ret |= _do_sh(prefs, config, fp, package, version);
			break;
	}
	fclose(fp);
	return ret;
}

static int _do_h(Prefs * prefs, Config * config, FILE * fp,
		String const * package, String const * version)
{
	char const * p;

	fprintf(fp, "%s%s%s%s%s%s", "#define PACKAGE \"", package, "\"\n",
			"#define VERSION \"", version, "\"\n");
	if((p = prefs->prefix) != NULL
			|| (p = config_get(config, "", "prefix")) != NULL)
		fprintf(fp, "%s%s%s", "\n#ifndef PREFIX\n\
# define PREFIX \"", p, "\"\n#endif\n");
	if((p = prefs->libdir) != NULL
			|| (p = config_get(config, "", "libdir")) != NULL)
	{
		fprintf(fp, "%s", "\n#ifndef LIBDIR\n# define LIBDIR ");
		fprintf(fp, "%s%s", p[0] == '/' ? "\"" : "PREFIX \"/", p);
		fprintf(fp, "%s", "\"\n#endif\n");
	}
	return 0;
}

static int _do_sh(Prefs * prefs, Config * config, FILE * fp,
		String const * package, String const * version)
{
	char const * p;

	fprintf(fp, "%s%s%s%s%s%s", "PACKAGE=\"", package, "\"\n",
			"VERSION=\"", version, "\"\n");
	if((p = prefs->prefix) != NULL
			|| (p = config_get(config, "", "prefix")) != NULL)
		fprintf(fp, "%s%s%s", "\nPREFIX=\"", p, "\"\n");
	if((p = prefs->libdir) != NULL
			|| (p = config_get(config, "", "libdir")) != NULL)
		fprintf(fp, "%s%s%s%s", "LIBDIR=\"", p[0] == '/' ? ""
				: "${PREFIX}/", p, "\"\n");
	return 0;
}
