/* $Id$ */
/* Copyright (c) 2007-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef KERNEL_FREEBSD_COMMON_H
# define KERNEL_FREEBSD_COMMON_H


# define SYS_exit		1
# define SYS_fork		2
# define SYS_read		3
# define SYS_write		4
# define SYS_open		5
# define SYS_close		6
# define SYS_wait4		7
# define SYS_link		9
# define SYS_unlink		10
# define SYS_chdir		12
# define SYS_fchdir		13
# define SYS_mknod		14
# define SYS_chmod		15
# define SYS_chown		16
# define SYS_brk		17
# define SYS_getpid		20
# define SYS_mount		21
# define SYS_unmount		22
# define SYS_setuid		23
# define SYS_getuid		24
# define SYS_geteuid		25
# define SYS_access		30
# define SYS_sync		36
# define SYS_kill		37
# define SYS_getppid		39
# define SYS_dup		41
# define SYS_pipe		42
# define SYS_getegid		43
# define SYS_getgid		47
# define SYS_ioctl		54
# define SYS_reboot		55
# define SYS_symlink		57
# define SYS_readlink		58
# define SYS_execve		59
# define SYS_umask		60
# define SYS_vfork		66
# define SYS_munmap		73
# define SYS_mprotect		74
# define SYS_getpgrp		81
# define SYS_setpgid		82
# define SYS_dup2		90
# define SYS_fcntl		92
# define SYS_select		93
# define SYS_fsync		95
# define SYS_setpriority	96
# define SYS_getpriority	100
# define SYS_gettimeofday	116
# define SYS_getrusage		117
# define SYS_fchown		123
# define SYS_fchmod		124
# define SYS_setreuid		126
# define SYS_setregid		127
# define SYS_rename		128
# define SYS_flock		131
# define SYS_mkfifo		132
# define SYS_mkdir		136
# define SYS_rmdir		137
# define SYS_utimes		138
# define SYS_setsid		147
# define SYS_uname		164
# define SYS_setgid		181
# define SYS_stat		188
# define SYS_fstat		189
# define SYS_lstat		190
# define SYS_getrlimit		194
# define SYS_setrlimit		195
# define SYS_getdents		196
# define SYS_mmap		197
# define SYS_lseek		199
# define SYS_mlock		203
# define SYS_munlock		204
# define SYS_poll		209
# define SYS_nanosleep		240
# define SYS_lchown		254
# define SYS__getcwd		326
# define SYS__statfs		396
# define SYS_sigaction		416
# define SYS_getcontext		421
# define SYS_setcontext		422
# define SYS_truncate		479
# define SYS_ftruncate		480
# define SYS_shmctl		512

#endif /* !KERNEL_FREEBSD_COMMON_H */
