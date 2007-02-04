/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBC_SYS_STAT_H
# define LIBC_SYS_STAT_H

# include "compat/sys/stat.h"


/* constants */
# define S_IFMT		0170000
# define S_IFSOCK	0140000
# define S_IFLNK	0120000
# define S_IFREG	0100000
# define S_IFBLK	0060000
# define S_IFDIR	0040000
# define S_IFCHR	0020000
# define S_IFIFO	0010000
# define S_ISUID	0004000
# define S_ISGID	0002000
# define S_ISVTX	0001000
# define S_IRUSR	0000400
# define S_IWUSR	0000200
# define S_IXUSR	0000100
# define S_IRWXU	(S_IRUSR | S_IWUSR | S_IXUSR)
# define S_IRGRP	0000040
# define S_IWGRP	0000020
# define S_IXGRP	0000010
# define S_IRWXG	(S_IRGRP | S_IWGRP | S_IXGRP)
# define S_IROTH	0000004
# define S_IWOTH	0000002
# define S_IXOTH	0000001
# define S_IRWXO	(S_IROTH | S_IWOTH | S_IXOTH)


/* macros */
# define S_ISBLK(m)	(((m) & S_IFBLK) ? 1 : 0)
# define S_ISCHR(m)	(((m) & S_IFCHR) ? 1 : 0)
# define S_ISDIR(m)	(((m) & S_IFDIR) ? 1 : 0)
# define S_ISFIFO(m)	(((m) & S_IFIFO) ? 1 : 0)
# define S_ISREG(m)	(((m) & S_IFREG) ? 1 : 0)
# define S_ISLNK(m)	(((m) & S_IFLNK) ? 1 : 0)
# define S_ISSOCK(m)	(((m) & S_IFSOCK) ? 1 : 0)


/* functions */
int chmod(char const * name, mode_t mode);
int fchmod(int fd, mode_t mode);
int fstat(int fd, struct stat * st);
int lstat(char const * name, struct stat * st);
int mkdir(char const * name, mode_t mode);
int mkfifo(char const * name, mode_t mode);
int mknod(char const * name, mode_t mode, dev_t dev);
int stat(char const * name, struct stat * st);
mode_t umask(mode_t mask);

#endif /* !LIBC_SYS_STAT_H */
