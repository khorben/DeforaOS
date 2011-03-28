/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VFS */
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
/* TODO:
 * - implement root
 * - unify whence values */



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "vfs.h"
#include "common.c"
#include "../data/VFS.h"
#include "../config.h"


/* VFS */
/* private */
/* types */
typedef struct _VFSFile
{
	int32_t fd;
	DIR * dir;
} VFSFile;

typedef struct _VFSClient
{
	void * id;
	mode_t umask;
	VFSFile * files;
	size_t files_cnt;
} VFSClient;


/* variables */
static AppServer * _appserver;
static VFSClient * _clients;
static size_t _clients_cnt;


/* macros */
#if 1
# define VFS_STUB1(type, name, type1, arg1) \
	type VFS_ ## name(type1 arg1) \
{ \
	return name(arg1); \
}
#else /* FIXME check if the following approach works too */
# define VFS_STUB1(type, name, type1, arg1) \
	type (*VFS_ ## name)(type1 arg1) = name;
#endif

#define VFS_STUB2(type, name, type1, arg1, type2, arg2) \
	type VFS_ ## name(type1 arg1, type2 arg2) \
{ \
	return name(arg1, arg2); \
}

#define VFS_STUB3(type, name, type1, arg1, type2, arg2, type3, arg3) \
	type VFS_ ## name(type1 arg1, type2 arg2, type3 arg3) \
{ \
	return name(arg1, arg2, arg3); \
}


/* prototypes */
/* client management */
static void _client_init(void);
static void _client_destroy(void);

/* accessors */
static VFSClient * _client_get(void);
static mode_t _client_get_umask(void);
static int _client_set_umask(mode_t mask);

static int _client_check(int32_t fd);
static DIR * _client_check_dir(int32_t fd);

/* useful */
static VFSClient * _client_add(void);
static int _client_add_file(int32_t fd, DIR * dir);
static int _client_remove_file(int32_t fd);


/* public */
/* functions */
/* vfs */
int vfs(AppServerOptions options, mode_t mask, char const * root)
	/* FIXME implement root */
{
	if((_appserver = appserver_new("VFS", options)) == NULL)
	{
		error_print(PACKAGE);
		return 1;
	}
	umask(mask);
	_client_init();
	appserver_loop(_appserver);
	_client_destroy();
	appserver_delete(_appserver);
	return 0;
}


/* stubs */
VFS_STUB2(int32_t, chmod, String const *, path, uint32_t, mode)
VFS_STUB3(int32_t, chown, String const *, path, uint32_t, owner, uint32_t,
		group)
VFS_STUB3(int32_t, lchown, String const *, path, uint32_t, owner, uint32_t,
		group)
VFS_STUB2(int32_t, link, String const *, name1, String const *, name2)
VFS_STUB2(int32_t, rename, String const *, from, String const *, to)
VFS_STUB1(int32_t, rmdir, String const *, path)
VFS_STUB2(int32_t, symlink, String const *, name1, String const *, name2)
VFS_STUB1(int32_t, unlink, String const *, path)


/* interface */
/* VFS_access */
int32_t VFS_access(String const * path, uint32_t mode)
{
	int vfsmode;

	if((vfsmode = _vfs_flags(_vfs_flags_access, _vfs_flags_access_cnt,
					mode, 0)) < 0)
		return -1;
	return access(path, vfsmode);
}


/* VFS_close */
int32_t VFS_close(int32_t fd)
{
	int32_t ret;

	if(!_client_check(fd))
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	if((ret = close(fd)) == 0)
		_client_remove_file(fd);
	return ret;
}


/* VFS_closedir */
int32_t VFS_closedir(int32_t dir)
{
	int32_t ret;
	DIR * d;

	if((d = _client_check_dir(dir)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, dir);
#endif
	if((ret = closedir(d)) == 0)
		_client_remove_file(dir);
	return ret;
}


/* VFS_dirfd */
int32_t VFS_dirfd(int32_t dir)
{
	if(_client_check_dir(dir) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, dir);
#endif
	return dir;
}


/* VFS_fchmod */
int32_t VFS_fchmod(int32_t fd, uint32_t mode)
{
	if(!_client_check(fd))
		return -1;
	return fchmod(fd, mode);
}


/* VFS_fchown */
int32_t VFS_fchown(int32_t fd, uint32_t owner, uint32_t group)
{
	if(!_client_check(fd))
		return -1;
	return fchown(fd, owner, group);
}


/* VFS_flock */
int32_t VFS_flock(int32_t fd, uint32_t operation)
{
	if(!_client_check(fd))
		return -1;
	return flock(fd, operation);
}


/* VFS_lseek */
int32_t VFS_lseek(int32_t fd, int32_t offset, int32_t whence)
	/* FIXME check types sizes */
{
	if(!_client_check(fd))
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d)\n", __func__, fd, offset,
			whence);
#endif
	if((whence = _vfs_flags(_vfs_flags_lseek, _vfs_flags_lseek_cnt, whence,
					0)) < 0)
		return -1;
	return lseek(fd, offset, whence);
}


/* VFS_mkdir */
int32_t VFS_mkdir(String const * path, uint32_t mode)
{
	mode_t mask;

	mask = _client_get_umask();
	return mkdir(path, mode & mask);
}


/* VFS_open */
int32_t VFS_open(String const * filename, uint32_t flags, uint32_t mode)
{
	int vfsflags;
	int mask;
	int fd;

	if((vfsflags = _vfs_flags(_vfs_flags_open, _vfs_flags_open_cnt, flags,
					0)) < 0)
		return -1;
	mask = _client_get_umask();
	fd = open(filename, vfsflags, mode & mask);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u, %u) => %d\n", __func__, filename,
			flags, mode, fd);
#endif
	if(fd < 0)
		return -1;
	if(_client_add_file(fd, NULL) != 0)
	{
		close(fd);
		return -1;
	}
	return fd;
}


/* VFS_opendir */
int32_t VFS_opendir(String const * filename)
{
	DIR * dir;
	int fd;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
#if defined(__sun__)
	if((fd = open(filename, O_RDONLY)) < 0 || (dir = fdopendir(fd)) == NULL)
	{
		if(fd >= 0)
			close(fd);
		return -1;
	}
#else
	if((dir = opendir(filename)) == NULL || (fd = dirfd(dir)) < 0)
	{
		if(dir != NULL)
			closedir(dir);
		return -1;
	}
#endif
	if(_client_add_file(fd, dir) != 0)
	{
		closedir(dir);
		return -1;
	}
	return fd;
}


/* VFS_read */
int32_t VFS_read(int32_t fd, Buffer * b, uint32_t size)
{
	int32_t ret;

	if(!_client_check(fd))
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %p, %u)\n", __func__, fd, (void*)b,
			size);
#endif
	if(buffer_set_size(b, size) != 0)
		return -1;
	ret = read(fd, buffer_get_data(b), size);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, buf, %u) => %d\n", __func__, fd, size,
			ret);
#endif
	if(buffer_set_size(b, (ret < 0) ? 0 : ret) != 0)
	{
		memset(buffer_get_data(b), 0, size);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %d\n", __func__, ret);
#endif
	return ret;
}


/* VFS_readdir */
int32_t VFS_readdir(int32_t dir, String ** string)
{
	DIR * d;
	struct dirent * de;

	if((d = _client_check_dir(dir)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %p)\n", __func__, dir, string);
#endif
	if((de = readdir(d)) == NULL
			|| (*string = string_new(de->d_name)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, \"%s\") => 0\n", __func__, dir, *string);
#endif
	return 0;
}


/* VFS_rewinddir */
int32_t VFS_rewinddir(int32_t dir)
{
	DIR * d;

	if((d = _client_check_dir(dir)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, dir);
#endif
	rewinddir(d);
	return 0;
}


/* VFS_umask */
uint32_t VFS_umask(uint32_t mask)
{
	mode_t ret;

	ret = _client_get_umask();
	_client_set_umask(mask); /* XXX may fail */
	return ret;
}


/* VFS_write */
int32_t VFS_write(int32_t fd, Buffer * b, uint32_t size)
{
	if(!_client_check(fd))
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, buf, %u)\n", __func__, fd, size);
#endif
	if(buffer_get_size(b) < size)
		return -1;
	return write(fd, buffer_get_data(b), size);
}


/* private */
/* functions */
/* client_init */
static void _client_init(void)
{
	_clients = NULL;
	_clients_cnt = 0;
}


/* client_destroy */
static void _client_destroy(void)
{
	free(_clients);
	_clients = NULL;
	_clients_cnt = 0;
}


/* accessors */
/* client_get */
static VFSClient * _client_get(void)
{
	void * id;
	size_t i;

	if((id = appserver_get_client_id(_appserver)) == NULL)
	{
		error_print(PACKAGE);
		return NULL;
	}
	for(i = 0; i < _clients_cnt; i++)
		if(_clients[i].id == id)
			return &_clients[i];
	return NULL;
}


/* client_get_umask */
static mode_t _client_get_umask(void)
{
	VFSClient * p;
	mode_t omask;

	if((p = _client_get()) != NULL)
		return p->umask;
	omask = umask(0); /* obtain the default umask and restore it */
	umask(omask);
	return omask;
}


/* client_set_umask */
static int _client_set_umask(mode_t mask)
{
	VFSClient * p;

	if((p = _client_add()) == NULL)
		return 1;
	p->umask = mask;
	return 0;
}


/* client_check */
static int _client_check(int32_t fd)
{
	VFSClient * client;
	size_t i;

	if((client = _client_get()) == NULL)
		return 0;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	for(i = 0; i < client->files_cnt; i++)
		if(client->files[i].fd == fd)
			return (client->files[i].dir == NULL) ? 1 : 0;
	return 0;
}


/* client_check_dir */
static DIR * _client_check_dir(int32_t fd)
{
	VFSClient * client;
	size_t i;

	if((client = _client_get()) == NULL)
		return NULL;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	for(i = 0; i < client->files_cnt; i++)
		if(client->files[i].fd == fd)
			return client->files[i].dir;
	return NULL;
}


/* useful */
/* client_add */
static VFSClient * _client_add(void)
{
	VFSClient * p;
	void * id;

	if((p = _client_get()) != NULL)
		return p;
	if((id = appserver_get_client_id(_appserver)) == NULL)
	{
		error_print(PACKAGE);
		return NULL;
	}
	if((p = realloc(_clients, sizeof(*p) * (_clients_cnt + 1))) == NULL)
	{
		error_set_print(PACKAGE, 1, "%s", strerror(errno));
		return NULL;
	}
	_clients = p;
	p = &_clients[_clients_cnt++];
	p->id = id;
	p->umask = umask(0);
	umask(p->umask); /* restore umask */
	p->files = NULL;
	p->files_cnt = 0;
	return p;
}


/* client_add_file */
static int _client_add_file(int32_t fd, DIR * dir)
{
	VFSClient * client;
	VFSFile * p;

	if((client = _client_add()) == NULL)
		return 1;
	if((p = realloc(client->files, sizeof(*p) * (client->files_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	client->files = p;
	p = &client->files[client->files_cnt++];
	p->fd = fd;
	p->dir = dir;
	return 0;
}


/* client_remove_file */
static int _client_remove_file(int32_t fd)
{
	VFSClient * client;
	size_t i;
	VFSFile * p;

	if(fd < 0) /* XXX should never happen */
		return error_set_print(PACKAGE, 1, "%s", strerror(EINVAL));
	if((client = _client_get()) == NULL)
		return 1;
	for(i = 0; i < client->files_cnt; i++)
		if(client->files[i].fd == fd)
			break;
	if(i == client->files_cnt)
		return 0;
	p = &client->files[i];
	memmove(p, p + 1, (--client->files_cnt - i) * sizeof(*p));
	if((p = realloc(client->files, sizeof(*p) * client->files_cnt))
			!= NULL || client->files_cnt == 0)
		client->files = p; /* we can ignore errors */
	return 0;
}
