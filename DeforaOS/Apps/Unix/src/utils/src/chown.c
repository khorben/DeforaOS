/* chown.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
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


/* chown */
static int _chown_error(char * message, int ret);
static int _chown_owner(char * owner, uid_t * uid, gid_t * gid);
static int _chown_do_recursive(int opts, uid_t uid, gid_t gid, char * file);
static int _chown_do(int opts, uid_t uid, gid_t gid, char * file);
static int _chown(int opts, char * owner, int argc, char * argv[])
{
	uid_t uid;
	gid_t gid;
	int res = 0;
	int i;

	if(_chown_owner(owner, &uid, &gid) != 0)
		return 2;
	if(opts & OPT_R)
	{
		for(i = 0; i < argc; i++)
			res += _chown_do_recursive(opts, uid, gid, argv[i]);
		return res;
	}
	for(i = 0; i < argc; i++)
		res += _chown_do(opts, uid, gid, argv[i]);
	return res;
}

static int _chown_error(char * message, int ret)
{
	fprintf(stderr, "%s", "chown: ");
	perror(message);
	return ret;
}

/* PRE
 * POST
 * 		0	success
 * 		else	failure */
static uid_t _chown_uid(char * owner);
static gid_t _chown_gid(char * group);
static int _chown_owner(char * owner, uid_t * uid, gid_t * gid)
{
	int i;

	for(i = 0; owner[i] != 0; i++)
		if(owner[i] == ':')
		{
			owner[i++] = '\0';
			break;
		}
	if((*uid = _chown_uid(owner)) == (uid_t)-1)
		return 1;
	if(owner[i] != '\0' && (*gid = _chown_gid(&owner[i])) == (gid_t)-1)
		return 1;
	return 0;
}

static int _chown_id_error(char * message, char * unknown, int ret);
static uid_t _chown_uid(char * owner)
{
	struct passwd * pwd;

	if((pwd = getpwnam(owner)) == NULL)
		return _chown_id_error(owner, "user", -1);
	return pwd->pw_uid;
}

static int _chown_id_error(char * message, char * unknown, int ret)
{
	if(errno == 0)
	{
		fprintf(stderr, "%s%s%s%s%s", "chown: ", message,
				": Unknown ", unknown, "\n");
		return ret;
	}
	return _chown_error(message, ret);
}

static gid_t _chown_gid(char * group)
{
	struct group * grp;

	if((grp = getgrnam(group)) == NULL)
		return _chown_id_error(group, "group", -1);
	return grp->gr_gid;
}

static int _chown_do_recursive_do(int opts, uid_t uid, gid_t gid, char * file);
static int _chown_do_recursive(int opts, uid_t uid, gid_t gid, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0)
		return _chown_error(file, 1);
	if(!S_ISDIR(st.st_mode))
		return _chown_do(opts, uid, gid, file);
	if(!S_ISLNK(st.st_mode))
		return _chown_do_recursive_do(opts, uid, gid, file);
	return 0;
}

static int _chown_do_recursive_do(int opts, uid_t uid, gid_t gid, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _chown_error(file, 1);
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len-1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		closedir(dir);
		return _chown_error(file, 1);
	}
	strcpy(s, file);
	s[len-2] = '/';
	s[len-1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			_chown_error("malloc", 0);
			continue;
		}
		s = p;
		strcat(s, de->d_name);
		_chown_do_recursive(opts, uid, gid, s);
		s[len-1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
}

static int _chown_do(int opts, uid_t uid, gid_t gid, char * file)
{
	int res;

	if((opts & OPT_h) == OPT_h)
		res = lchown(file, uid, gid);
	else
		res = chown(file, uid, gid);
	if(res != 0)
		return _chown_error(file, 1);
	return 2;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: chown [-hR] owner[:group] file ...\n\
       chown -R [-H | -L | -P] owner[:group] file ...\n\
  -h    Set the user and group IDs on symbolic links\n\
  -R    Recursively change file user and group IDs\n");
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
				opts |= OPT_h;
				break;
			case 'R':
				opts |= OPT_R;
				break;
			case 'H':
			case 'L':
			case 'P':
				fprintf(stderr, "%s%c%s", "chown: -", o,
						": Not yet implemented\n");
			default:
				return _usage();
		}
	}
	if(argc - optind < 2)
		return _usage();
	return _chown(opts, argv[optind], argc - optind - 1, &argv[optind+1]);
}
