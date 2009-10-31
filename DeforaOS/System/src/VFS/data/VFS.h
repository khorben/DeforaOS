/* $Id$ */



#ifndef VFS_H
# define VFS_H

# include <stdint.h>
# include <System.h>


/* types */
typedef Buffer * BUFFER;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef String const * STRING;

typedef BUFFER BUFFER_IN;
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


/* functions */
INT32 VFS_chmod(STRING pathname, UINT32 mode);
INT32 VFS_chown(STRING pathname, UINT32 owner, UINT32 group);
INT32 VFS_close(INT32 fd);
INT32 VFS_closedir(INT32 dir);
INT32 VFS_dirfd(INT32 dir);
INT32 VFS_fchmod(INT32 fd, UINT32 mode);
INT32 VFS_fchown(INT32 fd, UINT32 owner, UINT32 group);
INT32 VFS_flock(INT32 fd, UINT32 operation);
INT32 VFS_link(STRING name1, STRING name2);
INT32 VFS_mkdir(STRING pathname, UINT32 mode);
INT32 VFS_open(STRING pathname, UINT32 flags, UINT32 mode);
INT32 VFS_opendir(STRING pathname);
INT32 VFS_read(INT32 fd, BUFFER_OUT buf, UINT32 size);
INT32 VFS_rename(STRING from, STRING to);
INT32 VFS_rmdir(STRING pathname);
INT32 VFS_symlink(STRING name1, STRING name2);
UINT32 VFS_umask(UINT32 mode);
INT32 VFS_unlink(STRING pathname);
INT32 VFS_write(INT32 fd, BUFFER buffer, UINT32 size);

#endif /* !VFS_H */
