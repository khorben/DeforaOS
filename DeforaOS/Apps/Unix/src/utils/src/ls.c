/* ls.c */



#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <dirent.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef struct _Prefs {
	int a;
	int l;
} Prefs;


/* ls */
static int _ls_is_directory(char * filename);
static int _ls_directory(char * filename, Prefs * p);
static int _ls_file(char * filename, Prefs * p);
static int _ls(int argc, char * argv[], Prefs * p)
{
	int res = 0;
	int i;

	if(argc == 0)
		return _ls_directory(".", p) != 0 ? 2 : 0;
	for(i = 0; i < argc; i++)
	{
		if(i != 0)
			printf("\n");
		if(_ls_is_directory(argv[i]))
		{
			printf("%s:\n", argv[i]);
			if(_ls_directory(argv[i], p) != 0)
				res = 2;
			continue;
		}
		if(_ls_file(argv[i], p) != 0)
			res = 2;
	}
	return res;
}

static int _ls_is_directory(char * filename)
{
	struct stat buf;

	if(stat(filename, &buf) == -1 || !S_ISDIR(buf.st_mode))
		return 0;
	return 1;
}

static int _ls_directory(char * filename, Prefs * p)
{
	DIR * dir;
	struct dirent * dirent;

	if((dir = opendir(filename)) == NULL)
	{
		fprintf(stderr, "%s", "ls: ");
		perror(filename);
		return 1;
	}
	while((dirent = readdir(dir)) != NULL)
	{
		if(dirent->d_name[0] == '.' && p->a == 0)
			continue;
		printf("%s\n", dirent->d_name);
	}
	return 0;
}

static int _ls_file(char * filename, Prefs * p)
{
	struct stat buf;

	if(stat(filename, &buf) == -1)
	{
		fprintf(stderr, "%s", "ls: ");
		perror(filename);
		return 1;
	}
	printf("%s", filename);
	return 0;
}


/* prefs_parse */
static int _prefs_parse(int argc, char * argv[], Prefs * p)
{
	int o;

	memset(p, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "CFRacdilqrtu1HL")) != -1)
	{
		switch(o)
		{
			case 'a':
				p->a = 1;
				break;
			case 'l':
				p->l = 1;
				break;
			case '?':
				return 1;
			default:
				fprintf(stderr, "-%c%s",
						o, ": Not yet implemented\n");
				return 1;
		}
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: ls [-CFRacdilqrtu1][-H|-L][-fgmnopsx]\n\
  -a    write out all directory entries\n\
  -c    use time of last modification of the files status\n\
  -l    write out as long format\n\
  -L    evaluate referenced files for symlinks\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p;

	if(_prefs_parse(argc, argv, &p) == 1)
		return _usage();
	return _ls(argc - optind, &argv[optind], &p);
}
