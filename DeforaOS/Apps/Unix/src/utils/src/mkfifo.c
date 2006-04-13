/* mkfifo.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define COMMON_MODE
#include "common.c"


/* mkfifo */
static int _mkfifo_error(char * message, int ret);
static int _mkfifo(mode_t mode, int argc, char * argv[])
{
	int ret = 0;
	int i;

	for(i = 0; i < argc; i++)
		if(mkfifo(argv[i], mode) == -1)
			ret+=_mkfifo_error(argv[i], 1);
	return ret;
}

static int _mkfifo_error(char * message, int ret)
{
	fprintf(stderr, "%s", "mkfifo: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: mkfifo [-m mode] file...\n\
  -m	create fifo with the specified mode value\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int o;

	while((o = getopt(argc, argv, "m:")) != -1)
		switch(o)
		{
			case 'm':
				if(_mode(optarg, &mode) != 0)
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc == optind)
		return _usage();
	return _mkfifo(mode, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
