/* chmod.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#define OPT_R 1


/* chmod */
static int _chmod_error(char * message, int ret);
static int _chmod_do(mode_t mode, char * file);
static int _chmod_do_recursive(mode_t mode, char * file);
static int _chmod(int opts, mode_t mode, int filec, char * filev[])
{
	int i;
	int res = 0;

	for(i = 0; i < filec; i++)
		if(opts & OPT_R)
			res += _chmod_do_recursive(mode, filev[i]);
		else
			res += _chmod_do(mode, filev[i]);
	return res == 0 ? 0 : 2;
}

static int _chmod_error(char * message, int ret)
{
	fprintf(stderr, "%s", "chmod: ");
	perror(message);
	return ret;
}

static int _chmod_do(mode_t mode, char * file)
{
	if(chmod(file, mode) != 0)
		return _chmod_error(file, 1);
	return 0;
}

static int _chmod_do_recursive(mode_t mode, char * file)
{
	return 1;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: chmod [-R] mode file\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int opts = 0;
	mode_t mode;
	int o;

	while((o = getopt(argc, argv, "R")) != -1)
	{
		switch(o)
		{
			case 'R':
				mode = OPT_R;
				break;
			default:
				return _usage();
		}
	}
	if(argc - optind < 2)
		return _usage();
	/* FIXME */
	mode = strtol(argv[optind], NULL, 8);
	return _chmod(opts, mode, argc - optind - 1, &argv[optind+1]);
}
