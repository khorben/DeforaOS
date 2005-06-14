/* rm.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2
#define PREFS_R 0x4


/* rm */
static int _rm_error(char * message, int ret);
static int _rm_do(Prefs * prefs, char * file);
static int _rm(Prefs * prefs, int argc, char * argv[])
{
	int i;
	int ret = 0;

	for(i = 0; i < argc; i++)
		ret+=_rm_do(prefs, argv[i]);
	return ret ? 2 : 0;
}

static int _rm_error(char * message, int ret)
{
	fprintf(stderr, "%s", "rm: ");
	perror(message);
	return ret;
}

static int _rm_confirm(char const * message, char const * type)
{
	int c;
	int tmp;

	fprintf(stderr, "%s%s%s%s%s", "rm: ", message, ": Remove ", type, "? ");
	if((c = fgetc(stdin)) == EOF)
		return _rm_error("stdin", 0);
	while(c != '\n' && (tmp = fgetc(stdin)) != EOF && tmp != '\n');
	return c == 'y';
}

static int _rm_do_recursive(Prefs * prefs, char * file);
static int _rm_do(Prefs * prefs, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0)
	{
		if(!(*prefs & PREFS_f))
			return _rm_error(file, 1);
		return 0;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(!(*prefs & PREFS_R))
		{
			fprintf(stderr, "%s%s%s", "rm: ", file,
					": Is a directory\n");
			return 0;
		}
		return _rm_do_recursive(prefs, file);
	}
	/* FIXME ask also if permissions do not allow file removal */
	if(*prefs & PREFS_i && !_rm_confirm(file, "file"))
		return 0;
	if(unlink(file) != 0)
		return _rm_error(file, 2);
	return 0;
}

static int _rm_do_recursive(Prefs * prefs, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len = strlen(file) + 2;
	char * path;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _rm_error(file, 2);
	readdir(dir);
	readdir(dir);
	if((path = malloc(len)) == NULL)
	{
		closedir(dir);
		return _rm_error("malloc", 2);
	}
	sprintf(path, "%s/", file);
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(path, len + strlen(de->d_name))) == NULL)
		{
			free(path);
			closedir(dir);
			return _rm_error("malloc", 2);
		}
		path = p;
		strcpy(&path[len-1], de->d_name);
		if(_rm_do(prefs, path) != 0)
			return 2;
	}
	free(path);
	closedir(dir);
	if(*prefs & PREFS_i && !_rm_confirm(file, "directory"))
		return 0;
	if(rmdir(file) != 0)
		return _rm_error(file, 2);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: rm [-fiRr] file...\n\
  -f    Do not prompt for confirmation or output error messages\n\
  -i    Prompt for confirmation\n\
  -R    Remove file hierarchies\n\
  -r    Equivalent to -R\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "fiRr")) != -1)
	{
		switch(o)
		{
			case 'f':
				prefs = prefs - (prefs & PREFS_i);
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs = prefs - (prefs & PREFS_f);
				prefs |= PREFS_i;
				break;
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			case '?':
				return _usage();
		}
	}
	if(optind == argc)
		return _usage();
	if(!(prefs & PREFS_f) && isatty(0))
		prefs |= PREFS_i;
	return _rm(&prefs, argc-optind, &argv[optind]);
}
