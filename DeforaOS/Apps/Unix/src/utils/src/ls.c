/* ls.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* FIXME */
#define max(a, b) ((a) > (b) ? (a) : (b))

/* Prefs */
typedef int Prefs;
#define PREFS_a 1

static int _usage(void);
static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

/*	memset(prefs, 0, sizeof(Prefs)); */
	*prefs = 0;
	while((o = getopt(argc, argv, "CFRa")) != -1)
	{
		switch(o)
		{
			case 'C':
			case 'F':
			case 'R':
				fprintf(stderr, "%s%c%s", "ls: -", o,
						": not yet implemented");
				return _usage();
			case 'a':
				*prefs &= PREFS_a;
				break;
			case '?':
				return _usage();
		}
	}
	return 0;
}


/* SList */
/* SListCell */
typedef struct _SListCell {
	void * data;
	struct _SListCell * next;
} SListCell;
typedef SListCell * SList;

static SListCell * _slistcell_new(void * data, SListCell * next)
{
	SListCell * slc;

	if((slc = malloc(sizeof(SListCell))) == NULL)
		return NULL;
	slc->data = data;
	slc->next = next;
	return slc;
}

static void _slistcell_delete(SListCell * slistcell)
{
	free(slistcell);
}

static SList * slist_new(void)
{
	SList * sl;

	if((sl = malloc(sizeof(SList))) == NULL)
		return NULL;
	*sl = NULL;
	return sl;
}

static void slist_delete(SList * slist)
{
	SListCell * slc = *slist;
	SListCell * p;

	while(slc != NULL)
	{
		p = slc->next;
		_slistcell_delete(slc);
		slc = p;
	}
	free(slist);
}

/* returns */
static void * slist_data(SList * slist)
{
	if(*slist == NULL)
		return NULL;
	return (*slist)->data;
}

static void slist_next(SList * slist)
{
	if(*slist == NULL)
		return;
	*slist = (*slist)->next;
}

static void slist_last(SList * slist)
{
	if(*slist == NULL)
		return;
	while((*slist)->next != NULL)
		*slist = (*slist)->next;
}

static int slist_append(SList * slist, void * data)
{
	slist_last(slist);
	if(*slist == NULL)
	{
		*slist = _slistcell_new(data, NULL);
		return *slist != NULL ? 0 : 2;
	}
	(*slist)->next = _slistcell_new(data, NULL);
	return (*slist)->next != NULL ? 0 : 2;
}

static void slist_apply(SList * slist, int (*func)(void *, void *), void * user)
{
	SListCell * slc = *slist;

	for(slc = *slist; slc != NULL; slc = slc->next)
		func(slc->data, user);
}

static int slist_insert_sorted(SList * slist, void * data,
		int (*func)(void *, void *))
{
	SListCell * slc = *slist;
	SListCell * p = NULL;

	while(slc != NULL && func(slc->data, data) < 0)
	{
		p = slc;
		slc = slc->next;
	}
	if(slc == NULL) /* empty or last */
	{
		if((slc = _slistcell_new(data, NULL)) == NULL)
			return 1;
		if(p == NULL)
			*slist = slc; /* empty */
		else
			p->next = slc; /* last */
		return 0;
	}
	if((p = _slistcell_new(slc->data, slc->next)) == NULL)
		return 1;
	slc->data = data;
	slc->next = p;
	return 0;
}

static size_t slist_length(SList * slist)
{
	SListCell * slc = *slist;
	size_t len;

	for(len = 0; slc != NULL; len++)
		slc = slc->next;
	return len;
}


/* ls */
static int _ls_do(SList * files, SList * dirs, Prefs * prefs);
static int _ls(int argc, char * argv[], Prefs * prefs)
{
	char * dircur = ".";
	int i;
	struct stat st;
	SList * files;
	SList * dirs;
	int res;

	if(argc == 0)
		return _ls(1, &dircur, prefs);
	if((files = slist_new()) == NULL)
		return 1;
	if((dirs = slist_new()) == NULL)
	{
		slist_delete(files);
		return 1;
	}
	for(i = 0; i < argc; i++)
	{
		if(stat(argv[i], &st) != 0)
		{
			fprintf(stderr, "%s", "ls: ");
			perror(argv[i]);
			continue;
		}
		if(S_ISDIR(st.st_mode) == 0)
			slist_insert_sorted(files, argv[i], strcmp);
		else
			slist_append(dirs, argv[i]);
	}
	res = _ls_do(files, dirs, prefs);
	slist_delete(files);
	slist_delete(dirs);
	return res;
}

static int _ls_files(SList * files, Prefs * prefs);
static int _ls_directories(SList * dirs, Prefs * prefs);
static int _ls_do(SList * files, SList * dirs, Prefs * prefs)
{
	_ls_files(files, prefs);
	if(slist_data(files) != NULL && slist_data(dirs) != NULL)
		printf("%s%s%s", "\n", (char*)slist_data(dirs), ":\n");
	else if(slist_length(dirs) > 1)
		printf("%s%s", (char*)slist_data(dirs), ":\n");
	_ls_directories(dirs, prefs);
	return 0;
}

static int _ls_files(SList * files, Prefs * prefs)
{
	char * cols;
	char * p;
	unsigned int len = 0;
	unsigned int lenmax = 0;
	unsigned int colnb = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	SList cur = *files;

#ifdef DEBUG
	fprintf(stderr, "%s", "_ls_files()\n");
#endif
	if((cols = getenv("COLUMNS")) != NULL
			&& *cols != '\0'
			&& (len = strtol(cols, &p, 10)) > 0
			&& *p == '\0')
	{
		while(cur != NULL)
		{
			lenmax = max(lenmax, strlen(slist_data(&cur)));
			slist_next(&cur);
		}
		if(lenmax > 0)
			colnb = len / ++lenmax;
	}
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		printf("%s", (char*)slist_data(&cur));
		if(++i < colnb)
		{
			for(j = strlen(slist_data(&cur)); j < lenmax; j++)
				fputc(' ', stdout);
			continue;
		}
		fputc('\n', stdout);
		i = 0;
	}
	if(i != 0)
		fputc('\n', stdout);
	return 0;
}

static int _ls_directory(char * filename, Prefs * prefs);
static int _ls_directories(SList * directories, Prefs * prefs)
{
	SList sl = *directories;

#ifdef DEBUG
	fprintf(stderr, "%s", "_ls_directories()\n");
#endif
	if(sl == NULL)
		return 0;
	_ls_directory((char*)slist_data(&sl), prefs);
	slist_next(&sl);
	while(sl != NULL)
	{
		printf("%s%s%s", "\n", (char*)slist_data(&sl), ":\n");
		_ls_directory(slist_data(&sl), prefs);
		slist_next(&sl);
	}
	return 0;
}

static int _ls_directory_do(char * filename, DIR * dir, SList * files,
		Prefs * prefs);
static int _ls_directory(char * filename, Prefs * prefs)
{
	DIR * dir;
	SList * files;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "_ls_directory(", filename, ")\n");
#endif
	if((dir = opendir(filename)) == NULL)
	{
		fprintf(stderr, "%s", "ls: ");
		perror(filename);
		return 2;
	}
	if((files = slist_new()) == NULL)
	{
		closedir(dir);
		return 1;
	}
	_ls_directory_do(filename, dir, files, prefs);
	slist_delete(files);
	closedir(dir);
	return 0;
}

static int _ls_free(char * filename, void * null);
static int _ls_directory_do(char * filename, DIR * dir, SList * files,
		Prefs * prefs)
{
	char * str = NULL;
	unsigned int len;
	struct dirent * dirent;
	struct stat st;
	char * p;

	len = strlen(filename);
	if((str = strdup(filename)) == NULL)
	{
		fprintf(stderr, "%s", "ls: ");
		perror("strdup");
		return 2;
	}
	while((dirent = readdir(dir)) != NULL)
	{
		if((p = realloc(str, len + strlen(dirent->d_name)
						+ 2)) == NULL)
		{
			fprintf(stderr, "%s", "ls: ");
			perror("realloc");
			break;
		}
		str = p;
		str[len] = '/';
		str[len+1] = '\0';
		strcat(str, dirent->d_name);
#ifdef DEBUG
		fprintf(stderr, "stat(%s)\n", str);
#endif
		if(stat(str, &st) == -1)
		{
			fprintf(stderr, "%s", "ls: ");
			perror(dirent->d_name);
			continue;
		}
		if((*prefs & PREFS_a) == 0 && *(dirent->d_name) == '.')
			continue;
		if((p = strdup(dirent->d_name)) == NULL)
		{
			fprintf(stderr, "%s", "ls: ");
			perror("strdup");
			continue;
		}
		slist_insert_sorted(files, p, strcmp);
	}
	free(str);
	_ls_files(files, prefs);
	slist_apply(files, _ls_free, prefs);
	return 0;
}

static int _ls_free(char * filename, void * null)
{
	free(filename);
	return 0;
	null = null;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: ls [-CFRa]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p;

	if(_prefs_parse(&p, argc, argv) != 0)
		return _usage();
	return _ls(argc - optind, &argv[optind], &p);
}
