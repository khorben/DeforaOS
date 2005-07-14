/* nice.c */



#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* nice */
static int _nice_error(char * message, int ret);
static int _nice(int nice, char * argv[])
{
	if(setpriority(PRIO_PROCESS, 0, nice) != 0)
		return _nice_error("Unable to set priority", 2);
	execvp(argv[0], argv);
	return _nice_error(argv[0], 2);
}

static int _nice_error(char * message, int ret)
{
	fprintf(stderr, "%s", "nice: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: nice [-n increment] utility [argument...]\n\
  -n	priority to set\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int nice = 0;
	int o;
	char * p;

	while((o = getopt(argc, argv, "n:")) != -1)
		switch(o)
		{
			case 'n':
				nice = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	return _nice(nice, &argv[optind]);
}
