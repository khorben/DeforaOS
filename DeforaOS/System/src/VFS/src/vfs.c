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
/* TODO:
 * - implement root
 * - check fd is valid for respective connections
 * - unify whence values */



/* XXX ugly work-around to compile */
#define lseek VFS_lseek
#define read VFS_read
#define write VFS_write
#include <sys/types.h>
#include <unistd.h>
#undef lseek
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
static int (*old_fchmod)(int fd, mode_t mode);
static int (*old_fchown)(int fd, uid_t owner, gid_t group);
static off_t (*old_lseek)(int fd, off_t offset, int whence);
static int (*old_open)(char const * path, int flags, mode_t mode);
static ssize_t (*old_read)(int fd, void * buf, size_t count);
static ssize_t (*old_write)(int fd, void const * buf, size_t count);


/* public */
/* functions */
static int _vfs(char const * root)
	/* FIXME implement root */
{
#ifdef __Linux__
	static const char libc[] = "/lib/libc.so.6";
#else
	static const char libc[] = "/lib/libc.so";
#endif
	void * hdl;
	Event * event;
	AppServer * appserver;

	if((hdl = dlopen(libc, RTLD_LAZY)) == NULL)
		exit(1);
	if((old_close = dlsym(hdl, "close")) == NULL
			|| (old_fchmod = dlsym(hdl, "fchmod")) == NULL
			|| (old_fchown = dlsym(hdl, "fchown")) == NULL
			|| (old_lseek = dlsym(hdl, "lseek")) == NULL
			|| (old_open = dlsym(hdl, "open")) == NULL
			|| (old_read = dlsym(hdl, "read")) == NULL
			|| (old_write = dlsym(hdl, "write")) == NULL)
		exit(1);
	dlclose(hdl);
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


/* fchmod */
int32_t fchmod(int32_t fd, uint32_t mode)
{
	if(fd < VFS_OFF)
		return old_fchmod(fd, mode);
	return old_fchmod(fd - VFS_OFF, mode);
}


/* fchown */
int32_t fchown(int32_t fd, uint32_t owner, uint32_t group)
{
	if(fd < VFS_OFF)
		return old_fchown(fd, owner, group);
	return old_fchown(fd - VFS_OFF, owner, group);
}


/* lseek */
int32_t lseek(int32_t fd, int32_t offset, int32_t whence)
	/* FIXME unify whence, check types sizes */
{
	if(fd < VFS_OFF)
		return old_lseek(fd, offset, whence);
	return old_lseek(fd - VFS_OFF, offset, whence);
}


/* open */
int32_t open(char const * filename, uint32_t flags, uint32_t mode)
{
	int ret;

	ret = old_open(filename, flags, mode);
	/* FIXME actually register this fd as for this connection */
#ifdef DEBUG
	fprintf(stderr, "VFS: open(%s, %u, %u) %d\n", filename, flags, mode,
			ret);
#endif
	if(ret < 0)
		return -1;
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
