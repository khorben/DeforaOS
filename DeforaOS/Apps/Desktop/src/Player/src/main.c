/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
	fprintf(stderr, "%s", "Usage: Player [file...]\n");
	return 1;
}


/* main */
static void _main_signals(void);
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
	_main_signals();
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

static void _signals_handler(int signum);
static void _main_signals(void)
	/* handle mplayer death; should be done in Player as a callback but
	 * would potentially conflict with other Player instances */
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _signals_handler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		fprintf(stderr, "%s", "Player: SIGCHLD: Not handled\n");
}

static void _signals_handler(int signum)
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
