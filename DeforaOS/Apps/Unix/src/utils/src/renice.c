/* renice */



#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* renice */
static int _renice_error(char * message, int ret);
static int _renice(int nice, int type, int argc, char * argv[])
{
	int i;
	int ret = 0;
	int id;
	char * p;

	for(i = 0; i < argc; i++)
	{
		id = strtol(argv[i], &p, 10);
		if(argv[i][0] == '\0' || *p != '\0')
		{
			fprintf(stderr, "%s%s%s", "renice: ", argv[i],
					"Invalid ID\n");
			ret = 2;
			continue;
		}
		if(setpriority(type, id, nice) != 0)
			ret = _renice_error(argv[i], 2);
	}
	return ret;
}

static int _renice_error(char * message, int ret)
{
	fprintf(stderr, "%s", "renice: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: renice -n increment [-g | -p | -u] ID...\n\
  -n\tpriority to set\n\
  -g\tprocess group IDs\n\
  -p\tinteger process IDs\n\
  -u\tuser IDs\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int nice = 0;
	int type = PRIO_PROCESS;
	int o;
	char * p;

	while((o = getopt(argc, argv, "n:gpu")) != -1)
	{
		switch(o)
		{
			case 'n':
				nice = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			case 'g':
				type = PRIO_PGRP;
				break;
			case 'p':
				type = PRIO_PROCESS;
				break;
			case 'u':
				type = PRIO_USER;
				break;
			default:
				return _usage();
		}
	}
	if(argc - optind < 1)
		return _usage();
	return _renice(nice, type, argc-optind, &argv[optind]);
}
