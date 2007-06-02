/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#ifndef LIBC_KERNEL_NETBSD_UTMPX_H
# define LIBC_KERNEL_NETBSD_UTMPX_H

# define _UTMPX_ID_SIZE		4
# define _UTMPX_LINE_SIZE	32
# define _UTMPX_USER_SIZE	32


/* types */
# ifndef id_t
#  define id_t id_t
typedef unsigned int id_t;
# endif
# ifndef pid_t
#  define pid_t pid_t
typedef signed int pid_t;
# endif
# ifndef suseconds_t
#  define suseconds_t suseconds_t
typedef int suseconds_t;
# endif
#ifndef timeval
# define timeval timeval
struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
};
#endif

struct utmpx
{
	char ut_user[_UTMPX_USER_SIZE];
	char ut_id[_UTMPX_ID_SIZE];
	char ut_line[_UTMPX_LINE_SIZE];
	char _padding0[258];
	unsigned short int ut_type; /* XXX uint16_t */
	pid_t ut_pid;
	char _padding1[4];
	char _padding2[0]; /* FIXME struct sockaddr_storage */
	struct timeval ut_tv;
	char _padding3[40];
};

#endif /* !LIBC_KERNEL_NETBSD_UTMPX_H */
