/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
 * - handle errno */



#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <System.h>
#include "../src/common.c"


/* libvfs */
/* private */
/* types */
typedef struct _VFSDIR
{
	DIR * dir;
	int32_t fd;
} VFSDIR;


/* constants */
#define PROGNAME	"libVFS"
#define VFS_OFF		1024

static char const _vfs_root[] = "Videos/";
#define VFS_ROOT_SIZE (sizeof(_vfs_root) - 1)


/* variables */
static AppClient * _appclient = NULL;

/* local functions */
static int (*old_access)(char const * path, int mode);
static int (*old_chmod)(char const * path, mode_t mode);
static int (*old_chown)(char const * path, uid_t uid, gid_t gid);
static int (*old_close)(int fd);
static int (*old_closedir)(DIR * dir);
#ifndef dirfd
static int (*old_dirfd)(DIR * dir);
#endif
#if 0
static int (*old_fstat)(int fd, struct stat * st);
#endif
static int (*old_lchown)(char const * path, uid_t uid, gid_t gid);
static off_t (*old_lseek)(int fd, off_t offset, int whence);
static int (*old_mkdir)(char const * path, mode_t mode);
static void * (*old_mmap)(void * addr, size_t len, int prot, int flags, int fd,
		off_t offset);
static int (*old_open)(char const * path, int flags, mode_t mode);
static DIR * (*old_opendir)(char const * path);
static ssize_t (*old_read)(int fd, void * buf, size_t count);
static struct dirent * (*old_readdir)(DIR * dir);
static int (*old_rename)(char const * from, char const * to);
static void (*old_rewinddir)(DIR * dir);
static int (*old_rmdir)(char const * path);
static int (*old_symlink)(char const * name1, char const * name2);
static mode_t (*old_umask)(mode_t mode);
static int (*old_unlink)(char const * path);
static ssize_t (*old_write)(int fd, void const * buf, size_t count);


/* prototypes */
static void _libvfs_init(void);


/* functions */
static void _libvfs_init(void)
{
	static void * hdl = NULL;
	static char libc[] = "/lib/libc.so";
	static char libc6[] = "/lib/libc.so.6";

	if(hdl != NULL)
		return;
	if((hdl = dlopen(libc, RTLD_LAZY)) == NULL
			&& (hdl = dlopen(libc6, RTLD_LAZY)) == NULL)
	{
		fprintf(stderr, "%s: %s\n", PROGNAME, dlerror());
		exit(1);
	}
	if((old_access = dlsym(hdl, "access")) == NULL
			|| (old_chmod = dlsym(hdl, "chmod")) == NULL
			|| (old_chown = dlsym(hdl, "chown")) == NULL
			|| (old_close = dlsym(hdl, "close")) == NULL
			|| (old_closedir = dlsym(hdl, "closedir")) == NULL
#ifndef dirfd
			|| (old_dirfd = dlsym(hdl, "dirfd")) == NULL
#endif
			|| (old_lchown = dlsym(hdl, "lchown")) == NULL
			|| (old_lseek = dlsym(hdl, "lseek")) == NULL
			|| (old_mkdir = dlsym(hdl, "mkdir")) == NULL
			|| (old_mmap = dlsym(hdl, "mmap")) == NULL
			|| (old_open = dlsym(hdl, "open")) == NULL
			|| (old_opendir = dlsym(hdl, "opendir")) == NULL
			|| (old_read = dlsym(hdl, "read")) == NULL
			|| (old_readdir = dlsym(hdl, "readdir")) == NULL
			|| (old_rewinddir = dlsym(hdl, "rewinddir")) == NULL
			|| (old_rename = dlsym(hdl, "rename")) == NULL
			|| (old_rmdir = dlsym(hdl, "rmdir")) == NULL
			|| (old_symlink = dlsym(hdl, "symlink")) == NULL
			|| (old_umask = dlsym(hdl, "umask")) == NULL
			|| (old_unlink = dlsym(hdl, "unlink")) == NULL
			|| (old_write = dlsym(hdl, "write")) == NULL)
		{
			fprintf(stderr, "%s: %s\n", PROGNAME, dlerror());
			exit(1);
		}
	dlclose(hdl);
	if((_appclient = appclient_new("VFS")) == NULL)
	{
		error_print(PROGNAME);
		exit(1);
	}
}


/* public */
/* interface */
/* access */
int access(const char * path, int mode)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_access(path, mode);
	if((mode = _vfs_flags(_vfs_flags_access, _vfs_flags_access_cnt, mode,
					1)) < 0)
	{
		errno = EINVAL;
		return -1;
	}
	if(appclient_call(_appclient, &ret, "access", path, mode) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: access(\"%s\", %o) => %d\n", path, mode, ret);
#endif
	return ret;
}


/* chmod */
int chmod(char const * path, mode_t mode)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_chmod(path, mode);
	if(appclient_call(_appclient, &ret, "chmod", path, mode) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: chmod(\"%s\", %o) => %d\n", path, mode, ret);
#endif
	return ret;
}


/* chown */
int chown(char const * path, uid_t uid, gid_t gid)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_chown(path, uid, gid);
	if(appclient_call(_appclient, &ret, "chown", path, uid, gid) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: chown(\"%s\", %d, %d) => %d\n", path, uid, gid,
			ret);
#endif
	return ret;
}


/* close */
int close(int fd)
{
	int ret;

	_libvfs_init();
	if(fd < VFS_OFF)
		return old_close(fd);
	if(appclient_call(_appclient, &ret, "close", fd - VFS_OFF) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: close(%d) => %d\n", fd - VFS_OFF, ret);
#endif
	return ret;
}


/* closedir */
int closedir(DIR * dir)
{
	int ret;
#ifndef dirfd
	VFSDIR * d = (VFSDIR*)dir;
#endif
	int fd;

	_libvfs_init();
#ifndef dirfd
	fd = d->fd;
	if(d->dir != NULL)
		ret = old_closedir(d->dir);
#else
	fd = dirfd(dir) - VFS_OFF;
	if(fd < 0)
		return old_closedir(dir);
#endif
	else if(appclient_call(_appclient, &ret, "closedir", fd) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: closedir(%p) => %d\n", dir, ret);
#endif
#ifndef dirfd
	free(d);
#else
	/* XXX not really closed because the fd is wrong */
	dirfd(dir) = -1;
	old_closedir(dir);
#endif
	return ret;
}


/* dirfd */
#ifndef dirfd /* XXX NetBSD... */
int dirfd(DIR * dir)
{
	int ret;
	VFSDIR * d = (VFSDIR*)dir;

	_libvfs_init();
	if(d->dir != NULL)
		ret = old_dirfd(d->dir);
	else if(appclient_call(_appclient, &ret, "dirfd", d->fd) != 0)
		return -1;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: dirfd(%p) => %d\n", dir, ret);
# endif
	if(ret < 0)
		return -1;
	return ret + VFS_OFF;
}
#endif


/* fstat */
#if 0
/* FIXME disabled for now (infinite loop on NetBSD) */
int fstat(int fd, struct stat * st)
{
	int ret = -1;

	_libvfs_init();
	if(fd < VFS_OFF)
		return old_fstat(fd, st);
	errno = ENOSYS;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: fstat(%d) => %d\n", fd - VFS_OFF, ret);
#endif
	return ret;
}
#endif


/* lchown */
int lchown(char const * path, uid_t uid, gid_t gid)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_lchown(path, uid, gid);
	if(appclient_call(_appclient, &ret, "lchown", path, uid, gid) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: lchown(\"%s\", %d, %d) => %d\n", path, uid, gid,
			ret);
#endif
	return ret;
}


/* lseek */
off_t lseek(int fd, off_t offset, int whence)
{
	int ret;

	_libvfs_init();
	if(fd < VFS_OFF)
		return old_lseek(fd, offset, whence);
	if((whence = _vfs_flags(_vfs_flags_lseek, _vfs_flags_lseek_cnt, whence,
					1)) < 0)
	{
		errno = EINVAL;
		return -1;
	}
	if(appclient_call(_appclient, &ret, "lseek", fd - VFS_OFF, offset,
				whence) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: lseek(%d, %zd, %d) => %d\n", fd - VFS_OFF,
			offset, whence, ret);
#endif
	return ret;
}


/* mkdir */
int mkdir(char const * path, mode_t mode)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_mkdir(path, mode);
	if(appclient_call(_appclient, &ret, "mkdir", path, mode) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: mkdir(\"%s\", %d) => %d\n", path, mode, ret);
#endif
	return ret;
}


/* mmap */
void * mmap(void * addr, size_t len, int prot, int flags, int fd,
		off_t offset)
{
	_libvfs_init();
	if(fd < VFS_OFF)
		return old_mmap(addr, len, prot, flags, fd, offset);
	errno = ENODEV;
	return MAP_FAILED;
}


/* open */
int open(const char * path, int flags, ...)
{
	int ret;
	int vfsflags;
	mode_t mode = 0;
	va_list ap;

	_libvfs_init();
	if(flags & O_CREAT)
	{
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_open(path, flags, mode);
	if((vfsflags = _vfs_flags(_vfs_flags_open, _vfs_flags_open_cnt,
					flags, 1)) < 0)
	{
		errno = EINVAL;
		return -1;
	}
	if(appclient_call(_appclient, &ret, "open", path, vfsflags, mode) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: open(\"%s\", %d, %#o) => %d\n", path, flags,
			mode, ret);
#endif
	if(ret < 0)
		return ret;
	return ret + VFS_OFF;
}


/* opendir */
DIR * opendir(char const * path)
{
#ifndef dirfd
	VFSDIR * dir;

	_libvfs_init();
	if((dir = malloc(sizeof(*dir))) == NULL)
		return NULL;
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
	{
		dir->dir = old_opendir(path);
		dir->fd = -1;
	}
	else
	{
		dir->dir = NULL;
		if(appclient_call(_appclient, &dir->fd, "opendir", path) != 0
				|| dir->fd < 0)
		{
			free(dir);
			return NULL;
		}
# ifdef DEBUG
		fprintf(stderr, "DEBUG: opendir(\"%s\") => %p %d\n", path, dir,
				dir->fd);
# endif
	}
	return (DIR*)dir;
#else
	DIR * dir;
	int fd;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_opendir(path);
	if((dir = old_opendir("/")) == NULL) /* XXX quite ugly */
		return NULL;
	if(appclient_call(_appclient, &fd, "opendir", path) != 0 || fd < 0)
	{
		old_closedir(dir);
		return NULL;
	}
# ifdef DEBUG
	fprintf(stderr, "DEBUG: opendir(\"%s\") => %p %d\n", path, dir, fd);
# endif
	dirfd(dir) = fd + VFS_OFF;
	return dir;
#endif
}


/* read */
ssize_t read(int fd, void * buf, size_t count)
{
	int32_t ret;
	Buffer * b;

	_libvfs_init();
	if(fd < VFS_OFF)
		return old_read(fd, buf, count);
	if((b = buffer_new(0, NULL)) == NULL)
		return -1;
	if(appclient_call(_appclient, &ret, "read", fd - VFS_OFF, b, count)
			!= 0)
	{
		buffer_delete(b);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: read(%d, buf, %zu) => %d\n", fd - VFS_OFF,
			count, ret);
#endif
	if(ret <= 0)
		return ret;
	memcpy(buf, buffer_get_data(b), ret);
	return ret;
}


/* readdir */
struct dirent * readdir(DIR * dir)
{
	static struct dirent de;
#ifndef dirfd
	VFSDIR * d = (VFSDIR*)dir;
#endif
	int fd;
	int res;
	String * filename = NULL;

	_libvfs_init();
#ifndef dirfd
	if(d->dir != NULL)
		return old_readdir(d->dir);
	fd = d->fd;
#else
	if(dirfd(dir) < VFS_OFF)
		return old_readdir(dir);
	fd = dirfd(dir) - VFS_OFF;
#endif
	if(appclient_call(_appclient, &res, "readdir", fd, &filename) != 0)
		return NULL;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: readdir(%p %d) => %d\n", dir, fd, res);
#endif
	if(res != 0)
	{
		string_delete(filename);
		return NULL;
	}
	memset(&de, 0, sizeof(de)); /* XXX set the other fields if possible */
	snprintf(de.d_name, sizeof(de.d_name), "%s", filename);
#ifdef _DIRENT_HAVE_D_RECLEN
	de.d_reclen = strlen(de.d_name);
#else
	de.d_namlen = strlen(de.d_name);
#endif
#ifdef DEBUG
	fprintf(stderr, "DEBUG: readdir(%p) => \"%s\"\n", dir, de.d_name);
#endif
	return &de;
}


/* rename */
int rename(char const * from, char const * to)
{
	int ret;
	int f;
	int t;

	_libvfs_init();
	f = strncmp(_vfs_root, from, VFS_ROOT_SIZE);
	t = strncmp(_vfs_root, to, VFS_ROOT_SIZE);
	if(f != 0 && t != 0)
		return old_rename(from, to);
	if((f == 0 && t != 0) || (f != 0 && t == 0))
	{
		errno = EXDEV;
		return -1;
	}
	if(appclient_call(_appclient, &ret, "rename", from, to) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: rename(\"%s\", \"%s\") => %d\n", from, to, ret);
#endif
	return ret;
}


/* rewinddir */
void rewinddir(DIR * dir)
{
#ifndef dirfd
	VFSDIR * d = (VFSDIR*)dir;
#endif
	int fd;

	_libvfs_init();
#ifndef dirfd
	fd = d->fd;
	if(d->dir != NULL)
		old_rewinddir(d->dir);
#else
	fd = dirfd(dir) - VFS_OFF;
	if(fd < 0)
		old_rewinddir(dir);
#endif
	else
	{
		/* XXX this call ignores errors */
		appclient_call(_appclient, NULL, "rewinddir", fd);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: rewinddir(%p)\n", dir);
#endif
	}
}


/* rmdir */
int rmdir(char const * path)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_rmdir(path);
	if(appclient_call(_appclient, &ret, "rmdir", path) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: rmdir(\"%s\") => %d\n", path, ret);
#endif
	return ret;
}


/* symlink */
int symlink(char const * name1, char const * name2)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, name2, VFS_ROOT_SIZE) != 0)
		return old_symlink(name1, name2);
	if(appclient_call(_appclient, &ret, "symlink", name1, name2) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: symlink(\"%s\", \"%s\") => %d\n", name1, name2,
			ret);
#endif
	return ret;
}


/* umask */
mode_t umask(mode_t mode)
	/* FIXME inherently incoherent: cannot return both old states */
{
	unsigned int ret;

	_libvfs_init();
#ifdef DEBUG
	fprintf(stderr, "DEBUG: umask(%o)\n", mode);
#endif
	/* FIXME what to do if it fails? */
	appclient_call(_appclient, (int*)&ret, "umask", mode);
	return umask(mode);
}


/* unlink */
int unlink(char const * path)
{
	int ret;

	_libvfs_init();
	if(strncmp(_vfs_root, path, VFS_ROOT_SIZE) != 0)
		return old_unlink(path);
	if(appclient_call(_appclient, &ret, "unlink", path) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: unlink(\"%s\") => %d\n", path, ret);
#endif
	return ret;
}


/* write */
ssize_t write(int fd, void const * buf, size_t count)
{
	int32_t ret;
	Buffer * b;

	_libvfs_init();
	if(fd < VFS_OFF)
		return old_write(fd, buf, count);
	if((b = buffer_new(count, buf)) == NULL)
		return -1;
	if(appclient_call(_appclient, &ret, "write", fd - VFS_OFF, b, count)
			!= 0)
	{
		buffer_delete(b);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: write(%d, buf, %zu) => %d\n", fd - VFS_OFF,
			count, ret);
#endif
	if(ret <= 0)
		return ret;
	return ret;
}
