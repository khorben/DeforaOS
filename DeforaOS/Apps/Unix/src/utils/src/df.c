/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#include <sys/types.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Prefs */
typedef int Prefs;
#define PREFS_k 1


/* df */
static int _df_error(char const * message, int ret);
static int _df_mtab(Prefs * prefs);
static int _df_do(Prefs * prefs, char const * file);
static int _df(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	int i;

	printf("%s%s%s", "Filesystem ", *prefs & PREFS_k ? "1024" : " 512",
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

static void _df_print(Prefs * prefs, const struct statvfs * f);
static int _df_mtab(Prefs * prefs)
{
	int cnt;
	struct statvfs * f;
	int i;

	/* FIXME not portable code here:
	 * - Linux: /etc/mtab
	 * - NetBSD: getvfsstat() */
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
	return 0;
}

static void _df_print(Prefs * prefs, const struct statvfs * f)
{
	int mod;

	mod = f->f_bsize / ((*prefs & PREFS_k) ? 1024 : 512);
	/* FIXME round up "Use%" result */
	printf("%-11s %10lu %10lu %10lu %7lu%% %s\n", f->f_mntfromname ,
			f->f_blocks * mod, (f->f_blocks - f->f_bfree) * mod,
			f->f_bavail * mod, ((f->f_blocks - f->f_bfree) * 100)
			/ ((f->f_blocks - f->f_bfree) + f->f_bavail),
			f->f_mntonname);
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
	int o;
	Prefs prefs;

	memset(&prefs, sizeof(prefs), 0);
	while((o = getopt(argc, argv, "kP")) != -1)
		switch(o)
		{
			case 'k':
				prefs |= PREFS_k;
				break;
			case 'P':
				break;
			default:
				return _usage();
		}
	return _df(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
