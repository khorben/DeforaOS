/* rmdir.c */



#include <unistd.h>
#include <stdio.h>


/* rmdir */
static int _rmdir_p(char * pathname);
static int _rmdir(int flagp, int argc, char * argv[])
{
	int res = 0;
	int i;

	for(i = 0; i < argc; i++)
	{
		if(rmdir(argv[i]) == -1)
		{
			fprintf(stderr, "%s", "rmdir: ");
			perror(argv[i]);
			res = 2;
			continue;
		}
		if(flagp)
		{
			if(_rmdir_p(argv[i]) == -1)
				res = 2;
		}
	}
	return res;
}

static int _rmdir_p(char * pathname)
{
	char * str;

	str = pathname;
	while(*str++);
	while(--str != pathname)
	{
		if(*str != '/')
			continue;
		*str = '\0';
		if(rmdir(pathname) == -1)
		{
			fprintf(stderr, "%s", "rmdir: ");
			perror(pathname);
			return -1;
		}
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: rmdir [-p] dir...\n\
  -p    remove all directories in a pathname\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flagp = 0;
	int o;

	while((o = getopt(argc, argv, "p")) != -1)
	{
		switch(o)
		{
			case 'p':
				flagp = 1;
				break;
			case '?':
				return _usage();
		}
	}
	if(optind == argc)
		return _usage();
	return _rmdir(flagp, argc - optind, &argv[optind]);
}
