/* test.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* test */
static int _test_single(char c, char * argv);
static int _test(int argc, char * argv[])
{
	int i;
	char * p;

	for(i = 0; i < argc; i++)
	{
		p = argv[i];
		if(strlen(p) == 2 && *p == '-')
		{
			if(i + 1 == argc)
				return 1;
			if(_test_single(*++p, argv[++i]) != 1)
				return 1;
			continue;
		}
		return 1;
	}
	return i == argc ? 0 : 1;
}

/* test_single */
static int _is_block(char * pathname);
static int _is_char(char * pathname);
static int _is_dir(char * pathname);
static int _is_file(char * pathname);
static int _is_file_regular(char * pathname);
static int _is_file_sgid(char * pathname);
static int _is_file_symlink(char * pathname);
static int _test_single(char c, char * argv)
{
	switch(c)
	{
		case 'b':
			return _is_block(argv);
		case 'c':
			return _is_char(argv);
		case 'd':
			return _is_dir(argv);
		case 'e':
			return _is_file(argv);
		case 'f':
			return _is_file_regular(argv);
		case 'g':
			return _is_file_sgid(argv);
		case 'h':
		case 'L':
			return _is_file_symlink(argv);
	}
	return 0;
}

static int _is_block(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISBLK(st.st_mode) ? 1 : 0;
}

static int _is_char(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISCHR(st.st_mode) ? 1 : 0;
}

static int _is_dir(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISDIR(st.st_mode) ? 1 : 0;
}

static int _is_file(char * pathname)
{
	struct stat st;

	return stat(pathname, &st) == 0 ? 1 : 0;
}

static int _is_file_regular(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISREG(st.st_mode) ? 1 : 0;
}

static int _is_file_sgid(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return (st.st_mode & S_ISGID) ? 1 : 0; /* FIXME */
}

static int _is_file_symlink(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISLNK(st.st_mode) ? 1 : 0; /* FIXME */
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: test [expression]\n\
       [ [expression] ]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc == 1)
		return 1; /* FIXME _usage(); */
	if(strcmp(argv[0], "[") == 0)
		if(strcmp(argv[--argc], "]") != 0)
			return _usage();
	return _test(argc - 1, argv + 1);
}
