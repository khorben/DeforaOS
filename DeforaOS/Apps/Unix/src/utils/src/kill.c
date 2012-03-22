/* $Id$ */
/* Copyright (c) 2009-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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



#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* kill */
/* private */
/* constants */
struct
{
	int signal;
	char const * name;
}
_kill_names[] =
{
#define signal(a) { a, "" # a }
#ifdef SIGABRT
	signal(SIGABRT),
#endif
#ifdef SIGALRM
	signal(SIGALRM),
#endif
#ifdef SIGBUS
	signal(SIGBUS),
#endif
#ifdef SIGCHLD
	signal(SIGCHLD),
#endif
#ifdef SIGCONT
	signal(SIGCONT),
#endif
#ifdef SIGFPE
	signal(SIGFPE),
#endif
#ifdef SIGHUP
	signal(SIGHUP),
#endif
#ifdef SIGKILL
	signal(SIGKILL),
#endif
#ifdef SIGILL
	signal(SIGILL),
#endif
#ifdef SIGINT
	signal(SIGINT),
#endif
#ifdef SIGQUIT
	signal(SIGQUIT),
#endif
#ifdef SIGSEGV
	signal(SIGSEGV),
#endif
#ifdef SIGSTOP
	signal(SIGSTOP),
#endif
#ifdef SIGTERM
	signal(SIGTERM),
#endif
#ifdef SIGTRAP
	signal(SIGTRAP),
#endif
#ifdef SIGUSR1
	signal(SIGUSR1),
#endif
#ifdef SIGUSR2
	signal(SIGUSR2),
#endif
	{ 0, NULL }
#undef signal
};


/* prototypes */
static int _kill(int sig, int argc, char * argv[]);
static int _kill_list(int argc, char * argv[]);


/* functions */
/* kill */
static int _kill(int sig, int argc, char * argv[])
{
	int ret = 0;
	int i;
	pid_t pid;
	char * p;

	for(i = 0; i < argc; i++)
	{
		pid = strtol(argv[i], &p, 10);
		if(argv[i][0] == '\0' || *p != '\0')
		{
			fprintf(stderr, "%s%s%s", "kill: ", argv[i],
					": Invalid process number\n");
			continue;
		}
		if(kill(pid, sig) != 0)
		{
			fputs("kill: ", stderr);
			perror("kill");
			ret |= 1;
		}
	}
	return ret;
}


/* kill_list */
static int _kill_list(int argc, char * argv[])
{
	size_t i;

	for(i = 0; _kill_names[i].name != NULL; i++)
		printf("%s\n", _kill_names[i].name);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: kill -s signal_name pid...\n\
       kill -l [exit_status]\n\
  -l	Write all signal values supported\n\
  -s	Specify the signal to send\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int sig = SIGTERM;
	int o;
	int list = 0;
	char * p;

	while((o = getopt(argc, argv, "ls:")) != -1)
	{
		switch(o)
		{
			case 'l':
				list = 1;
				break;
			case 's':
				/* FIXME signal_name expected, NaN... */
				sig = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	}
	if(list == 1)
	{
		if(argc - optind > 1)
			return _usage();
		return _kill_list(argc - optind, &argv[optind]);
	}
	if(optind == argc)
		return _usage();
	return (_kill(sig, argc - optind, &argv[optind]) == 0) ? 0 : 2;
}
