/* mkdir.c */



#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define COMMON_MODE
#include "common.c"


/* mkdir */
static int _mkdir_error(char * message, int ret);
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
		else if(mkdir(argv[i], mode) != 0)
			res = _mkdir_error(argv[i], 2);
	}
	return res;
}

static int _mkdir_error(char * message, int ret)
{
	fprintf(stderr, "%s", "mkdir: ");
	perror(message);
	return ret;
}

static int _mkdir_p(mode_t mode, char * pathname)
{
	char * p;
	struct stat st;

	for(p = pathname; *p != '\0'; p++)
	{
		if(*p != '/')
			continue;
		*p = '\0';
		if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
				&& mkdir(pathname, mode) == -1)
			return _mkdir_error(pathname, 2);
		for(*p++ = '/'; *p == '/'; p++);
		if(*p == '\0')
			return 0;
	}
	if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
			&& mkdir(pathname, mode) == -1)
		return _mkdir_error(pathname, 2);
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
		switch(o)
		{
			case 'm':
				if(_mode(optarg, &mode) != 0)
					return _usage();
				break;
			case 'p':
				flagp = 1;
				break;
			case '?':
				return _usage();
		}
	if(argc == optind)
		return _usage();
	return _mkdir(flagp, mode, argc - optind, &argv[optind]);
}
