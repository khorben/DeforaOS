/* chmod.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define OPT_R 1


/* chmod */
static int _chmod_error(char * message, int ret);
static int _chmod_do(mode_t mode, char * file);
static int _chmod_do_recursive(int opts, mode_t mode, char * file);
static int _chmod(int opts, mode_t mode, int filec, char * filev[])
{
	int i;
	int res = 0;

	for(i = 0; i < filec; i++)
		if(opts & OPT_R)
			res += _chmod_do_recursive(opts, mode, filev[i]);
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

static int _chmod_do_recursive_do(int opts, mode_t mode, char * file);
static int _chmod_do_recursive(int opts, mode_t mode, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0)
		return _chmod_error(file, 1);
	if(!S_ISDIR(st.st_mode))
		return _chmod_do(mode, file);
	if(!S_ISLNK(st.st_mode))
		return _chmod_do_recursive_do(opts, mode, file);
	return 0;
}

static int _chmod_do_recursive_do(int opts, mode_t mode, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _chmod_error(file, 1);
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len-1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		closedir(dir);
		return _chmod_error(file, 1);
	}
	strcpy(s, file);
	s[len-2] = '/';
	s[len-1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			_chmod_error("malloc", 0);
			continue;
		}
		s = p;
		strcat(s, de->d_name);
		_chmod_do_recursive(opts, mode, s);
		s[len-1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
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
