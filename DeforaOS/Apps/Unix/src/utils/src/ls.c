/* ls.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern int optind;
#include <dirent.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>


/* FIXME */
#define max(a, b) ((a) > (b) ? (a) : (b))

/* Prefs */
typedef int Prefs;
#define PREFS_a 1
#define PREFS_d 2
#define PREFS_l 4
#define PREFS_1 8
#define PREFS_R 16

static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "CFRadl1")) != -1)
	{
		switch(o)
		{
			case 'C':
			case 'F':
				fprintf(stderr, "%s%c%s", "ls: -", o,
						": Not yet implemented\n");
				return 1;
			case 'R':
				*prefs |= PREFS_R;
				break;
			case 'a':
				*prefs |= PREFS_a;
				break;
			case 'd':
				*prefs |= PREFS_d;
				break;
			case 'l':
				*prefs |= PREFS_l;
				break;
			case '1':
				*prefs |= PREFS_1;
				break;
			case '?':
				return 1;
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
	SList sl = *slist;

	if(sl == NULL)
	{
		*slist = _slistcell_new(data, NULL);
		return *slist != NULL ? 0 : 2;
	}
	slist_last(&sl);
	sl->next = _slistcell_new(data, NULL);
	return sl->next != NULL ? 0 : 2;
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
static int _ls_error(char * message, int ret);
static int _ls_directory_do(char * dir, Prefs * prefs);
static int _ls_args(SList ** files, SList ** dirs);
static int _is_directory(char * dir, Prefs * prefs);
static int _ls_do(char * directory, SList * files, SList * dirs, Prefs * prefs);
static int _ls(int argc, char * argv[], Prefs * prefs)
{
	SList * files;
	SList * dirs;
	int res = 0;
	int i;
	int j;
	char * str;

	if(argc == 0)
		return _ls_directory_do(".", prefs);
	if(_ls_args(&files, &dirs) != 0)
		return 2;
	for(i = 0; i < argc; i++)
	{
		if((j = _is_directory(argv[i], prefs)) == 2)
		{
			res++;
			continue;
		}
		if((str = strdup(argv[i])) == NULL)
		{
			res += _ls_error("malloc", 1);
			continue;
		}
		if(*prefs & PREFS_d)
		{
			res += slist_insert_sorted(files, str, strcmp);
			continue;
		}
		res += slist_insert_sorted(j ? dirs : files, str, strcmp);
	}
	res += _ls_do(NULL, files, dirs, prefs);
	return res == 1 ? 2 : res;
}

static int _ls_error(char * message, int ret)
{
	fprintf(stderr, "%s", "ls: ");
	perror(message);
	return ret;
}

static int _ls_directory_do(char * directory, Prefs * prefs)
{
	SList * files;
	SList * dirs;
	int res = 0;
	DIR * dir;
	struct dirent * de;
	char * file = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "_ls_directory_do(%s, ...)\n", directory);
#endif
	if((dir = opendir(directory)) == NULL)
		return _ls_error(directory, 2);
	_ls_args(&files, &dirs);
	readdir(dir);
	readdir(dir);
	while((de = readdir(dir)) != NULL)
	{
		slist_insert_sorted(files, strdup(de->d_name), strcmp);
		if((p = realloc(file, strlen(directory)
						+ strlen(de->d_name)
						+ 2)) == NULL)
		{
			_ls_error("malloc", 0);
			continue;
		}
		file = p;
		sprintf(file, "%s/%s", directory, de->d_name);
		if((*prefs & PREFS_R) && _is_directory(file, prefs) == 1)
			slist_insert_sorted(dirs, strdup(file), strcmp);
	}
	free(file);
	closedir(dir);
	_ls_do(directory, files, dirs, prefs);
	return res;
}

static int _ls_args(SList ** files, SList ** dirs)
{
	if((*files = slist_new()) == NULL)
		return _ls_error("slist", 1);
	if((*dirs = slist_new()) == NULL)
	{
		slist_delete(*files);
		return _ls_error("slist", 1);
	}
	return 0;
}

static int _is_directory(char * file, Prefs * prefs)
{
	struct stat st;

	if((stat(file, &st)) != 0)
		return _ls_error(file, 2);
	return S_ISDIR(st.st_mode) ? 1 : 0;
}

static int _ls_do_files(char * directory, SList * files, Prefs * prefs);
static int _ls_do_dirs(char * directory, SList * dirs, Prefs * prefs);
static int _ls_do(char * directory, SList * files, SList * dirs, Prefs * prefs)
{
	int res = 0;

	res += _ls_do_files(directory, files, prefs);
	res += _ls_do_dirs(directory, dirs, prefs);
	return res;
}

static int _ls_free(void * data, void * user);
static int _ls_do_files_short(SList * files, Prefs * prefs);
static int _ls_do_files_long(char * directory, SList * files, Prefs * prefs);
static int _ls_do_files(char * directory, SList * files, Prefs * prefs)
{
	int res = 0;

	if(*prefs & PREFS_l)
		res = _ls_do_files_long(directory, files, prefs);
	else
		res = _ls_do_files_short(files, prefs);
	slist_apply(files, _ls_free, NULL);
	slist_delete(files);
	return res;
}

static int _ls_do_files_short(SList * files, Prefs * prefs)
{
	char * cols;
	char * p;
	unsigned int len = 0;
	unsigned int lenmax = 0;
	unsigned int colnb = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	SList cur;

	if(((*prefs & PREFS_1) == 0) && (cols = getenv("COLUMNS")) != NULL
			&& *cols != '\0'
			&& (len = strtol(cols, &p, 10)) > 0
			&& *p == '\0')
	{
		for(cur = *files; cur != NULL; slist_next(&cur))
			lenmax = max(lenmax, strlen(slist_data(&cur)));
		if(lenmax > 0)
			colnb = len / ++lenmax;
	}
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		p = slist_data(&cur);
		j = strlen(p);
		fwrite(p, sizeof(char), j, stdout);
		if(i + 1 < colnb)
		{
			for(i++; j < lenmax; j++)
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

static void _long_mode(char str[11], mode_t mode);
static char * _long_owner(uid_t uid);
static char * _long_group(gid_t gid);
static char * _long_date(time_t date);
static int _ls_do_files_long(char * directory, SList * files, Prefs * prefs)
{
	SList cur;
	char * file = NULL;
	char * p;
	struct stat st;
	char mode[11];
	char * owner;
	char * group;
	char * date;

#ifdef DEBUG
	fprintf(stderr, "DEBUG _ls_do_files_long(%s, ...)\n", directory);
#endif
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		/* FIXME */
		if((p = realloc(file, strlen(directory)
						+ strlen(slist_data(&cur))
						+ 2)) == NULL)
		{
			_ls_error("malloc", 0);
			continue;
		}
		file = p;
		sprintf(file, "%s/%s", directory, slist_data(&cur));
		if(stat(file, &st) != 0)
		{
			_ls_error(file, 0);
			continue;
		}
		_long_mode(mode, st.st_mode);
		owner = _long_owner(st.st_uid);
		group = _long_group(st.st_gid);
		date = _long_date(st.st_mtime);
		printf("%s %u %s %s %lu %s %s\n", mode, st.st_nlink,
				owner, group, st.st_size, date,
				slist_data(&cur));
	}
	free(file);
	return 0;
}

static void _long_mode(char str[11], mode_t mode)
{
	unsigned int i;

	str[10] = '\0';
	/* FIXME */
	for(i = 0; i < 10; i++)
		str[i] = '-';
}

static char * _long_owner(uid_t uid)
{
	struct passwd * pwd;

	if((pwd = getpwuid(uid)) == NULL)
		return "unknown";
	return pwd->pw_name;
}

static char * _long_group(gid_t gid)
{
	struct group * grp;

	if((grp = getgrgid(gid)) == NULL)
		return "unknown";
	return grp->gr_name;
}

static char * _long_date(time_t date)
{
	/* FIXME */
	return NULL;
}

static int _ls_free(void * data, void * user)
{
	free(data);
	return 0;
	user = user;
}

static int _ls_do_dirs(char * directory, SList * dirs, Prefs * prefs)
{
	int res = 0;
	SList cur;
	char * dir = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG _ls_do_dirs(%s, ...)\n", directory);
#endif
	for(cur = *dirs; cur != NULL; slist_next(&cur))
	{
		dir = slist_data(&cur);
		printf("\n%s%s", dir, ":\n");
		res += _ls_directory_do(dir, prefs);
	}
	slist_apply(dirs, _ls_free, NULL);
	slist_delete(dirs);
	return res;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: ls [-CFRadl1]\n");
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
