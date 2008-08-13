/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define PACKAGE	"linuxrc"

#define CDROM_PATH	"/mnt/cdrom"
#define INIT_PATH	CDROM_PATH "/sbin/init"


/* linuxrc_error */
static int _linuxrc_error(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* main */
int main(int argc, char * argv[])
{
	size_t i;
	char dev_cdrom[] = "/dev/cdroms/cdromX";
	int found = 0;

	/* mount /proc */
	if(mount(NULL, "/proc", "proc", MS_NOEXEC | MS_NOSUID | MS_NODEV, NULL)
			!= 0)
		_linuxrc_error("/proc", 0);
	/* look for the installation CD-ROM */
	for(i = 0; i < 4; i++)
	{
		dev_cdrom[sizeof(dev_cdrom) - 2] = i + '0';
		if(mount(dev_cdrom, CDROM_PATH, "iso9660", MS_NOSUID | MS_NODEV
					| MS_RDONLY, NULL) != 0)
		{
			_linuxrc_error(dev_cdrom, 0);
			continue;
		}
		if(stat(INIT_PATH, NULL) != 0)
		{
			found = 1;
			break;
		}
		_linuxrc_error(INIT_PATH, 0);
		umount(dev_cdrom);
	}
	if(!found)
	{
		fputs(PACKAGE ": Could not find the installation CD-ROM\n",
				stderr);
		return 2;
	}
	return 0;
}
