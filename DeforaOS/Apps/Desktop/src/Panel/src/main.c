/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <gtk/gtk.h>
#include "common.h"
#include "../config.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: " PACKAGE " [-m monitor]\n", stderr);
	return 1;
}


/* main */
static void _main_sigchld(int signum);

int main(int argc, char * argv[])
{
	int o;
	Panel * panel;
	PanelPrefs prefs;
	char * p;
	struct sigaction sa;

	gtk_init(&argc, &argv);
	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "m:")) != -1)
		switch(o)
		{
			case 'm':
				prefs.monitor = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((panel = panel_new(&prefs)) == NULL)
		return 2;
	sa.sa_handler = _main_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		panel_error(panel, "signal handling error", 0);
	gtk_main();
	panel_delete(panel);
	return 0;
}

static void _main_sigchld(int signum)
{
	wait(NULL);
}
