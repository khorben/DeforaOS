/* mkfifo.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern char * optarg;
extern int optind;
#include <stdio.h>
#include <stdlib.h>


/* mkfifo */
static int _mkfifo(char const * pathname, mode_t mode)
{
	if(mkfifo(pathname, mode) == -1)
	{
		perror("mkfifo");
		return 2;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: mkfifo [-m mode] file...\n\
  -m    create fifo with the specified mode value\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int errcode = 0;
	int o;
	int i;

	while((o = getopt(argc, argv, "m:")) != -1)
	{
		switch(o)
		{
			case 'm':
				/* FIXME */
				if((mode = strtol(optarg, NULL, 8)) > 0777)
					return _usage();
				break;
			case '?':
				return _usage();
		}
	}
	if(argc == optind)
		return _usage();
	for(i = optind; i < argc; i++)
		if(_mkfifo(argv[i], mode) == 2)
			errcode = 2;
	return errcode;
}
