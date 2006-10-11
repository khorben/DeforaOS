/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* rmdir */
static int _rmdir_error(char * message, int ret);
static int _rmdir_p(char * pathname);
static int _rmdir(int flagp, int argc, char * argv[])
{
	int res = 0;
	int i;

	for(i = 0; i < argc; i++)
	{
		if(rmdir(argv[i]) != 0)
			res = _rmdir_error(argv[i], 2);
		if(flagp && _rmdir_p(argv[i]) != 0)
			res = 2;
	}
	return res;
}

static int _rmdir_error(char * message, int ret)
{
	fprintf(stderr, "%s", "rmdir: ");
	perror(message);
	return ret;
}

static int _rmdir_p(char * pathname)
{
	char * str;

	for(str = pathname; *str != '\0'; str++);
	for(str--; str > pathname && *str == '/'; str--);
	while(--str > pathname)
	{
		if(*str != '/')
			continue;
		for(*str = '\0'; --str > pathname && *str == '/'; *str = '\0');
		if(*str == '\0')
			return 0;
		if(rmdir(pathname) == -1)
			return _rmdir_error(pathname, 2);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: rmdir [-p] dir...\n\
  -p    remove all directories in a pathname\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flagp = 0;
	int o;

	while((o = getopt(argc, argv, "p")) != -1)
		switch(o)
		{
			case 'p':
				flagp = 1;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _rmdir(flagp, argc - optind, &argv[optind]);
}
