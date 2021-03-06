/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
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
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include "player.h"
#include "../config.h"
#define _(string) gettext(string)

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


/* public */
/* variables */
Player * player;


/* private */
/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: player [file...]\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
static void _main_signal(void);

int main(int argc, char * argv[])
{
	int o;
	int i;

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
	_main_signal();
	if((player = player_new()) == NULL)
		return 2;
	if(optind < argc)
		player_open(player, argv[optind]);
	for(i = optind + 1; i < argc; i++)
		player_playlist_add(player, argv[i]);
	gtk_main();
	player_delete(player);
	return 0;
}

static void _signal_handler(int signum);

static void _main_signal(void)
	/* handle mplayer death; should be done in Player as a callback but
	 * would potentially conflict with other Player instances */
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _signal_handler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		fputs("player: SIGCHLD: Not handled\n", stderr);
}

static void _signal_handler(int signum)
{
	pid_t pid;
	int status;

	if(signum != SIGCHLD)
		return;
	if(player_sigchld(player) == 0)
		return;
	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		player_error(NULL, "waitpid", 0);
		return;
	}
	if(pid == 0)
		return;
	fputs("player: ", stderr);
	if(WIFEXITED(status))
		fprintf(stderr, "%s%d%s%u\n", "child ", pid,
				": exited with code ", WEXITSTATUS(status));
	else
		fprintf(stderr, "%d%s", pid, ": Unknown state\n");
}
