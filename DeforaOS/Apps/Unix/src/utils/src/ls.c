/* ls.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/* FIXME */
#define max(a, b) ((a) > (b) ? (a) : (b))

/* Prefs */
typedef int Prefs;
#define PREFS_C 00001
#define PREFS_F 00002
#define PREFS_R 00004
#define PREFS_a 00010
#define PREFS_c 00020
#define PREFS_d 00040
#define PREFS_l 00100
#define PREFS_u 00200
#define PREFS_1 00400
#define PREFS_H 01000
#define PREFS_L 02000

static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "CFRacdlu1HL")) != -1)
	{
		switch(o)
		{
			case 'C':
				*prefs -= *prefs & PREFS_1;
				*prefs |= PREFS_C;
				break;
			case 'F':
				*prefs |= PREFS_F;
				break;
			case 'R':
				*prefs |= PREFS_R;
				break;
			case 'a':
				*prefs |= PREFS_a;
				break;
			case 'c':
				*prefs -= *prefs & PREFS_u;
				*prefs |= PREFS_c;
				break;
			case 'd':
				*prefs |= PREFS_d;
				break;
			case 'l':
				*prefs |= PREFS_l;
				break;
			case 'u':
				*prefs -= *prefs & PREFS_c;
				*prefs |= PREFS_u;
				break;
			case '1':
				*prefs |= PREFS_1;
				break;
			case 'H':
				*prefs -= *prefs & PREFS_L;
				*prefs |= PREFS_H;
				break;
			case 'L':
				*prefs -= *prefs & PREFS_H;
				*prefs |= PREFS_L;
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

/* static void slist_last(SList * slist)
{
	if(*slist == NULL)
		return;
	while((*slist)->next != NULL)
		*slist = (*slist)->next;
} */

/* static int slist_append(SList * slist, void * data)
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
} */

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

/* static size_t slist_length(SList * slist)
{
	SListCell * slc = *slist;
	size_t len;

	for(len = 0; slc != NULL; len++)
		slc = slc->next;
	return len;
} */


/* ls */
static int _ls_error(char const * message, int ret);
static int _ls_directory_do(char * dir, Prefs * prefs);
static int _ls_args(SList ** files, SList ** dirs);
static int _is_directory(Prefs * prefs, char * dir);
static int _ls_do(char * directory, SList * files, SList * dirs, Prefs * prefs);
typedef int (*compare_func)(void*, void*);
static int _ls(int argc, char * argv[], Prefs * prefs)
{
	SList * files;
	SList * dirs;
	int res = 0;
	int i;
	int isdir;
	char * str;

	if(argc == 0)
		return _ls_directory_do(".", prefs);
	if(_ls_args(&files, &dirs) != 0)
		return 2;
	for(i = 0; i < argc; i++)
	{
		if((isdir = _is_directory(prefs, argv[i])) == 2)
			res++;
		else if((str = strdup(argv[i])) == NULL)
			res += _ls_error("malloc", 1);
		else if(*prefs & PREFS_d)
			res += slist_insert_sorted(files, str,
					(compare_func)strcmp);
		else
			res += slist_insert_sorted(isdir ? dirs : files, str,
				(compare_func)strcmp);
	}
	res += _ls_do(NULL, files, dirs, prefs);
	return res == 1 ? 2 : res;
}

static int _ls_error(char const * message, int ret)
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
	int pos = 1;

#ifdef DEBUG
	fprintf(stderr, "_ls_directory_do(%s, ...)\n", directory);
#endif
	if((dir = opendir(directory)) == NULL)
		return _ls_error(directory, 2);
	_ls_args(&files, &dirs);
	for(; (de = readdir(dir)) != NULL; pos++)
	{
		if(*(de->d_name) == '.' && !(*prefs & PREFS_a))
			continue;
		slist_insert_sorted(files, strdup(de->d_name),
				(compare_func)strcmp);
		if(pos <= 2)
			continue;
		if((p = realloc(file, strlen(directory) + strlen(de->d_name)
						+ 2)) == NULL)
		{
			_ls_error("malloc", 0);
			continue;
		}
		file = p;
		sprintf(file, "%s/%s", directory, de->d_name);
		if((*prefs & PREFS_R) && _is_directory(prefs, file) == 1)
			slist_insert_sorted(dirs, strdup(file),
					(compare_func)strcmp);
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

static int _is_directory(Prefs * prefs, char * file)
{
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;

	if(*prefs & PREFS_H)
		_stat = stat;
	if((_stat(file, &st)) != 0)
		return _ls_error(file, 2);
	return S_ISDIR(st.st_mode) ? 1 : 0;
}

static int _ls_do_files(char * directory, SList * files, Prefs * prefs);
static int _ls_do_dirs(SList * dirs, Prefs * prefs);
static int _ls_do(char * directory, SList * files, SList * dirs, Prefs * prefs)
{
	int res = 0;

	res += _ls_do_files(directory, files, prefs);
	res += _ls_do_dirs(dirs, prefs);
	return res;
}

static int _ls_free(void * data, void * user);
static int _ls_do_files_short(char * directory, SList * files, Prefs * prefs);
static int _ls_do_files_long(char * directory, SList * files, Prefs * prefs);
static int _ls_do_files(char * directory, SList * files, Prefs * prefs)
{
	int res = 0;

	if(*prefs & PREFS_l)
		res = _ls_do_files_long(directory, files, prefs);
	else
		res = _ls_do_files_short(directory, files, prefs);
	slist_apply(files, _ls_free, NULL);
	slist_delete(files);
	return res;
}

static char _short_file_mode(Prefs * prefs, char const * directory,
		char const * file);
static int _ls_do_files_short(char * directory, SList * files, Prefs * prefs)
{
	char * cols;
	char * p;
	unsigned int len = 0;
	unsigned int lenmax = 0;
	unsigned int colnb = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	char c;
	SList cur;

	if(((*prefs & PREFS_1) == 0) && (cols = getenv("COLUMNS")) != NULL
			&& *cols != '\0' && (len = strtol(cols, &p, 10)) > 0
			&& *p == '\0')
	{
		for(cur = *files; cur != NULL; slist_next(&cur))
			lenmax = max(lenmax, strlen(slist_data(&cur)));
		if(*prefs * PREFS_F)
			lenmax++;
		if(lenmax > 0)
			colnb = len / ++lenmax;
	}
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		p = slist_data(&cur);
		j = strlen(p);
		fwrite(p, sizeof(char), j, stdout);
		if((*prefs & PREFS_F)
				&& (c = _short_file_mode(prefs, directory, p)))
		{
			fputc(c, stdout);
			j++;
		}
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

static char _file_mode_letter(mode_t mode);
static char _short_file_mode(Prefs * prefs, char const * directory,
		char const * file)
{
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;
	char * p;
	char c = '\0';

	if((p = malloc(strlen(directory) + 1 + strlen(file) + 1)) == NULL)
		return _ls_error("malloc", 0);
	sprintf(p, "%s/%s", directory, file);
	if(*prefs & PREFS_H)
		_stat = stat;
	if(_stat(p, &st) != 0)
		_ls_error(file, 0);
	else
		c = _file_mode_letter(st.st_mode);
	free(p);
	return c;
}

static void _long_mode(char str[11], mode_t mode);
static char * _long_owner(uid_t uid);
static char * _long_group(gid_t gid);
static void _long_date(time_t date, char buf[15]);
static char _file_mode_letter(mode_t mode);
static int _ls_do_files_long(char * directory, SList * files, Prefs * prefs)
{
	SList cur;
	char * file = NULL;
	char * p;
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;
	char mode[11];
	char * owner;
	char * group;
	char date[15];

#ifdef DEBUG
	fprintf(stderr, "DEBUG _ls_do_files_long(%s, ...)\n", directory);
#endif
	if(*prefs & PREFS_H)
		_stat = stat;
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		/* FIXME */
		p = slist_data(&cur);
		if((p = realloc(file, strlen(directory) + strlen(p) + 2))
				== NULL)
		{
			_ls_error("malloc", 0);
			continue;
		}
		file = p;
		p = slist_data(&cur);
		sprintf(file, "%s/%s", directory, p);
		if(_stat(file, &st) != 0)
		{
			_ls_error(file, 0);
			continue;
		}
		_long_mode(mode, st.st_mode);
		owner = _long_owner(st.st_uid);
		group = _long_group(st.st_gid);
		if(*prefs & PREFS_u)
			_long_date(st.st_atime, date);
		else if(*prefs & PREFS_c)
			_long_date(st.st_ctime, date);
		else
			_long_date(st.st_mtime, date);
		printf("%s %u %s %s %lu %s %s%c\n", mode, st.st_nlink,
				owner, group, st.st_size, date, p,
				(*prefs & PREFS_F)
				? _file_mode_letter(st.st_mode) : '\0');
	}
	free(file);
	return 0;
}

static void _long_mode(char str[11], mode_t mode)
{
	unsigned int i;

	str[0] = '-';
	if(!S_ISREG(mode))
	{
		if(S_ISLNK(mode))
			str[0] = 'l';
		else if(S_ISBLK(mode))
			str[0] = 'b';
		else if(S_ISCHR(mode))
			str[0] = 'c';
		else if(S_ISFIFO(mode))
			str[0] = 'p';
		else if(S_ISDIR(mode))
			str[0] = 'd';
	}
	for(i = 1; i < 10; i++)
		str[i] = '-';
	if(mode & S_IRUSR)
		str[1] = 'r';
	if(mode & S_IWUSR)
		str[2] = 'w';
	if(mode & S_IXUSR)
		str[3] = (mode & S_ISUID ? 's' : 'x');
	else if(mode & S_ISUID)
		str[3] = 'S';
	if(mode & S_IRGRP)
		str[4] = 'r';
	if(mode & S_IWGRP)
		str[5] = 'w';
	if(mode & S_IXGRP)
		str[6] = 'x';
	if(mode & S_IROTH)
		str[7] = 'r';
	if(mode & S_IWOTH)
		str[8] = 'w';
	if(mode & S_IXOTH)
		str[9] = 'x';
	str[10] = '\0';
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

static void _long_date(time_t date, char buf[15])
{
	struct tm tm;
	static time_t sixmonths = -1;

	if(sixmonths == -1)
		sixmonths = time(NULL) - 15552000;
	localtime_r(&date, &tm);
	if(date < sixmonths)
		strftime(buf, 14, "%b %e  %Y", &tm);
	else
		strftime(buf, 14, "%b %e %H:%M", &tm);
}

static char _file_mode_letter(mode_t mode)
{
	if(S_ISLNK(mode)) /* FIXME not in POSIX? */
		return '@';
	if(S_ISDIR(mode))
		return '/';
	if(S_ISFIFO(mode))
		return '|';
	if(mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		return '*';
	return '\0';
}

static int _ls_free(void * data, void * user)
{
	free(data);
	return 0;
	user = user;
}

static int _ls_do_dirs(SList * dirs, Prefs * prefs)
{
	int res = 0;
	SList cur;
	char * dir = NULL;

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
	fprintf(stderr, "%s", "Usage: ls [-CFRacdilqrtu1][-H | -L]\n\
  -F    write a symbol after files names depending on their type\n\
  -R    recursively list subdirectories encountered\n\
  -a    write out all hidden directory entries\n\
  -c    use time of last modification of file status\n\
  -l    write out in long format\n\
  -u    use time of last access\n\
  -H    dereference symbolic links\n\
  -L    evaluate symbolic links\n");
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
