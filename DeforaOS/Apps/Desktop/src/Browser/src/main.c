/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */
/* This file is part of Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "browser.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: browser [directory]\n");
	return 1;
}


/* main */
static void _main_sigchld(int signum);
int main(int argc, char * argv[])
{
	int o;
	Browser * browser;
	struct sigaction sa;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind < argc-1)
		return _usage();
	if((browser = browser_new(argv[optind])) == NULL)
	{
		gtk_main();
		return 2;
	}
	sa.sa_handler = _main_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		browser_error(browser, "signal handling error", 0);
	gtk_main();
	browser_delete(browser);
	return 0;
}

static void _main_sigchld(int signum)
{
	wait(NULL);
}
