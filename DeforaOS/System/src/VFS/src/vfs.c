/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VFS */
/* VFS is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * VFS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with VFS; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



/* XXX ugly work-around to compile */
#define read VFS_read
#define write VFS_write
#include <unistd.h>
#undef read
#undef write
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <System.h>
#include "../config.h"


/* VFS */
/* private */
/* constants */
#define VFS_OFF		1024

/* variables */
static int (*old_close)(int fd);
static int (*old_open)(char const * path, int flags, mode_t mode);
static ssize_t (*old_read)(int fd, void * buf, size_t count);
static ssize_t (*old_write)(int fd, void const * buf, size_t count);

/* public */
/* functions */
static int _vfs(char const * root)
	/* FIXME implement root */
{
	static const char libc[] = "/lib/libc.so";
	void * hdl;
	Event * event;
	AppServer * appserver;

	if((hdl = dlopen(libc, RTLD_LAZY)) == NULL)
		exit(1);
	old_close = dlsym(hdl, "close");
	old_open = dlsym(hdl, "open");
	old_read = dlsym(hdl, "read");
	old_write = dlsym(hdl, "write");
	dlclose(hdl);
	if(old_close == NULL || old_open == NULL || old_read == NULL
			|| old_write == NULL)
		exit(1);
	if((event = event_new()) == NULL)
		return error_print(PACKAGE);
	if((appserver = appserver_new_event("VFS", ASO_LOCAL, event)) == NULL)
	{
		error_print(PACKAGE);
		event_delete(event);
		return 1;
	}
	event_loop(event);
	appserver_delete(appserver);
	event_delete(event);
	return 0;
}


/* close */
int32_t close(int32_t fd)
{
	/* FIXME actually check if fd is valid for this connection */
	if(fd < VFS_OFF)
		return old_close(fd);
#ifdef DEBUG
	fprintf(stderr, "VFS: close(%d)\n", fd - VFS_OFF);
#endif
	return old_close(fd - VFS_OFF);
}


/* open */
int32_t open(char const * filename, uint32_t flags, uint32_t mode)
{
	int ret;

	if((ret = old_open(filename, flags, mode)) < 0)
		return -1;
	/* FIXME actually register this fd as for this connection */
#ifdef DEBUG
	fprintf(stderr, "VFS: open(%s, %u, %u) %d\n", filename, flags, mode,
			ret);
#endif
	return ret + VFS_OFF;
}


/* read */
int32_t read(int fd, Buffer * b, uint32_t count)
{
	ssize_t ret;

	/* FIXME actually check if fd is valid for this connection */
	if(fd < VFS_OFF)
		return old_read(fd, b, count);
	if(buffer_set_size(b, count) != 0)
		return -1;
	ret = old_read(fd - VFS_OFF, buffer_get_data(b), count);
#ifdef DEBUG
	fprintf(stderr, "VFS: read(%d, buf, %u) %zd\n", fd - VFS_OFF, count,
			ret);
#endif
	if(buffer_set_size(b, ret < 0 ? 0 : ret) != 0)
	{
		memset(buffer_get_data(b), 0, count);
		return -1;
	}
	return ret;
}


/* write */
int32_t write(int fd, Buffer * b, uint32_t count)
{
	/* FIXME actually check if fd is valid for this connection */
	if(fd < VFS_OFF)
		return old_write(fd, b, count);
#ifdef DEBUG
	fprintf(stderr, "VFS: write(%d, buf, %u)\n", fd, count);
#endif
	if(buffer_get_size(b) != count)
		return -1;
	return old_write(fd - VFS_OFF, buffer_get_data(b), count);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: VFS [-r root]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * root = "/";

	while(getopt(argc, argv, "r:") != -1)
		switch(o)
		{
			case 'r':
				root = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _vfs(root) == 0 ? 0 : 2;
}
