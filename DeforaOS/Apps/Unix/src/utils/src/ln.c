/* ln.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <stdio.h>
#include <errno.h>


/* ln */
int ln(int flgf, int flgs, int argc, char * argv[])
{
	struct stat buf;
	int i;

	if(argc <= 1)
		return 1;
	if(stat(argv[argc-1], &buf) == -1)
	{
		if(errno != ENOENT)
		{
			perror("stat");
			return 2;
		}
		if(argc > 2)
			return 1;
		if(flgs)
		{
			if(symlink(argv[0], argv[1]) == -1)
			{
				perror("symlink");
				return 3;
			}
		}
		else
		{
			if(link(argv[0], argv[1]) == -1)
			{
				perror("link");
				return 3;
			}
		}
		return 0;
	}
	else
	{
		if(S_ISDIR(buf.st_mode))
		{
			char * newpath;

			for(i = 0; i < argc - 1; i++)
			{
				/* FIXME */
			}
			return 0; /* FIXME could be errors */
		}
		if(argc > 2)
			return 1;
	}
	return 0;
}


/* usage */
static int usage(void)
{
	fprintf(stderr, "Usage: ln [-fs] source_file target_file\n\
        ln [-fs] source_file ... target_dir\n\
  -f    force existing destination pathnames to be removed\n\
  -s    create symbolic links instead of hard links\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flgf = 0;
	int flgs = 0;
	int o;
	int res;

	while((o = getopt(argc, argv, "fs")) != -1)
	{
		switch(o)
		{
			case 'f':
				flgf = 1;
				break;
			case 's':
				flgs = 1;
				break;
			case '?':
				return usage();
		}
	}
	if((res = ln(flgf, flgs, argc - optind, &argv[optind])) == 1)
		return usage();
	return res;
}
