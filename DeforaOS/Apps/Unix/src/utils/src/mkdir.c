/* mkdir.c */



#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
extern int optind;
extern char * optarg;
#include <stdlib.h>
#include <stdio.h>


/* mkdir */
static int _mkdir_p(mode_t mode, char * pathname);
static int _mkdir(int flagp, mode_t mode, int argc, char * argv[])
{
	int res = 0;
	int i;

	for(i = 0; i < argc; i++)
	{
		if(flagp == 1)
		{
			if(_mkdir_p(mode, argv[i]) != 0)
				res = 2;
		}
		if(mkdir(argv[i], mode) == -1)
		{
			fprintf(stderr, "%s", "mkdir: ");
			perror(argv[i]);
			res = 2;
		}
	}
	return res;
}

static int _mkdir_p(mode_t mode, char * pathname)
{
	char * p;
	char c;

	for(p = pathname; *p != '\0'; p++)
	{
		if(*p != '/')
			continue;
		c = *p;
		*p = '\0';
		if(mkdir(pathname, mode) == -1)
		{
			fprintf(stderr, "%s", "mkdir: ");
			perror(pathname);
			return 2;
		}
		*p = c;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: mkdir [-p][-m mode] dir...\n\
  -p    create any missing intermediate pathname components\n\
  -m    file permission bits of the newly-created directory\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int flagp = 0;
	int o;

	while((o = getopt(argc, argv, "pm:")) != -1)
	{
		switch(o)
		{
			case 'm':
				/* FIXME */
				mode = strtol(optarg, NULL, 8);
				break;
			case 'p':
				flagp = 1;
				break;
			case '?':
				return _usage();
		}
	}
	if(argc == optind)
		return _usage();
	return _mkdir(flagp, mode, argc - optind, &argv[optind]);
}
