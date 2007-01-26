/* cp.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x01
#define PREFS_i 0x02
#define PREFS_H 0x04
#define PREFS_L 0x08
#define PREFS_p 0x10
#define PREFS_P 0x20
#define PREFS_r 0x40
#define PREFS_R 0x80


/* cp */
static int _cp_error(char * message, int ret);
static int _cp_single(Prefs * prefs, char * src, char * dst);
static int _cp_multiple(Prefs * prefs, int argc, char * argv[]);
static int _cp(Prefs * prefs, int argc, char * argv[])
{
	/* FIXME
	 * - cp_multiple already checks if last arg is a dir
	 *   => always call cp_multiple
	 *      if argc == 2 && returns 1, call cp_single
	 * - blah blah */
	struct stat st;

	if(argc > 2)
		return _cp_multiple(prefs, argc, argv);
	if(stat(argv[1], &st) == -1 && errno != ENOENT)
		_cp_error(argv[1], 0);
	if(S_ISDIR(st.st_mode))
		return _cp_multiple(prefs, argc, argv);
	return _cp_single(prefs, argv[0], argv[1]);
}

static int _cp_error(char * message, int ret)
{
	fputs("cp: ", stderr);
	perror(message);
	return ret;
}

static int _cp_single(Prefs * prefs, char * src, char * dst)
{
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;
	char * err = src;
	int fd;
	struct stat st;

	if((fsrc = fopen(src, "r")) == NULL)
		return _cp_error(src, 1);
	if((fd = fileno(fsrc)) == -1 || fstat(fd, &st) != 0)
	{
		fclose(fsrc);
		return _cp_error(src, 0);
	}
	if(S_ISDIR(st.st_mode))
	{
		fprintf(stderr, "%s%s%s", "cp: ", src,
				": Omitting directory\n");
		return 2;
	}
	if((fdst = fopen(dst, "w")) == NULL)
	{
		fclose(fsrc);
		_cp_error(dst, 0);
	}
	while((size = fread(buf, sizeof(char), BUFSIZ, fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != BUFSIZ)
		{
			err = dst;
			break;
		}
	fclose(fdst);
	if(!feof(fsrc))
	{
		fclose(fsrc);
		_cp_error(err, 0);
	}
	fclose(fsrc);
	return 0;
}

static int _cp_multiple(Prefs * prefs, int argc, char * argv[])
{
	char * dst = NULL;
	char * p;
	int i, len;

	for(i = 0; i < argc - 1; i++)
	{
		len = strlen(argv[i]) + strlen(argv[argc-1]) + 2;
		if((p = realloc(dst, len * sizeof(char))) == NULL)
		{
			free(dst);
			_cp_error("malloc", 0);
		}
		dst = p;
		sprintf(dst, "%s/%s", argv[argc-1], argv[i]);
		_cp_single(prefs, argv[i], dst);
	}
	free(dst);
	return 2;
}

/* usage */
static int _usage(void)
{
	fputs("Usage: cp [-fip] source_file target_file\n\
       cp [-fip] source_file ... target\n\
       cp -R [-H | -L | -P][-fip] source_file ... target\n\
       cp -r [-H | -L | -P][-fip] source_file ... target\n\
  -f    attempt to remove destination file before a copy if necessary\n\
  -i    prompt before a copy to an existing file\n\
  -p    duplicate characteristics of the source files\n\
  -R    copy file hierarchies\n\
  -r    copy file hierarchies\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & PREFS_i;
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs -= prefs & PREFS_f;
				prefs |= PREFS_i;
				break;
			case 'H':
				prefs -= prefs & PREFS_L;
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs -= prefs & PREFS_H;
				prefs |= PREFS_L;
				break;
			case 'P':
				break;
			case 'p':
				break;
			case 'R':
				prefs -= prefs & PREFS_R;
				break;
			case 'r':
				prefs -= prefs & PREFS_r;
				break;
			default:
				return _usage();
		}
	if(optind + 1 >= argc)
		return _usage();
	return _cp(&prefs, argc - optind, &argv[optind]);
}
