/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <limits.h>


/* ps */
/* private */
/* types */
typedef struct _Prefs
{
	char const * group;
	pid_t pid;
	char const * username;
} Prefs;


/* prototypes */
static int _ps(Prefs * prefs);
static int _ps_error(char const * message, int ret);


/* functions */
/* ps */
static int _ps(Prefs * prefs)
{
	char const proc[] = "/proc";
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	char buf[256];
	DIR * dir;
	struct dirent * de;
	long pid;
	char * p;
	FILE * fp;
	struct stat st;
	size_t s;

	if(prefs->username != NULL && (pw = getpwnam(prefs->username)) == NULL)
		return -_ps_error(prefs->username, 1);
	if(prefs->group != NULL && (gr = getgrnam(prefs->group)) == NULL)
		return -_ps_error(prefs->group, 1);
	if((dir = opendir(proc)) == NULL)
		return -_ps_error(proc, 1);
	printf("%5s %5s %s\n", "PID", "UID", "CMD");
	while((de = readdir(dir)) != NULL)
	{
		pid = strtol(de->d_name, &p, 10);
		if(de->d_name[0] == '\0' || *p != '\0' || pid < 0)
			continue;
		if(prefs->pid != -1 && pid != prefs->pid)
			continue;
		snprintf(buf, sizeof(buf), "%s/%s", proc, de->d_name);
		if(lstat(buf, &st) != 0)
		{
			_ps_error(buf, 0);
			continue;
		}
		if(pw != NULL && st.st_uid != pw->pw_uid)
			continue;
		if(gr != NULL && st.st_gid != gr->gr_gid)
			continue;
		snprintf(buf, sizeof(buf), "%s/%s/%s", proc, de->d_name,
				"cmdline");
		if((fp = fopen(buf, "r")) != NULL)
		{
			s = fread(buf, sizeof(*buf), sizeof(buf) - 1, fp);
			fclose(fp);
			buf[s] = '\0';
		}
		else
			buf[0] = '\0';
		printf("%5lu %5lu %s\n", pid, (unsigned long)st.st_uid, buf);
	}
	closedir(dir);
	return 0;
}


/* ps_error */
static int _ps_error(char const * message, int ret)
{
	fputs("ps: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ps [-g group][-p pid][-u username]\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.pid = -1;
	while((o = getopt(argc, argv, "g:p:u:")) != -1)
		switch(o)
		{
			case 'g':
				prefs.group = optarg;
				break;
			case 'p':
				prefs.pid = strtol(optarg, &p, 0);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.pid < 0)
					return _usage();
				break;
			case 'u':
				prefs.username = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_ps(&prefs) == 0) ? 0 : 2;
}
