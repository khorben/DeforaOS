/* cp.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define sys_error(msg) { fprintf(stderr, "%s", "cp: "); perror(msg); return 2; }


/* cp */
static int _cp_single(char * src, char * dst);
static int _cp_multiple(int argc, char * argv[]);
static int _cp(int argc, char * argv[])
{
	/* FIXME
	 * - cp_multiple already checks if last arg is a dir
	 *   => always call cp_multiple
	 *      if argc == 2 && returns 1, call cp_single
	 * - blah blah */
	struct stat st;

	if(argc > 2)
		return _cp_multiple(argc, argv);
	if(stat(argv[1], &st) == -1 && errno != ENOENT)
		sys_error(argv[1]);
	if(S_ISDIR(st.st_mode))
		return _cp_multiple(argc, argv);
	return _cp_single(argv[0], argv[1]);
}

static int _cp_single(char * src, char * dst)
{
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;
	char * err = src;
	struct stat st;

	if(stat(src, &st) == -1)
		sys_error(src);
	if(S_ISDIR(st.st_mode))
	{
		fprintf(stderr, "%s%s%s", "cp: ", src, ": omitting directory\n");
		return 2;
	}
	if((fsrc = fopen(src, "r")) == NULL)
		sys_error(src);
	if((fdst = fopen(dst, "w")) == NULL)
	{
		fclose(fsrc);
		sys_error(dst);
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
		sys_error(err);
	}
	fclose(fsrc);
	return 0;
}

static int _cp_multiple(int argc, char * argv[])
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
			sys_error("malloc");
		}
		dst = p;
		sprintf(dst, "%s/%s", argv[argc-1], argv[i]);
		_cp_single(argv[i], dst);
	}
	free(dst);
	return 2;
}

/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: cp [-fip] source_file target_file\n\
       cp [-fip] source_file ... target\n\
       cp -R [-H | -L | -P][-fip] source_file ... target\n\
       cp -r [-H | -L | -P][-fip] source_file ... target\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
	{
		switch(o)
		{
			case '?':
				return _usage();
		}
	}
	if(optind + 1 >= argc)
		return _usage();
	return _cp(argc - optind, &argv[optind]);
}
