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
#ifdef MOUNT_ADOSFS
# define MT_ADOSFS	MOUNT_ADOSFS
# include <adosfs/adosfs.h>
#endif
#ifdef MOUNT_CD9660
# define MT_ISO9660	MOUNT_CD9660
# include <isofs/cd9660/cd9660_mount.h>
#endif
#ifdef MOUNT_EXT2FS
# define MT_EXT2FS	MOUNT_EXT2FS
# include <ufs/ufs/ufsmount.h>
#endif
#ifdef MOUNT_FFS
# define MT_FFS		MOUNT_FFS
# include <ufs/ufs/ufsmount.h>
#endif
#ifdef MOUNT_HFS
# define MT_HFS		MOUNT_HFS
# include <fs/hfs/hfs.h>
#endif
#ifdef MOUNT_MSDOS
# define MT_FAT		MOUNT_MSDOS
# include <msdosfs/msdosfsmount.h>
#endif
#ifdef MOUNT_NFS
# define MT_NFS		MOUNT_NFS
# include <nfs/nfsmount.h>
#endif
#ifdef MOUNT_NTFS
# define MT_NTFS	MOUNT_NTFS
# include <ntfs/ntfsmount.h>
#endif
#ifdef MOUNT_NULLFS
# define MT_NULLFS	MOUNT_NULLFS
# include <miscfs/nullfs/null.h>
#endif
#ifdef MOUNT_PROCFS
# define MT_PROCFS	MOUNT_PROCFS
# include <miscfs/procfs/procfs.h>
# define PROCFS_ARGS_VERSION	PROCFS_ARGSVERSION
#endif
#ifdef MOUNT_TMPFS
# define MT_TMPFS	MOUNT_TMPFS
# include <fs/tmpfs/tmpfs_args.h>
#endif
#ifdef MOUNT_UNION
# define MT_UNIONFS	MOUNT_UNION
# include <miscfs/union/union.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* portability */
#ifndef MT_ISO9660
# define MT_ISO9660	"iso9660"
struct iso_args
{
	char const * fspec;
};
#endif
#ifndef MT_PROCFS
# define MT_PROCFS	"proc"
struct procfs_args
{
	int version;
	int args;
};
# define PROCFS_ARGS_VERSION	0
#endif


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
#define PREFS_u	0x4


/* variables */
static const struct
{
	size_t len;
	char const * name;
	int flags;
} _mount_options[] =
{
#ifdef MNT_ASYNC
	{ 5,	"async",	MNT_ASYNC	},
# ifndef MNT_SYNCHRONOUS
	{ 4,	"sync",		-MNT_ASYNC	},
# endif
#endif
#ifdef MNT_NOATIME
	{ 5,	"atime",	-MNT_NOATIME	},
	{ 7,	"noatime",	MNT_NOATIME	},
#endif
#ifdef MS_NOATIME
	{ 5,	"atime",	-MS_NOATIME	},
	{ 7,	"noatime",	MS_NOATIME	},
#endif
#ifdef MNT_NOCOREDUMP
	{ 8,	"coredump",	-MNT_NOCOREDUMP	},
	{ 10,	"nocoredump",	MNT_NOCOREDUMP	},
#endif
#ifdef MNT_NODEV
	{ 3,	"dev",		-MNT_NODEV	},
	{ 5,	"nodev",	MNT_NODEV	},
#endif
#ifdef MS_NODEV
	{ 3,	"dev",		-MS_NODEV	},
	{ 5,	"nodev",	MS_NODEV	},
#endif
#ifdef MNT_NODEVMTIME
	{ 8,	"devmtime",	-MNT_NODEVMTIME	},
	{ 10,	"nodevmtime",	MNT_NODEVMTIME	},
#endif
#ifdef MNT_NOEXEC
	{ 4,	"exec",		-MNT_NOEXEC	},
	{ 6,	"noexec",	MNT_NOEXEC	},
#endif
#ifdef MS_NOEXEC
	{ 4,	"exec",		-MS_NOEXEC	},
	{ 6,	"noexec",	MS_NOEXEC	},
#endif
#ifdef MNT_NOSUID
	{ 4,	"suid",		-MNT_NOSUID	},
	{ 6,	"nosuid",	MNT_NOSUID	},
#endif
#ifdef MS_NOSUID
	{ 4,	"suid",		-MS_NOSUID	},
	{ 6,	"nosuid",	MS_NOSUID	},
#endif
#ifdef MNT_RDONLY
	{ 2,	"ro",		MNT_RDONLY	},
	{ 2,	"rw",		-MNT_RDONLY	},
#endif
#ifdef MS_RDONLY
	{ 2,	"ro",		MS_RDONLY	},
	{ 2,	"rw",		-MS_RDONLY	},
#endif
#ifdef MNT_SYNCHRONOUS
# ifndef MNT_ASYNC
	{ 5,	"async",	-MNT_SYNCHRONOUS},
# endif
	{ 4,	"sync",		MNT_SYNCHRONOUS	},
#endif
#ifdef MS_SYNCHRONOUS
# ifndef MS_ASYNC
	{ 5,	"async",	-MS_SYNCHRONOUS},
# endif
	{ 4,	"sync",		MS_SYNCHRONOUS	},
#endif
#ifdef MNT_UNION
	{ 5,	"union",	MNT_UNION	},
	{ 7,	"nounion",	-MNT_UNION	},
#endif
	{ 0,	NULL,		0		}
};


/* prototypes */
#ifdef MT_ADOSFS
static int _mount_callback_adosfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_EXT2FS
static int _mount_callback_ext2fs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_FAT
static int _mount_callback_fat(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_FFS
static int _mount_callback_ffs(char const * type, int flags,
		char const * special, char const * node);
#endif
static int _mount_callback_generic(char const * type, int flags,
		char const * special, char const * node);
#ifdef MT_HFS
static int _mount_callback_hfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_ISO9660
static int _mount_callback_iso9660(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_MFS
static int _mount_callback_mfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_NFS
static int _mount_callback_nfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_NTFS
static int _mount_callback_ntfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_NULLFS
static int _mount_callback_nullfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_PROCFS
static int _mount_callback_procfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_TMPFS
static int _mount_callback_tmpfs(char const * type, int flags,
		char const * special, char const * node);
#endif
#ifdef MT_UNIONFS
static int _mount_callback_unionfs(char const * type, int flags,
		char const * special, char const * node);
#endif


/* variables */
static struct
{
	char * name;
	char * type;
	int (*callback)(char const * type, int flags, char const * special,
			char const * node);
} _mount_supported[] =
{
#ifdef MT_ADOSFS
	{ "adosfs",	MT_ADOSFS,	_mount_callback_adosfs	},
#endif
#ifdef MT_EXT2FS
	{ "ext2fs",	MT_EXT2FS,	_mount_callback_ext2fs	},
#endif
#ifdef MT_EXT3FS
	{ "ext3fs",	MT_EXT3FS,	_mount_callback_ext2fs	}, /* XXX */
#endif
#ifdef MT_FAT
	{ "fat",	MT_FAT,		_mount_callback_fat	},
#endif
#ifdef MT_FFS
	{ "ffs",	MT_FFS,		_mount_callback_ffs	},
#endif
#ifdef MT_HFS
	{ "hfs",	MT_HFS,		_mount_callback_hfs	},
#endif
#ifdef MT_ISO9660
	{ "iso9660",	MT_ISO9660,	_mount_callback_iso9660	},
#endif
#ifdef MT_MFS
	{ "mfs",	MT_MFS,		_mount_callback_mfs	},
#endif
#ifdef MT_NFS
	{ "nfs",	MT_NFS,		_mount_callback_nfs	},
#endif
#ifdef MT_NTFS
	{ "ntfs",	MT_NTFS,	_mount_callback_ntfs	},
#endif
#ifdef MT_NULLFS
	{ "nullfs",	MT_NULLFS,	_mount_callback_nullfs	},
#endif
#ifdef MT_PROCFS
	{ "procfs",	MT_PROCFS,	_mount_callback_procfs	},
#endif
#ifdef MT_TMPFS
	{ "tmpfs",	MT_TMPFS,	_mount_callback_tmpfs	},
#endif
#ifdef MT_UNIONFS
	{ "unionfs",	MT_UNIONFS,	_mount_callback_unionfs	},
#endif
	{ NULL,		NULL,		_mount_callback_generic	}
};


/* functions */
/* mount */
static int _mount_all(Prefs * prefs, char const * node);
static int _mount_print(void);
static int _mount_do(Prefs * prefs, char const * special, char const * node);
static int _mount_do_mount(char const * type, int flags, char const * special,
		char const * node, void * data, size_t datalen);
static void _mount_do_options(Prefs * prefs, int * flags);

static int _mount(Prefs * prefs, char const * special, char const * node)
{
	if(special == NULL && node == NULL)
		return (prefs != NULL && (prefs->flags & PREFS_a) == PREFS_a)
			? _mount_all(prefs, NULL) : _mount_print();
	return _mount_do(prefs, special, node);
}

static int _mount_error(char const * message, int ret)
{
	fputs("mount: ", stderr);
	perror(message);
	return ret;
}

static int _mount_all(Prefs * prefs, char const * node)
{
	int ret = 0;
	Prefs p;
	const char fstab[] = "/etc/fstab";
	FILE * fp;
	char buf[128];
	size_t len;
	int res;
	char fsspecial[32];
	char fsnode[32];
	char fstype[32];
	char fsoptions[32];
	unsigned int freq;
	unsigned int passno;

	if(prefs != NULL)
		memcpy(&p, prefs, sizeof(p));
	else
		memset(&p, 0, sizeof(p));
	p.type = fstype;
	p.options = fsoptions;
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
		fsoptions[0] = '\0';
		res = sscanf(buf, "%31s %31s %31s %31s %u %u\n", fsspecial,
				fsnode, fstype, fsoptions, &freq, &passno);
		if(res < 3)
		{
			errno = EINVAL;
			break; /* not enough arguments */
		}
		if(node != NULL && strcmp(node, fsnode) != 0)
			continue;
		if(prefs != NULL && prefs->type != NULL
				&& strcmp(prefs->type, fstype) != 0)
			continue;
		fsspecial[sizeof(fsspecial) - 1] = '\0';
		fsnode[sizeof(fsnode) - 1] = '\0';
		fstype[sizeof(fstype) - 1] = '\0';
		fsoptions[sizeof(fsoptions) - 1] = '\0';
		ret |= _mount_do(&p, fsspecial, fsnode);
	}
	if(!feof(fp))
		ret |= -_mount_error(fstab, 1);
	if(fclose(fp) != 0)
		ret |= -_mount_error(fstab, 1);
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
{
	int flags = 0;
	size_t i;

#ifdef MNT_FORCE
	if(prefs->flags & PREFS_f)
		flags |= MNT_FORCE;
#endif
#ifdef MNT_UPDATE
	if(prefs->flags & PREFS_u)
		flags |= MNT_UPDATE;
#endif
#ifdef MS_REMOUNT
	if(prefs->flags & PREFS_u)
		flags |= MS_REMOUNT;
#endif
	_mount_do_options(prefs, &flags);
	if(prefs->type == NULL)
		return _mount_callback_generic(NULL, flags, special, node);
	for(i = 0; _mount_supported[i].name != NULL; i++)
		if(strcmp(_mount_supported[i].name, prefs->type) == 0)
			return _mount_supported[i].callback(
					_mount_supported[i].type, flags,
					special, node);
	errno = ENOTSUP;
	return -_mount_error(prefs->type, 1);
}

static int _mount_do_mount(char const * type, int flags, char const * special,
		char const * node, void * data, size_t datalen)
{
	struct stat st;

#if defined(__NetBSD__) /* NetBSD */
#if !defined(__NetBSD_Version__) || __NetBSD_Version__ >= 499000000
	if(mount(type, node, flags, data, datalen) == 0)
# else
	if(mount(type, node, flags, data) == 0)
# endif
#elif defined(__FreeBSD__) /* FreeBSD */
	if(mount(type, node, flags, data) == 0)
#else
	struct { char const * fspec; } * d = data;

	if(mount(special, node, type, flags, d->fspec) == 0)
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

static void _mount_do_options(Prefs * prefs, int * flags)
{
	char const * o;
	size_t i;
	size_t j;

	if((o = prefs->options) == NULL)
		return;
	for(i = 0;; i++)
	{
		if(o[i] != ',' && o[i] != '\0')
			continue;
		for(j = 0; j < sizeof(_mount_options) / sizeof(*_mount_options);
				j++)
			if(i > 0 && _mount_options[j].len == i
					&& strncmp(_mount_options[j].name, o, i)
					== 0)
			{
				if(_mount_options[j].flags >= 0)
					*flags |= _mount_options[j].flags;
				else
					*flags &= ~(_mount_options[j].flags);
				break;
			}
		o += i;
		i = 0;
		if(o[i] == '\0')
			break;
		o++;
	}
}

#ifdef MT_ADOSFS
static int _mount_callback_adosfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct adosfs_args adosfs;
	void * data = &adosfs;
	struct stat st;

	memset(&adosfs, 0, sizeof(adosfs));
	if((adosfs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	if(stat(node, &st) == 0)
	{
		adosfs.uid = st.st_uid;
		adosfs.gid = st.st_gid;
		adosfs.mask = ~st.st_mode & 0777;
	}
	/* FIXME actually parse options */
	adosfs.mask = 0755;
	type = MT_ADOSFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(adosfs));
	free(adosfs.fspec);
	return ret;
}
#endif

#ifdef MT_EXT2FS
static int _mount_callback_ext2fs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct ufs_args ffs;
	void * data = &ffs;

	memset(&ffs, 0, sizeof(ffs));
	if((ffs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	type = MT_EXT2FS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(ffs));
	free(ffs.fspec);
	return ret;
}
#endif

#ifdef MT_FAT
static int _mount_callback_fat(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct msdosfs_args msdosfs;
	void * data = &msdosfs;
	struct stat st;

	memset(&msdosfs, 0, sizeof(msdosfs));
	if((msdosfs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	if(stat(node, &st) == 0)
	{
		msdosfs.uid = st.st_uid;
		msdosfs.gid = st.st_gid;
		msdosfs.mask = ~st.st_mode & 0666;
		msdosfs.dirmask = ~st.st_mode & 0777;
	}
	/* FIXME actually parse options */
	msdosfs.version = MSDOSFSMNT_VERSION;
	type = MT_FAT;
	ret = _mount_do_mount(type, flags, special, node, data,
			sizeof(msdosfs));
	free(msdosfs.fspec);
	return ret;
}
#endif

#ifdef MT_FFS
static int _mount_callback_ffs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct ufs_args ffs;
	void * data = &ffs;

	memset(&ffs, 0, sizeof(ffs));
	if((ffs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	type = MT_FFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(ffs));
	free(ffs.fspec);
	return ret;
}
#endif

static int _mount_callback_generic(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	void * data;

	if(node == NULL)
	{
		errno = EINVAL;
		return -_mount_error("mount", 1);
	}
	if(special == NULL)
		return -_mount_all(NULL, node);
	if((data = strdup(special)) == NULL)
		return -_mount_error(special, 1);
	ret = _mount_do_mount(type, flags, special, node, data,
			sizeof(special));
	free(data);
	return ret;
}

#ifdef MT_HFS
static int _mount_callback_hfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct hfs_args hfs;
	void * data = &hfs;

	memset(&hfs, 0, sizeof(hfs));
	if((hfs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	/* XXX does not support options at the moment */
	type = MT_HFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(hfs));
	free(hfs.fspec);
	return ret;
}
#endif

#ifdef MT_ISO9660
static int _mount_callback_iso9660(char const * type, int flags,
		char const * special, char const * node)
{
	struct iso_args iso9660;
	void * data = &iso9660;

	memset(&iso9660, 0, sizeof(iso9660));
	iso9660.fspec = special;
	/* FIXME actually parse options */
	type = MT_ISO9660;
	return _mount_do_mount(type, flags, special, node, data,
			sizeof(iso9660));
}
#endif

#ifdef MT_MFS
static int _mount_callback_mfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct mfs_args mfs;
	void * data = &mfs;

	memset(&mfs, 0, sizeof(mfs));
	if((mfs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	/* FIXME actually parse options */
	mfs.size = (2 << 24);
	type = MT_MFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(mfs));
	free(mfs.fspec);
	return ret;
}
#endif

#ifdef MT_NFS
static int _mount_callback_nfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct nfs_args nfs;
	void * data = &nfs;
	char * p;
	char * q;

	memset(&nfs, 0, sizeof(nfs));
	if(special == NULL || strchr(special, ':') == NULL)
	{
		errno = EINVAL;
		return -_mount_error(node, 1);
	}
	if((p = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	q = strchr(p, ':');
	*(q++) = '\0';
	/* FIXME untested */
	nfs.version = NFS_ARGSVERSION;
	nfs.hostname = p;
	nfs.fh = (unsigned char *)q;
	/* FIXME implement the rest */
	type = MT_NFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(nfs));
	free(q);
	return ret;
}
#endif

#ifdef MT_NTFS
static int _mount_callback_ntfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct ntfs_args ntfs;
	void * data = &ntfs;
	struct stat st;

	memset(&ntfs, 0, sizeof(ntfs));
	if((ntfs.fspec = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	if(stat(node, &st) == 0)
	{
		ntfs.uid = st.st_uid;
		ntfs.gid = st.st_gid;
		ntfs.mode = ~st.st_mode & 0777;
	}
	/* FIXME actually parse options */
	type = MT_NTFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(ntfs));
	free(ntfs.fspec);
	return ret;
}
#endif

#ifdef MT_NULLFS
static int _mount_callback_nullfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct null_args nullfs;
	void * data = &nullfs;

	memset(&nullfs, 0, sizeof(nullfs));
	if((nullfs.la.target = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	type = MT_NULLFS;
	ret = _mount_do_mount(type, flags, special, node, data, sizeof(nullfs));
	free(nullfs.la.target);
	return ret;
}
#endif

#ifdef MT_PROCFS
static int _mount_callback_procfs(char const * type, int flags,
		char const * special, char const * node)
{
	struct procfs_args procfs;
	void * data = &procfs;

	memset(&procfs, 0, sizeof(procfs));
	procfs.version = PROCFS_ARGS_VERSION;
	type = MT_PROCFS;
	return _mount_do_mount(type, flags, special, node, data,
			sizeof(procfs));
}
#endif

#ifdef MT_TMPFS
static int _mount_callback_tmpfs(char const * type, int flags,
		char const * special, char const * node)
{
	struct tmpfs_args tmpfs;
	void * data = &tmpfs;
	struct stat st;

	memset(&tmpfs, 0, sizeof(tmpfs));
	tmpfs.ta_version = TMPFS_ARGS_VERSION;
	if(stat(node, &st) == 0)
	{
		tmpfs.ta_root_uid = st.st_uid;
		tmpfs.ta_root_gid = st.st_gid;
		tmpfs.ta_root_mode = st.st_mode;
	}
	/* FIXME actually parse options */
	tmpfs.ta_nodes_max = 1024;
	tmpfs.ta_root_mode = 0755;
	type = MT_TMPFS;
	return _mount_do_mount(type, flags, special, node, data, sizeof(tmpfs));
}
#endif

#ifdef MT_UNIONFS
static int _mount_callback_unionfs(char const * type, int flags,
		char const * special, char const * node)
{
	int ret;
	struct union_args unionfs;
	void * data = &unionfs;

	memset(&unionfs, 0, sizeof(unionfs));
	if((unionfs.target = strdup(special)) == NULL)
		return -_mount_error(node, 1);
	/* FIXME actually parse options */
	type = MT_UNIONFS;
	ret = _mount_do_mount(type, flags, special, node, data,
			sizeof(unionfs));
	free(unionfs.target);
	return ret;
}
#endif


/* usage */
static int _usage(void)
{
	size_t i;
	char const * sep = " ";

	fputs("Usage: mount [-a][-t type]\n"
"       mount [-f] special | node\n"
"       mount [-f][-u][-o options] special node\n", stderr);
	fputs("\nOptions supported:", stderr);
	for(i = 0; _mount_options[i].name != NULL; i++)
		if(_mount_options[i].flags >= 0)
		{
			fprintf(stderr, "%s%s", sep, _mount_options[i].name);
			sep = ",";
		}
	fputc('\n', stderr);
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
	while((o = getopt(argc, argv, "afo:t:u")) != -1)
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
			case 'u':
				prefs.flags |= PREFS_u;
				break;
			default:
				return _usage();
		}
	if(optind + 2 == argc)
		special = argv[optind++];
	if(optind + 1 == argc)
		node = argv[optind];
	else if(optind != argc)
		return _usage();
	return (_mount(&prefs, special, node) == 0) ? 0 : 2;
}
