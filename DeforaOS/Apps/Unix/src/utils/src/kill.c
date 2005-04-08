/* kill.c */



#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* kill */
static int _kill(int sig, int argc, char * argv[])
{
	int i;
	int res = 0;

	for(i = 0; i < argc; i++)
	{
		int pid;
		char * p;

		pid = strtol(argv[i], &p, 10);
		if(*(argv[i]) == '\0' || *p != '\0')
		{
			fprintf(stderr, "%s%s", argv[i],
					": Invalid process number\n");
			continue;
		}
		if(kill(pid, sig) == -1)
		{
			perror("kill");
			res = 2;
		}
	}
	return res;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: kill -s signal_name pid...\n\
       kill -l [exit_status]\n\
  -l    write all signal values supported\n\
  -s    specify the signal to send\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int sig = SIGTERM;
	int o;
	char * p;

	while((o = getopt(argc, argv, "ls:")) != -1)
	{
		switch(o)
		{
			case 'l':
				fprintf(stderr, "%s%c%s", "kill: -", o,
						": Not yet implemented\n");
				return _usage();
			case 's':
				/* FIXME signal_name expected, NaN... */
				sig = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			case '?':
				return _usage();
		}
	}
	if(optind == argc)
		return _usage();
	return _kill(sig, argc - optind, &argv[optind]);
}
