/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
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



#ifndef LIBC_SYS_IPC_H
# define LIBC_SYS_IPC_H


/* types */
# ifndef id_t
#  define id_t id_t
typedef unsigned int id_t;
# endif
# ifndef key_t
#  define key_t key_t
typedef long key_t;
# endif
# ifndef mode_t
#  define mode_t mode_t
typedef unsigned int mode_t;
# endif

# ifndef gid_t
#  define gid_t gid_t
typedef id_t gid_t;
# endif
# ifndef uid_t
#  define uid_t uid_t
typedef id_t uid_t;
# endif


/* constants */
# define IPC_PRIVATE	((key_t)0)
# define IPC_CREAT	01000
# define IPC_EXCL	02000
# define IPC_NOWAIT	04000

# define IPC_RMID	0
# define IPC_SET	1
# define IPC_STAT	2


/* functions */
key_t ftok(const char *, int);

#endif /* !LIBC_SYS_IPC_H */
