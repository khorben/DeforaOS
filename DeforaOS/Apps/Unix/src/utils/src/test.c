/* test.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* test */
static int _is_block(char * pathname);
static int _is_char(char * pathname);
static int _is_dir(char * pathname);
static int _is_file(char * pathname);
static int _is_file_regular(char * pathname);
static int _test(int argc, char * argv[])
{
	int i;
	char * p;

	for(i = 0; i < argc; i++)
	{
		p = argv[i];
		if(strlen(p) == 2 && *p == '-')
		{
			switch(*++p)
			{
				case 'b':
					if(!_is_block(argv[++i]))
						return 1;
				case 'c':
					if(!_is_char(argv[++i]))
						return 1;
					break;
				case 'd':
					if(!_is_dir(argv[++i]))
						return 1;
					break;
				case 'e':
					if(!_is_file(argv[++i]))
						return 1;
					break;
				case 'f':
					if(!_is_file_regular(argv[++i]))
						return 1;
					break;
				default:
					return 1;
			}
			continue;
		}
		return 1;
	}
	return i == argc ? 0 : 1;
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
	return _test(argc, argv);
}
