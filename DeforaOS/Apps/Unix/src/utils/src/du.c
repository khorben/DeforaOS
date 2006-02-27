/* du.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Prefs */
typedef int Prefs;
#define PREFS_a 1
#define PREFS_s 2 
#define PREFS_x 4
#define PREFS_k 8 
#define PREFS_H 16
#define PREFS_L 48


/* du */
static int _du_do(Prefs * prefs, char const * filename);
static void _du_print(Prefs * prefs, off_t size, char const * filename);
static int _du(Prefs * prefs, int argc, char * argv[])
{
	int res = 0;
	int i;

	if(argc == 0)
		return _du_do(prefs, ".") == 0 ? 0 : 2;
	for(i = 0; i < argc; i++)
		res += _du_do(prefs, argv[i]);
	return res == 0 ? 0 : 2;
}

static int _du_error(char const * error)
{
	fprintf(stderr, "%s", "du: ");
	perror(error);
	return 1;
}

static off_t _du_blocks(Prefs * prefs, off_t size);
static int _du_do_recursive(Prefs * prefs, char const * filename);
static int _du_do(Prefs * prefs, char const * filename)
{
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;

	if(*prefs & PREFS_H)
		_stat = stat;
	if(_stat(filename, &st) != 0)
		return _du_error(filename);
	if(!S_ISDIR(st.st_mode))
	{
		_du_print(prefs, st.st_blocks, filename);
		return 0;
	}
	return _du_do_recursive(prefs, filename);
}

static off_t _du_blocks(Prefs * prefs, off_t size)
{
	if(*prefs & PREFS_k)
		return size / 2;
	return size;
}

static int _recursive_do(Prefs * prefs, off_t * size, char ** filename);
static int _du_do_recursive(Prefs * prefs, char const * filename)
{
	off_t size = 0;
	char * p;
	int len;
	int res;

	len = strlen(filename);
	if((p = malloc(len + 1)) == NULL)
		return _du_error("malloc");
	strcpy(p, filename);
	res = _recursive_do(prefs, &size, &p);
	printf("%ld %s\n", size, filename);
	free(p);
	return res;
}

static void _recursive_do_stat(Prefs * prefs, off_t * size, char ** filename);
static int _recursive_do(Prefs * prefs, off_t * size, char ** filename)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * p;

	if((dir = opendir(*filename)) == NULL)
		return _du_error(*filename);
	readdir(dir);
	readdir(dir);
	len = strlen(*filename);
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(*filename, len + strlen(de->d_name) + 2))
				== NULL)
		{
			_du_error("malloc");
			continue;
		}
		*filename = p;
		for(; len > 0 && p[len-1] == '/'; len--);
		p[len] = '/';
		strcpy(&p[len+1], de->d_name);
		_recursive_do_stat(prefs, size, filename);
	}
	return closedir(dir) == 0 ? 0 : 1;
}

static void _recursive_do_stat(Prefs * prefs, off_t * size, char ** filename)
{
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;
	char * p;
	off_t dirsize;

	if(*prefs & PREFS_L)
		_stat = stat;
	if(_stat(*filename, &st) != 0)
	{
		_du_error(*filename);
		return;
	}
	*size += _du_blocks(prefs, st.st_blocks);
	if(S_ISDIR(st.st_mode))
	{
		dirsize = _du_blocks(prefs, st.st_blocks) - *size;
		if((p = strdup(*filename)) == NULL)
		{
			_du_error("malloc");
			return;
		}
		_recursive_do(prefs, size, filename);
		if(!(*prefs & PREFS_s))
			printf("%ld %s\n", dirsize + *size, p);
		free(p);
	}
	else if(*prefs & PREFS_a)
		_du_print(prefs, st.st_blocks, *filename);
}

static void _du_print(Prefs * prefs, off_t size, char const * filename)
{
	printf("%ld %s\n", _du_blocks(prefs, size), filename);
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: du [-a | -s][-kx][-H | -L][file...]\n\
  -a    report the size of every file encountered\n\
  -s    report only the total sum for each of the specified files\n\
  -k    write the file sizes in units of 1024 bytes rather than 512\n\
  -x    evaluate file sizes only on the same device as the file specified\n\
  -H    dereference specified files\n\
  -L	dereference every file evaluated\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;

	memset(&prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "askxHL")) != -1)
		switch(o)
		{
			case 'a':
				prefs -= prefs & PREFS_s;
				prefs |= PREFS_a;
				break;
			case 's':
				prefs -= prefs & PREFS_a;
				prefs |= PREFS_s;
				break;
			case 'k':
				prefs |= PREFS_k;
				break;
			case 'x':
				prefs |= PREFS_x;
				break;
			case 'H':
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs |= PREFS_L;
				break;
			default:
				return _usage();
		}
	return _du(&prefs, argc - optind, &argv[optind]);
}
