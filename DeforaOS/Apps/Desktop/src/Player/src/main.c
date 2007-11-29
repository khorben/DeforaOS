/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
/* Player is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Player is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Player; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "player.h"


/* variables */
Player * player;


/* usage */
static int _usage(void)
{
	fputs("Usage: Player [file...]\n", stderr);
	return 1;
}


/* main */
static void _main_signal(void);
int main(int argc, char * argv[])
{
	int o;
	int i;

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
	for(i = optind+1; i < argc; i++)
		player_queue_add(player, argv[i]);
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
		fputs("Player: SIGCHLD: Not handled\n", stderr);
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
	fprintf(stderr, "%s", "Player: ");
	if(WIFEXITED(status))
		fprintf(stderr, "%s%d%s%u\n", "child ", pid,
				": exited with code ", WEXITSTATUS(status));
	else
		fprintf(stderr, "%d%s", pid, ": Unknown state\n");
}
