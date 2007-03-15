/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_H 0x00
#define PREFS_L 0x01
#define PREFS_P 0x02
#define PREFS_f 0x04
#define PREFS_i 0x08
#define PREFS_p 0x10
#define PREFS_r 0x20
#define PREFS_R PREFS_r


/* cp */
static int _cp_error(char const * message, int ret);
static int _cp_single(Prefs * prefs, char const * src, char const * dst);
static int _cp_multiple(Prefs * prefs, int filec, char * const filev[]);
static int _cp(Prefs * prefs, int filec, char * filev[])
{
	/* FIXME
	 * - cp_multiple already checks if last arg is a dir
	 *   => always call cp_multiple
	 *      if argc == 2 && returns 1, call cp_single
	 * - blah blah */
	struct stat st;

	if(stat(filev[filec - 1], &st) == -1)
	{
		if(errno != ENOENT)
			return _cp_error(filev[filec - 1], 1);
		if(filec > 2)
		{
			fprintf(stderr, "%s%s%s", "cp: ", filev[filec - 1],
					": Does not exist and more than two"
					" operands were given\n");
			return 1;
		}
	}
	else if(S_ISDIR(st.st_mode))
		return _cp_multiple(prefs, filec, filev);
	return _cp_single(prefs, filev[0], filev[1]);
}

static int _cp_error(char const * message, int ret)
{
	fputs("cp: ", stderr);
	perror(message);
	return ret;
}

static int _cp_single(Prefs * prefs, char const * src, char const * dst)
{
	int ret = 0;
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;
	int fd;
	struct stat st;

	if((fsrc = fopen(src, "r")) == NULL)
		return _cp_error(src, 1);
	if((fd = fileno(fsrc)) == -1 || fstat(fd, &st) != 0)
	{
		ret = _cp_error(src, 1);
		fclose(fsrc);
		return ret;
	}
	if(S_ISDIR(st.st_mode))
	{
		fprintf(stderr, "%s%s%s", "cp: ", src,
				": Omitting directory\n");
		return 1;
	}
	if((fdst = fopen(dst, "w")) == NULL)
	{
		ret = _cp_error(dst, 1);
		fclose(fsrc);
		return ret;
	}
	while((size = fread(buf, sizeof(char), BUFSIZ, fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != size)
		{
			ret = _cp_error(dst, 1);
			fclose(fsrc);
			fclose(fdst);
			return ret;
		}
	if(!feof(fsrc))
		ret = _cp_error(src, 1);
	if(fclose(fsrc) != 0)
		ret = _cp_error(src, 1);
	if(fclose(fdst) != 0)
		return _cp_error(dst, 1);
	return ret;
}

static int _cp_multiple(Prefs * prefs, int filec, char * const filev[])
{
	int ret = 0;
	char * dst = NULL;
	char * p;
	int i;
	int len;

	for(i = 0; i < filec - 1; i++)
	{
		len = strlen(filev[i]) + strlen(filev[filec - 1]) + 2;
		if((p = realloc(dst, len * sizeof(char))) == NULL)
		{
			_cp_error(filev[filec - 1], 0);
			continue;
		}
		dst = p;
		sprintf(dst, "%s/%s", filev[filec - 1], filev[i]);
		ret |= _cp_single(prefs, filev[i], dst);
	}
	free(dst);
	return ret;
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
	while((o = getopt(argc, argv, "HLPfipRr")) != -1)
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
				prefs -= prefs & PREFS_P;
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs -= prefs & PREFS_H;
				prefs -= prefs & PREFS_P;
				prefs |= PREFS_L;
				break;
			case 'P':
				prefs -= prefs & PREFS_H;
				prefs -= prefs & PREFS_L;
				prefs |= PREFS_P;
				break;
			case 'p':
				prefs |= PREFS_p;
				break;
			case 'r':
			case 'R':
				prefs -= prefs & PREFS_r;
				break;
			default:
				return _usage();
		}
	if(optind + 1 >= argc)
		return _usage();
	return _cp(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
