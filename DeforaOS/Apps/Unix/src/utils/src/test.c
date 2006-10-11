/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>


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
/* b */ static int _is_file_block(char * pathname);
/* c */ static int _is_file_char(char * pathname);
/* d */ static int _is_file_dir(char * pathname);
/* e */ static int _is_file(char * pathname);
/* f */ static int _is_file_regular(char * pathname);
/* g */ static int _is_file_sgid(char * pathname);
/* h */ static int _is_file_symlink(char * pathname);
/* n */ static int _is_string_not_empty(char * string);
/* r */ static int _is_file_readable(char * pathname);
/* S */ static int _is_file_socket(char * pathname);
/* s */ static int _is_file_size(char * pathname);
/* w */ static int _is_file_writable(char * pathname);
/* x */ static int _is_file_executable(char * pathname);
/* z */ static int _is_string_empty(char * string);
static int _test_single(char c, char * argv)
{
	switch(c)
	{
		case 'b':
			return _is_file_block(argv);
		case 'c':
			return _is_file_char(argv);
		case 'd':
			return _is_file_dir(argv);
		case 'e':
			return _is_file(argv);
		case 'f':
			return _is_file_regular(argv);
		case 'g':
			return _is_file_sgid(argv);
		case 'h':
		case 'L':
			return _is_file_symlink(argv);
		case 'n':
			return _is_string_not_empty(argv);
		case 'r':
			return _is_file_readable(argv);
		case 'S':
			return _is_file_socket(argv);
		case 's':
			return _is_file_size(argv);
		case 'w':
			return _is_file_writable(argv);
		case 'x':
			return _is_file_executable(argv);
		case 'z':
			return _is_string_empty(argv);
	}
	return 0;
}

static int _is_file_block(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISBLK(st.st_mode) ? 1 : 0;
}

static int _is_file_char(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return S_ISCHR(st.st_mode) ? 1 : 0;
}

static int _is_file_dir(char * pathname)
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

	if(lstat(pathname, &st) != 0)
		return 0;
	return S_ISLNK(st.st_mode) ? 1 : 0;
}

static int _is_string_not_empty(char * string)
{
	return *string == '\0' ? 0 : 1;
}

static int _file_rwx_g(gid_t g);
static int _is_file_rwx(char * pathname, mode_t u, mode_t g, mode_t o)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	if(geteuid() == st.st_uid)
		if((st.st_mode & u) != 0)
			return 1;
	if((st.st_mode & g) != 0)
	{
		if(_file_rwx_g(st.st_gid) == 1)
			return 1;
		if(geteuid() == st.st_uid)
			return 0;
	}
	return (st.st_mode & o) != 0 ? 1 : 0;
}

static int _file_rwx_g(gid_t gid)
{
	struct passwd * passwd;
	struct group * group;
	char ** p;

	if((passwd = getpwuid(geteuid())) == NULL)
		return 0;
	if((group = getgrgid(gid)) == NULL)
		return 0;
	for(p = group->gr_mem; *p != NULL; p++)
		if(strcmp(passwd->pw_name, *p) == 0)
			return 1;
	return 0;
}

static int _is_file_readable(char * pathname)
{
	return _is_file_rwx(pathname, S_IRUSR, S_IRGRP, S_IROTH);
}

static int _is_file_socket(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return (st.st_mode & S_IFSOCK) ? 1 : 0; /* FIXME */
}

static int _is_file_size(char * pathname)
{
	struct stat st;

	if(stat(pathname, &st) != 0)
		return 0;
	return st.st_size > 0 ? 1 : 0;
}

static int _is_string_empty(char * string)
{
	return *string == '\0' ? 1 : 0;
}

static int _is_file_writable(char * pathname)
{
	return _is_file_rwx(pathname, S_IWUSR, S_IWGRP, S_IWOTH);
}

static int _is_file_executable(char * pathname)
{
	return _is_file_rwx(pathname, S_IXUSR, S_IXGRP, S_IXOTH);
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
