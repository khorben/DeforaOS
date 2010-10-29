/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Splasher */
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



# include <string.h>
# include <errno.h>
#ifdef __NetBSD__
# include <sys/ioctl.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include "splasher.h"
#include "../config.h"


/* Splasher */
/* private */
/* variables */
static AppServer * _appserver;


/* public */
/* functions */
/* splasher */
int splasher(AppServerOptions options)
{
	if((_appserver = appserver_new(PACKAGE, options)) == NULL)
	{
		error_print(PACKAGE);
		return 1;
	}
	appserver_loop(_appserver);
	appserver_delete(_appserver);
	return 0;
}


/* interface */
/* Splasher_enable */
int32_t Splasher_enable(int32_t enabled)
{
	int ret = 0;

#ifdef __NetBSD__
# define WSDISPLAYIO_SSPLASH	0x8004575d
	int fd;
	char const filename[] = "/dev/ttyE0";

	if((fd = open(filename, O_WRONLY)) < 0)
		return -error_set_print(PACKAGE, 1, "%s: %s", filename,
				strerror(errno));;
	if(ioctl(fd, WSDISPLAYIO_SSPLASH, &enabled) < 0)
		ret = -error_set_print(PACKAGE, 1, "%s: %s", filename,
				strerror(errno));
	close(fd);
#else
	/* FIXME implement */
#endif
	return ret;
}


/* Splasher_progress */
int32_t Splasher_progress(uint32_t progress, char const * text)
{
	int ret = 0;

#ifdef __NetBSD__
# define WSDISPLAYIO_PROGRESS	0x8004575e
	int fd;
	char const filename[] = "/dev/ttyE0";

	if(progress > 100)
		return -error_set_print(PACKAGE, 1, "%s", strerror(ERANGE));
	if((fd = open(filename, O_WRONLY)) < 0)
		return -error_set_print(PACKAGE, 1, "%s: %s", filename,
				strerror(errno));;
	if(ioctl(fd, WSDISPLAYIO_PROGRESS, &progress) < 0)
		ret = -error_set_print(PACKAGE, 1, "%s: %s", filename,
				strerror(errno));
	close(fd);
#else
	if(progress > 100)
		return -error_set_print(PACKAGE, 1, "%s", strerror(ERANGE));
	/* FIXME implement */
#endif
	return ret;
}
