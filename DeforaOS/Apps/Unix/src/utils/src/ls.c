/* ls.c */
/* TODO: use a dynamic array and qsort() instead of lists */



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
#include <limits.h>

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
#define PREFS_t 00200
#define PREFS_u 00400
#define PREFS_1 01000
#define PREFS_H 02000
#define PREFS_L 04000

static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "CFRacdltu1HL")) != -1)
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
			case 't':
				*prefs |= PREFS_t;
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
			default:
				return 1;
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


/* ls */
static int _ls_error(char const * message, int ret);
typedef int (*compare_func)(void *, void *);
static compare_func _ls_compare(Prefs * prefs);
static int _ls_directory_do(Prefs * prefs, char * directory);
static int _ls_args(SList ** files, SList ** dirs);
static int _is_directory(Prefs * prefs, char * dir);
static int _ls_do(Prefs * prefs, int argc, char * directory, SList * files,
		SList * dirs);
static int _ls(int argc, char * argv[], Prefs * prefs)
{
	SList * files;
	SList * dirs;
	compare_func cmp = _ls_compare(prefs);
	int ret = 0;
	int i;
	int isdir;
	char * str;

	if(argc == 0 && !(*prefs & PREFS_d))
		return _ls_directory_do(prefs, ".");
	if(_ls_args(&files, &dirs) != 0)
		return 2;
	if(argc == 0)
		ret |= slist_insert_sorted(files, strdup("."), cmp);
	for(i = 0; i < argc; i++)
	{
		if((isdir = _is_directory(prefs, argv[i])) == 2)
			ret |= 1;
		else if((str = strdup(argv[i])) == NULL)
			ret |= _ls_error("malloc", 1);
		else if(*prefs & PREFS_d)
			ret |= slist_insert_sorted(files, str, cmp);
		else
			ret |= slist_insert_sorted(isdir ? dirs : files, str,
					cmp);
	}
	ret |= _ls_do(prefs, argc, NULL, files, dirs);
	return ret == 0 ? 0 : 1;
}

static int _ls_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "ls: ");
	perror(message);
	return ret;
}

static int _acccmp(char * a, char * b);
static int _modcmp(char * a, char * b);
static compare_func _ls_compare(Prefs * prefs)
{
	if(!(*prefs & PREFS_t))
		return (compare_func)strcmp;
	if(*prefs & PREFS_u)
		return (compare_func)_acccmp;
	return (compare_func)_modcmp;
}

static int _acccmp(char * a, char * b)
{
	struct stat sta;
	struct stat stb;

	if(lstat(a, &sta) != 0)
		return _ls_error(a, 0);
	if(lstat(b, &stb) != 0)
		return _ls_error(b, 0);
	return sta.st_atime - stb.st_atime;
}

static int _modcmp(char * a, char * b)
{
	struct stat sta;
	struct stat stb;

	if(lstat(a, &sta) != 0)
		return _ls_error(a, 0);
	if(lstat(b, &stb) != 0)
		return _ls_error(b, 0);
	return sta.st_mtime - stb.st_mtime;
}

static int _ls_directory_do(Prefs * prefs, char * directory)
{
	SList * files;
	SList * dirs;
	compare_func cmp = _ls_compare(prefs);
	DIR * dir;
	struct dirent * de;
	char * file = NULL;
	char * p;
	int pos = 1;

	if((dir = opendir(directory)) == NULL)
		return _ls_error(directory, 1);
	_ls_args(&files, &dirs);
	for(; (de = readdir(dir)) != NULL; pos++)
	{
		if(*(de->d_name) == '.' && !(*prefs & PREFS_a))
			continue;
		slist_insert_sorted(files, strdup(de->d_name), cmp);
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
			slist_insert_sorted(dirs, strdup(file), cmp);
	}
	free(file);
	closedir(dir);
	_ls_do(prefs, 2, directory, files, dirs);
	return 0;
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
	int (*_stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;

	if(*prefs & PREFS_H || *prefs & PREFS_L)
		_stat = stat;
	if((_stat(file, &st)) != 0)
		return _ls_error(file, 2);
	return S_ISDIR(st.st_mode) ? 1 : 0;
}

static int _ls_do_files(Prefs * prefs, char * directory, SList * files);
static int _ls_do_dirs(Prefs * prefs, int argc, SList * dirs);
static int _ls_do(Prefs * prefs, int argc, char * directory, SList * files,
		SList * dirs)
{
	int res = 0;
	char sep = 0;

	if(slist_data(files) != NULL && slist_data(dirs) != NULL)
		sep = '\n';
	res += _ls_do_files(prefs, directory, files);
	if(sep != 0)
		fputc(sep, stdout);
	res += _ls_do_dirs(prefs, argc, dirs);
	return res;
}

static int _ls_free(void * data, void * user);
static int _ls_do_files_short(Prefs * prefs, char * directory, SList * files);
static int _ls_do_files_long(Prefs * prefs, char * directory, SList * files);
static int _ls_do_files(Prefs * prefs, char * directory, SList * files)
{
	int res = 0;

	if(*prefs & PREFS_l)
		res = _ls_do_files_long(prefs, directory, files);
	else
		res = _ls_do_files_short(prefs, directory, files);
	slist_apply(files, _ls_free, NULL);
	slist_delete(files);
	return res;
}

static char _short_file_mode(Prefs * prefs, char const * directory,
		char const * file);
static int _ls_do_files_short(Prefs * prefs, char * directory, SList * files)
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

	if(*prefs & PREFS_H || *prefs & PREFS_L)
		_stat = stat;
	if(directory == NULL)
	{
		if(_stat(file, &st) != 0)
			return _ls_error(file, 0);
	}
	else
	{
		if((p = malloc(strlen(directory) + 1 + strlen(file) + 1))
				== NULL)
			return _ls_error("malloc", 0);
		sprintf(p, "%s/%s", directory, file);
		if(_stat(p, &st) != 0)
		{
			free(p);
			return _ls_error(file, 0);
		}
		free(p);
	}
	return _file_mode_letter(st.st_mode);
}

static void _long_print(Prefs * prefs, char const * filename,
		char const * basename, struct stat * st);
static int _ls_do_files_long(Prefs * prefs, char * directory, SList * files)
{
	SList cur;
	char * file = NULL;
	char * p;
	int (* _stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;

	if(*prefs & PREFS_H || *prefs & PREFS_L)
		_stat = stat;
	for(cur = *files; cur != NULL; slist_next(&cur))
	{
		p = slist_data(&cur);
		if(directory != NULL)
		{
			if((p = realloc(file, strlen(directory) + strlen(p)
							+ 2)) == NULL)
			{
				_ls_error("malloc", 0);
				continue;
			}
			file = p;
			p = slist_data(&cur);
			sprintf(file, "%s/%s", directory, p);
		}
		if(_stat(directory == NULL ? p : file, &st) != 0)
			_ls_error(file, 0);
		else
			_long_print(prefs, file != NULL ? file : p, p, &st);
	}
	free(file);
	return 0;
}

static void _long_mode(char str[11], mode_t mode);
static char * _long_owner(uid_t uid);
static char * _long_group(gid_t gid);
static void _long_date(time_t date, char buf[15]);
static char _file_mode_letter(mode_t mode);
static void _print_link(char const * filename);
static void _long_print(Prefs * prefs, char const * filename,
		char const * basename, struct stat * st)
{
	char mode[11];
	char * owner;
	char * group;
	char date[15];

	_long_mode(mode, st->st_mode);
	owner = _long_owner(st->st_uid);
	group = _long_group(st->st_gid);
	if(*prefs & PREFS_u)
		_long_date(st->st_atime, date);
	else if(*prefs & PREFS_c)
		_long_date(st->st_ctime, date);
	else
		_long_date(st->st_mtime, date);
	printf("%s %u %s %s %6u %s %s", mode, (unsigned)st->st_nlink,
			owner, group, (unsigned)st->st_size, date, basename);
	if(S_ISLNK(st->st_mode) && !(*prefs & PREFS_L)) /* FIXME not in POSIX? */
		_print_link(filename);
	else if(*prefs & PREFS_F)
		fputc(_file_mode_letter(st->st_mode), stdout);
	fputc('\n', stdout);
}

static void _long_mode(char str[11], mode_t mode)
{
	unsigned int i;

	for(i = 0; i < 10; i++)
		str[i] = '-';
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
	return ' ';
}

static void _print_link(char const * filename)
{
	char buf[PATH_MAX+1];
	int len;

	if((len = readlink(filename, buf, sizeof(buf)-1)) == -1)
	{
		_ls_error(filename, 0);
		return;
	}
	buf[len] = '\0';
	printf("%s%s", " -> ", buf);
}

static int _ls_free(void * data, void * user)
{
	free(data);
	return 0;
}

static int _ls_do_dirs(Prefs * prefs, int argc, SList * dirs)
{
	int res = 0;
	SList cur;
	char * dir = NULL;
	char * eol = "";

	for(cur = *dirs; cur != NULL; slist_next(&cur))
	{
		dir = slist_data(&cur);
		if(argc != 1)
			printf("%s%s%s", eol, dir, ":\n");
		eol = "\n";
		res += _ls_directory_do(prefs, dir);
	}
	slist_apply(dirs, _ls_free, NULL);
	slist_delete(dirs);
	return res;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: ls [-CFRacdilqrtu1][-H | -L]\n\
  -C	write multi-column output\n\
  -F	write a symbol after files names depending on their type\n\
  -R	recursively list subdirectories encountered\n\
  -a	write out all hidden directory entries\n\
  -c	use time of last modification of file status\n\
  -d	treat directories like files\n\
  -l	write out in long format\n\
  -t	sort with the last modified file first\n\
  -u	use time of last access\n\
  -1	force output to be one entry per line\n\
  -H	dereference symbolic links\n\
  -L	evaluate symbolic links\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p;

	if(_prefs_parse(&p, argc, argv) != 0)
		return _usage();
	return _ls(argc - optind, &argv[optind], &p) == 0 ? 0 : 1;
}
