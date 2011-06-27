/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "mailer.h"
#include "compose.h"
#include "../config.h"
#define _(string) gettext(string)


/* private */
/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* functions */
/* compose */
static Compose * _compose(Config * config, int toc, char * tov[])
{
	Compose * compose;
	int i;

	if((compose = compose_new(config)) == NULL)
		return NULL;
	compose_set_standalone(compose, TRUE);
	for(i = 0; i < toc; i++)
		compose_add_field(compose, "To:", tov[i]);
	return compose;
}


/* compose_config */
static Config * _compose_config(void)
{
	Config * config;
	char const * homedir;
	String * filename;

	if((config = config_new()) == NULL)
		return NULL;
	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	if((filename = string_new_append(homedir, "/" MAILER_CONFIG_FILE))
			!= NULL)
	{
		config_load(config, filename);
		string_delete(filename);
	}
	return config;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: compose address...\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Config * config;
	Compose * compose;
	int o;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	config = _compose_config();
	if((compose = _compose(config, argc - optind, &argv[optind])) != NULL)
	{
		gtk_main();
		compose_delete(compose);
	}
	if(config != NULL)
		config_delete(config);
	return 0;
}
