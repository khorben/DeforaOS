/* ln.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



/* types */
/* force link */
typedef enum _LinkForce {
	LF_NO,
	LF_YES
} LinkForce;
/* link type */
typedef enum _LinkType {
	LT_HARD = 0,
	LT_SOFT
} LinkType;


/* ln
 * PRE	lf	whether to unlink destination if already exists or not
 * 	lt	whether to link or symlink
 * 	argc	number of source + destination arguments
 * POST
 * 	0	success
 * 	2	error */
static int _ln_is_directory(char * dest);
static int _ln_single(LinkForce lf, LinkType lt, char * src, char * dest);
static int _ln_multiple(LinkForce lf, LinkType lt, int argc, char * argv[]);
static int _ln(LinkForce lf, LinkType lt, int argc, char * argv[])
{
	if(argc == 2 && !_ln_is_directory(argv[1]))
		return _ln_single(lf, lt, argv[0], argv[1]);
	return _ln_multiple(lf, lt, argc, argv);
}

static int _ln_is_directory(char * dest)
{
	struct stat buf;

	if(stat(dest, &buf) == -1 || !S_ISDIR(buf.st_mode))
		return 0;
	return 1;
}

static int _ln_single(LinkForce lf, LinkType lt, char * src, char * dest)
{
	if(lf == LF_YES)
		unlink(dest);
	if((lt == LT_HARD ? link(src, dest)
				: symlink(src, dest)) == -1)
	{
		perror(src);
		return 2;
	}
	return 0;
}

static int _ln_multiple(LinkForce lf, LinkType lt, int argc, char * argv[])
{
	int i;
	char * dest = argv[argc-1];
	char * p;

	for(i = 0; i < argc - 1; i++)
	{
		if((p = realloc(dest, strlen(dest) + strlen(argv[i]) + 2))
				== NULL)
		{
			perror("realloc");
			continue;
		}
		sprintf(dest, "%s/%s", argv[argc-1], argv[i]);
		_ln_single(lf, lt, argv[i], dest);
	}
	free(dest);
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
	LinkForce flgf = LF_NO;
	LinkType flgs = LT_HARD;
	int o;

	while((o = getopt(argc, argv, "fs")) != -1)
	{
		switch(o)
		{
			case 'f':
				flgf = LF_YES;
				break;
			case 's':
				flgs = LT_SOFT;
				break;
			case '?':
				return usage();
		}
	}
	if(argc - optind <= 1)
		return usage();
	return _ln(flgf, flgs, argc - optind, &argv[optind]);
}
