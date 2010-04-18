/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* portability */
#if defined(MNT_RDONLY)	/* FreeBSD, NetBSD */
# include <isofs/cd9660/cd9660_mount.h>
# ifndef MOUNT_CD9660
#  define MT_ISO9660		"cd9660"
# else
#  define MT_ISO9660		MOUNT_CD9660
# endif
# ifndef MOUNT_PROCFS
#  define MT_PROCFS		"procfs"
# else
#  define MT_PROCFS		MOUNT_PROCFS
# endif
# ifndef MNT_NODEV
#  define MF_NODEV		NODEV
# else
#  define MF_NODEV		MNT_NODEV
# endif
# define MF_NOEXEC		MNT_NOEXEC
# define MF_NOSUID		MNT_NOSUID
# define MF_RDONLY		MNT_RDONLY
# if defined(__FreeBSD__) || \
	(defined(__NetBSD_Version__) && __NetBSD_Version__ < 499000000)
#  define mount(type, dir, flags, data, data_len) \
	mount(type, dir, flags, data)
# endif
#elif defined(MT_ISO9660)
struct iso_args
{
	char const * fspec;
};
#else
# define MF_ASYNC		MS_ASYNC
# define MF_NODEV		MS_NODEV
# define MF_NOEXEC		MS_NOEXEC
# define MF_NOSUID		MS_NOSUID
# define MF_RDONLY		MS_RDONLY
# define MT_ISO9660		"iso9660"
# define MT_PROCFS		"proc"
struct iso_args
{
	char const * fspec;
};
# define mount(type, dir, flags, data, data_len) \
	mount(data ? ((struct iso_args *)data)->fspec : NULL, dir, type, \
				flags, NULL)
# define unmount(dir, flags)	umount(dir)
#endif

/* constants */
#define PACKAGE			"linuxrc"

#define CDROM_PATH		"/mnt/cdrom"
#define DEV_CDROMX		"/dev/cdroms/cdromX"
#define INIT_PATH		CDROM_PATH "/sbin/init"
#define PROC_REAL_ROOT_DEV	"/proc/sys/kernel/real-root-dev"


/* linuxrc_error */
static int _linuxrc_error(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* linuxrc_mount_cdrom */
static int _linuxrc_mount_cdrom(char * source, char const * dir)
{
	struct iso_args ia;

	memset(&ia, 0, sizeof(ia));
	ia.fspec = source;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: mount -t %s %s %s\n", MT_ISO9660,
			((struct iso_args *)&ia)->fspec, dir);
#endif
	return mount(MT_ISO9660, dir, MF_NOSUID | MF_NODEV | MF_RDONLY, &ia,
			sizeof(ia));
}


/* main */
int main(void)
{
	size_t i;
	char dev_cdrom[] = DEV_CDROMX;
	struct stat st;
	int found = 0;
	FILE * fp;

	/* mount /proc */
	if(mount(MT_PROCFS, "/proc", MF_NOEXEC | MF_NOSUID | MF_NODEV, NULL, 0)
			!= 0)
		_linuxrc_error("/proc", 0);
	/* look for the installation CD-ROM */
	for(i = 0; i < 4; i++)
	{
		dev_cdrom[sizeof(dev_cdrom) - 2] = i + '0';
		if(_linuxrc_mount_cdrom(dev_cdrom, CDROM_PATH) != 0)
		{
			_linuxrc_error(dev_cdrom, 0);
			continue;
		}
		if(stat(INIT_PATH, &st) != 0)
		{
			found = 1;
			break;
		}
		_linuxrc_error(INIT_PATH, 0);
		unmount(dev_cdrom, 0); /* FIXME check the flags */
	}
	if(!found)
	{
		fputs(PACKAGE ": Could not find the bootable CD-ROM\n", stderr);
		return 2;
	}
	if((fp = fopen(PROC_REAL_ROOT_DEV, "w")) == NULL)
		return _linuxrc_error(PROC_REAL_ROOT_DEV, 0);
	if(fwrite("0x100\n", sizeof(char), 6, fp) != 6)
		_linuxrc_error(PROC_REAL_ROOT_DEV, 0);
	if(fclose(fp) != 0)
		_linuxrc_error(PROC_REAL_ROOT_DEV, 0);
	return 0;
}
