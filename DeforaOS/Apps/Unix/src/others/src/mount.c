/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
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
#include <sys/statvfs.h>
#include <sys/param.h>
#include <sys/mount.h>
#ifdef MOUNT_FFS
# include <ufs/ufs/ufsmount.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* mount */
/* types */
typedef struct _Prefs
{
	int flags;
	char const * options;
	char const * type;
} Prefs;
#define PREFS_a 0x1
#define PREFS_f 0x2


/* functions */
/* mount */
static int _mount_all(Prefs * prefs);
static int _mount_print(void);
static int _mount_do(Prefs * prefs, char const * special, char const * node);

static int _mount(Prefs * prefs, char const * special, char const * node)
{
	if(special == NULL && node == NULL)
		return (prefs->flags & PREFS_a) == PREFS_a
			? _mount_all(prefs) : _mount_print();
	return _mount_do(prefs, special, node);
}

static int _mount_error(char const * message, int ret)
{
	fputs("mount: ", stderr);
	perror(message);
	return ret;
}

static int _mount_all(Prefs * prefs)
{
	int ret = 0;
	Prefs p;
	const char fstab[] = "/etc/fstab";
	FILE * fp;
	char buf[128];
	size_t len;
	int res;
	char special[32];
	char node[32];
	char type[32];
	char options[32];
	unsigned int freq;
	unsigned int passno;

	memcpy(&p, prefs, sizeof(p));
	p.type = type;
	p.options = options;
	if((fp = fopen(fstab, "r")) == NULL)
		return -_mount_error(fstab, 1);
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if((len = strlen(buf)) == 0)
			continue; /* empty line */
		if(buf[len - 1] != '\n')
		{
			errno = E2BIG; /* XXX */
			break; /* line is too long */
		}
		if(buf[0] == '#')
			continue; /* comment */
		freq = 0;
		passno = 0;
		options[0] = '\0';
		res = sscanf(buf, "%31s %31s %31s %31s %u %u\n", special, node,
				type, options, &freq, &passno);
		if(res < 3)
		{
			errno = EINVAL;
			break; /* not enough arguments */
		}
		special[sizeof(special) - 1] = '\0';
		node[sizeof(node) - 1] = '\0';
		type[sizeof(type) - 1] = '\0';
		options[sizeof(options) - 1] = '\0';
		_mount_do(&p, special, node);
	}
	if(!feof(fp))
		ret = -_mount_error(fstab, 1);
	if(fclose(fp) != 0)
		ret = -_mount_error(fstab, 1);
	return ret;
}

static int _mount_print(void)
{
#ifdef ST_WAIT
	int cnt;
	struct statvfs * f;
	int i;

	if((cnt = getvfsstat(NULL, 0, ST_WAIT)) < 0)
		return _mount_error("getvfsstat", 1);
	if((f = malloc(sizeof(*f) * cnt)) == NULL)
		return _mount_error("malloc", 1);
	if(getvfsstat(f, sizeof(*f) * cnt, ST_WAIT) != cnt)
	{
		free(f);
		return _mount_error("getvfsstat", 1);
	}
	for(i = 0; i < cnt; i++)
		printf("%s%s%s%s%s%s%lx%s", f[i].f_mntfromname, " on ",
				f[i].f_mntonname, " type ", f[i].f_fstypename,
				" (", f[i].f_flag, ")\n");
	free(f);
	return 0;
#else /* workaround when getvfsstat() is missing */
	int ret = 0;
	FILE * fp;
	const char mtab[] = "/etc/mtab";
	const char mounts[] = "/proc/mounts";
	char const * file = mtab;
	size_t res;
	char buf[256];

	if((fp = fopen(file, "r")) == NULL)
		file = mounts;
	if((fp = fopen(file, "r")) == NULL)
		return -_mount_error(file, 1);
	while((res = fread(buf, 1, sizeof(buf), fp)) > 0)
		fwrite(buf, 1, res, stdout);
	if(!feof(fp))
		ret = -_mount_error(file, 1);
	if(fclose(fp) != 0)
		ret = -_mount_error(file, 1);
	return ret;
#endif
}

static int _mount_do(Prefs * prefs, char const * special, char const * node)
	/* FIXME handle more flags and options */
{
	void * data = NULL;
	int flags = 0;
#ifdef MOUNT_FFS
	struct ufs_args ffs;
#endif
	struct stat st;

#ifdef MNT_FORCE
	if(prefs->flags & PREFS_f)
		flags |= MNT_FORCE;
#endif
#ifdef MOUNT_FFS
	if(prefs->type != NULL && strcmp(prefs->type, MOUNT_FFS) == 0)
	{
		ffs.fspec = special;
		data = &ffs;
	}
#endif
#ifdef __NetBSD_Version__ /* NetBSD */
# if __NetBSD_Version__ >= 499000000
	if(mount(prefs->type, node, flags, data, 0) == 0)
# else
	if(mount(prefs->type, node, flags, data) == 0)
# endif
#endif
		return 0;
	switch(errno)
	{
		case ENOENT:
			if(stat(node, &st) == 0)
				return -_mount_error(special, 1);
			return -_mount_error(node, 1);
		case ENXIO:
			return -_mount_error(special, 1);
		default:
			return -_mount_error(node, 1);
	}
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mount [-a][-t type]\n"
"       mount [-f][-o options] special | node\n"
"       mount [-f][-o options] special node\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char const * special = NULL;
	char const * node = NULL;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "afo:t:")) != -1)
		switch(o)
		{
			case 'a':
				prefs.flags |= PREFS_a;
				break;
			case 'f':
				prefs.flags |= PREFS_f;
				break;
			case 'o':
				prefs.options = optarg;
				break;
			case 't':
				prefs.type = optarg;
				break;
			default:
				return _usage();
		}
	if(optind + 1 <= argc)
		special = argv[optind];
	if(optind + 2 == argc)
		node = argv[optind + 1];
	else if(optind + 2 < argc)
		return _usage();
	return (_mount(&prefs, special, node) == 0) ? 0 : 2;
}
