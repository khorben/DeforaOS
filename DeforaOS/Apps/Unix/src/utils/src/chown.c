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
static int _chown_owner(char * owner, uid_t * uid, gid_t * gid);
static int _chown_do(int opts, uid_t uid, gid_t gid, int argc, char * argv[]);
static int _chown(int opts, char * owner, int argc, char * argv[])
{
	uid_t uid;
	gid_t gid;

	if(_chown_owner(owner, &uid, &gid) != 0)
		return 2;
	return _chown_do(opts, uid, gid, argc, argv);
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
	if((*uid = _chown_uid(owner)) == -1)
		return 1;
	if(owner[i] != '\0' && (*gid = _chown_gid(&owner[i])) == -1)
		return 1;
	else
		*gid = -1;
	return 0;
}

static uid_t _chown_uid(char * owner)
{
	struct passwd * pwd;

	if((pwd = getpwnam(owner)) == NULL)
		return -1;
	return pwd->pw_uid;
}

static gid_t _chown_gid(char * group)
{
	struct group * gr;

	if((gr = getgrnam(group)) == NULL)
		return -1;
	return gr->gr_gid;
}

static int _chown_do(int opts, uid_t uid, gid_t gid, int argc, char * argv[])
{
	return 2;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: chown [-hR] owner[:group] file ...\n\
       chown -R [-H | -L | -P] owner[:group] file ...\n");
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
