/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/types.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Prefs */
typedef int Prefs;
#define DF_PREFS_k 1
#define DF_PREFS_P 2


/* df */
static int _df_error(char const * message, int ret);
static int _df_mtab(Prefs * prefs);
static int _df_do(Prefs * prefs, char const * file);
static int _df(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	int i;

	printf("%s%s%s", "Filesystem ", (*prefs & DF_PREFS_k) ? "1024" : " 512",
			"-blocks       Used  Available Capacity Mounted on\n");
	if(filec == 0)
		return _df_mtab(prefs);
	for(i = 0; i < filec; i++)
		ret |= _df_do(prefs, filev[i]);
	return ret;
}

static int _df_error(char const * message, int ret)
{
	fputs("df: ", stderr);
	perror(message);
	return ret;
}

static void _df_print(Prefs * prefs, struct statvfs const * f);
static int _df_mtab(Prefs * prefs)
{
#ifdef ST_WAIT
	int cnt;
	struct statvfs * f;
	int i;

	if((cnt = getvfsstat(NULL, 0, ST_WAIT)) < 0)
		return _df_error("getvfsstat", 1);
	if((f = malloc(sizeof(*f) * cnt)) == NULL)
		return _df_error("malloc", 1);
	if(getvfsstat(f, sizeof(*f) * cnt, ST_WAIT) != cnt)
	{
		free(f);
		return _df_error("getvfsstat", 1);
	}
	for(i = 0; i < cnt; i++)
		_df_print(prefs, &f[i]);
	free(f);
#else /* FIXME incomplete workaround when getvfsstat() is missing */
	struct statvfs f;

	if(statvfs(".", &f) != 0)
		return _df_error(".", 1);
	_df_print(prefs, &f);
#endif
	return 0;
}

static void _df_print(Prefs * prefs, struct statvfs const * f)
{
	unsigned long long mod;
	unsigned long long cnt;
	unsigned long long used;
	unsigned long long avail;
	unsigned int cap;

	mod = f->f_bsize / ((*prefs & DF_PREFS_k) ? 8192 : 4096);
	cnt = f->f_blocks * mod;
	used = (f->f_blocks - f->f_bfree) * mod;
	avail = f->f_bavail * mod;
	cap = ((f->f_blocks - f->f_bfree) * 100)
		/ ((f->f_blocks - f->f_bfree) + f->f_bavail);
#ifdef ST_WAIT
	printf("%-11s %10llu %10llu %10llu %7u%% %s\n", f->f_mntfromname,
			cnt, used, avail, cap, f->f_mntonname);
#else
	printf("%-11s %10llu %10llu %10llu %7u%% %s\n", "", cnt, used, avail,
			cap, "");
#endif
}

static int _df_do(Prefs * prefs, char const * file)
{
	struct statvfs f;

	if(statvfs(file, &f) != 0)
		return _df_error(file, 1);
	_df_print(prefs, &f);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: df [-k][-P][file...]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "kP")) != -1)
		switch(o)
		{
			case 'k':
				prefs |= DF_PREFS_k;
				break;
			case 'P':
				prefs |= DF_PREFS_P;
				break;
			default:
				return _usage();
		}
	return _df(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
