/* chgrp.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* types */
#define OPT_h 1
#define OPT_R 2
#define OPT_H 4
#define OPT_L 8
#define OPT_P 12


/* chgrp */
static int _chgrp_grp_error(char * group);
static int _chgrp_gid(int opts, gid_t gid, int argc, char * argv[]);
static int _chgrp(int opts, char * group, int argc, char * argv[])
{
	struct group * grp;

	if((grp = getgrnam(group)) == NULL)
		return _chgrp_grp_error(group);
	return _chgrp_gid(opts, grp->gr_gid, argc, argv);
}

static int _chgrp_grp_error(char * group)
{
	fprintf(stderr, "%s", "chgrp: ");
	if(errno == 0)
		fprintf(stderr, "%s%s", group, ": Unknown group\n");
	else
		perror(group);
	return 2;
}

static int _chgrp_do_recursive(int opts, gid_t gid, char * file);
static int _chgrp_do(int opts, gid_t gid, char * file);
static int _chgrp_gid(int opts, gid_t gid, int argc, char * argv[])
{
	int res = 0;
	int i;

	if((opts & OPT_R) == OPT_R)
	{
		for(i = 0; i < argc; i++)
			res +=_chgrp_do_recursive(opts, gid, argv[i]);
		return res;
	}
	for(i = 0; i < argc; i++)
		res +=_chgrp_do(opts, gid, argv[i]);
	return res;
}

static int _chgrp_do_recursive_do(int opts, gid_t gid, char * file);
static int _chgrp_do_recursive(int opts, gid_t gid, char * file)
{
	struct stat st;

	if((lstat(file, &st)) != 0)
	{
		fprintf(stderr, "%s", "chgrp: ");
		perror(file);
		return 1;
	}
	if(S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode))
		_chgrp_do_recursive_do(opts, gid, file);
	return 0;
}

static int _chgrp_do_recursive_do(int opts, gid_t gid, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
	{
		fprintf(stderr, "%s", "chgrp: ");
		perror(file);
		return 1;
	}
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len-1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		fprintf(stderr, "%s", "chgrp: ");
		perror(file);
		return 1;
	}
	strcpy(s, file);
	s[len-2] = '/';
	s[len-1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			fprintf(stderr, "%s", "chgrp: ");
			perror("malloc");
			continue;
		}
		s = p;
		strcat(s, de->d_name);
#ifdef DEBUG
		fprintf(stderr, "%s\n", s);
#endif
		_chgrp_do(opts, gid, s);
		_chgrp_do_recursive(opts, gid, s);
		s[len-1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
}

static int _chgrp_do(int opts, gid_t gid, char * file)
{
	struct stat st;
	int res;

	if(stat(file, &st) != 0)
	{
		fprintf(stderr, "%s", "chgrp: ");
		perror(file);
		return 1;
	}
	if((opts & OPT_h) == OPT_h)
		res = lchown(file, st.st_uid, gid);
	else
		res = chown(file, st.st_uid, gid);
	if(res != 0)
	{
		fprintf(stderr, "%s", "chgrp: ");
		perror(file);
		return 1;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: chgrp [-hR] group file ...\n\
       chgrp -R [-H | -L | -P] group file ...\n");
	return 1;
}

/* main */
int main(int argc, char * argv[])
{
	int o;
	int opts = 0;

	while((o = getopt(argc, argv, "hRHLP")) != -1)
	{
		switch(o)
		{
			case 'h':
				if((opts & OPT_h) == 0)
					opts += OPT_h;
				break;
			case 'R':
				if((opts & OPT_R) == 0)
					opts += OPT_R;
				break;
			case 'H':
			case 'L':
			case 'P':
				fprintf(stderr, "%s%c%s", "chgrp: -", o,
						": Not yet implemented\n");
			default:
				return _usage();
		}
	}
	if(argc - optind < 2)
		return _usage();
	return _chgrp(opts, argv[optind], argc - optind - 1, &argv[optind+1]);
}
