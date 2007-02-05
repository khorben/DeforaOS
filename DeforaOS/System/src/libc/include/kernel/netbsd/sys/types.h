/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#ifndef LIBC_KERNEL_NETBSD_SYS_TYPES_H
# define LIBC_KERNEL_NETBSD_SYS_TYPES_H


/* types */
# ifndef blkcnt_t
#  define blkcnt_t blkcnt_t
typedef unsigned long long blkcnt_t;
# endif
# ifndef clock_t
#  define clock_t clock_t
typedef unsigned int clock_t;
# endif
# ifndef dev_t
#  define dev_t dev_t
typedef int dev_t;
# endif
# ifndef fsblkcnt_t
#  define fsblkcnt_t fsblkcnt_t
typedef unsigned long long fsblkcnt_t;
# endif
# ifndef fsfilcnt_t
#  define fsfilcnt_t fsfilcnt_t
typedef unsigned long long fsfilcnt_t;
# endif
# ifndef off_t
#  define off_t off_t
typedef long long off_t;
# endif
# ifndef time_t
#  define time_t time_t
typedef unsigned int time_t;
# endif

#endif /* !LIBC_KERNEL_NETBSD_SYS_TYPES_H */
