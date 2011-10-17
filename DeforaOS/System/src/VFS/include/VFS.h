/* $Id$ */



#ifndef VFS_VFS_H
# define VFS_VFS_H

# include <stdint.h>
# include <System.h>


/* types */
typedef Buffer * BUFFER;
typedef double * DOUBLE;
typedef float * FLOAT;
typedef int16_t INT16;
typedef int32_t INT32;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef String const * STRING;
typedef void VOID;

typedef BUFFER BUFFER_IN;

typedef DOUBLE DOUBLE_IN;

typedef FLOAT FLOAT_IN;
typedef INT32 INT32_IN;
typedef UINT32 UINT32_IN;
typedef STRING STRING_IN;

typedef Buffer * BUFFER_OUT;
typedef int32_t * INT32_OUT;
typedef uint32_t * UINT32_OUT;
typedef String ** STRING_OUT;

typedef Buffer * BUFFER_INOUT;
typedef int32_t * INT32_INOUT;
typedef uint32_t * UINT32_INOUT;
typedef String ** STRING_INOUT;


/* constants */
# define VFS_F_OK	0
# define VFS_R_OK	1
# define VFS_X_OK	2
# define VFS_W_OK	4
# define VFS_EPERM	1
# define VFS_ENOENT	2
# define VFS_EIO	5
# define VFS_ENXIO	6
# define VFS_EBADF	9
# define VFS_EACCES	13
# define VFS_EFAULT	14
# define VFS_EBUSY	16
# define VFS_EEXIST	17
# define VFS_EXDEV	18
# define VFS_ENODEV	19
# define VFS_ENOTDIR	20
# define VFS_EISDIR	21
# define VFS_EINVAL	22
# define VFS_ENFILE	23
# define VFS_EMFILE	24
# define VFS_ETXTBUSY	26
# define VFS_EFBIG	27
# define VFS_ENOSPC	28
# define VFS_EROFS	30
# define VFS_EMLINK	31
# define VFS_ENOTEMPTY	66
# define VFS_ENOSYS	78
# define VFS_ENOTSUP	86
# define VFS_EPROTO	96
# define VFS_SEEK_SET	0
# define VFS_SEEK_CUR	1
# define VFS_SEEK_END	2
# define VFS_O_RDONLY	0x0
# define VFS_O_WRONLY	0x1
# define VFS_O_RDWR	0x2
# define VFS_O_ACCMODE	0x4
# define VFS_O_CREAT	0x10
# define VFS_O_EXCL	0x20
# define VFS_O_TRUNC	0x40
# define VFS_O_NOCTTY	0x80
# define VFS_O_APPEND	0x100
# define VFS_O_DSYNC	0x200
# define VFS_O_NONBLOCK	0x400
# define VFS_O_RSYNC	0x800
# define VFS_O_SYNC	0x1000


/* calls */
INT32 VFS_access(STRING pathname, UINT32 mode);
INT32 VFS_chmod(STRING pathname, UINT32 mode);
INT32 VFS_chown(STRING pathname, UINT32 owner, UINT32 group);
INT32 VFS_close(INT32 fd);
INT32 VFS_closedir(INT32 dir);
INT32 VFS_dirfd(INT32 dir);
INT32 VFS_fchmod(INT32 fd, UINT32 mode);
INT32 VFS_fchown(INT32 fd, UINT32 owner, UINT32 group);
INT32 VFS_flock(INT32 fd, UINT32 operation);
INT32 VFS_lchown(STRING pathname, UINT32 owner, UINT32 group);
INT32 VFS_link(STRING name1, STRING name2);
INT32 VFS_lseek(INT32 fd, INT32 offset, INT32 whence);
INT32 VFS_mkdir(STRING pathname, UINT32 mode);
INT32 VFS_open(STRING pathname, UINT32 flags, UINT32 mode);
INT32 VFS_opendir(STRING pathname);
INT32 VFS_read(INT32 fd, BUFFER_OUT buf, UINT32 size);
INT32 VFS_readdir(INT32 dir, STRING_OUT de);
INT32 VFS_rename(STRING from, STRING to);
INT32 VFS_rewinddir(INT32 dir);
INT32 VFS_rmdir(STRING pathname);
INT32 VFS_symlink(STRING name1, STRING name2);
UINT32 VFS_umask(UINT32 mode);
INT32 VFS_unlink(STRING pathname);
INT32 VFS_write(INT32 fd, BUFFER buffer, UINT32 size);

#endif /* !VFS_VFS_H */
