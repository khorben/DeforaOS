/* $Id$ */
/* Copyright (c) 2004-2012 Pierre Pronchery <khorben@defora.org> */
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



.text


/* FreeBSD */
#if defined(__FreeBSD__)
# include "kernel/freebsd/common.h"
# if defined(__amd64__)
#  include "kernel/freebsd/amd64/syscalls.S"
# elif defined(__i386__)
#  include "kernel/freebsd/i386/syscalls.S"
# else /* !amd64 && !i386 */
#  warning Unsupported FreeBSD architecture
# endif

/* Linux */
#elif defined(__linux__)
# include "kernel/linux/common.h"
# if defined(__amd64__)
#  include "kernel/linux/amd64/syscalls.S"
# elif defined(__arm__)
#  include "kernel/linux/arm/syscalls.S"
# elif defined(__i386__)
#  include "kernel/linux/i386/syscalls.S"
# else /* !__amd64__ && !__arm__ && !__i386__ */
#  warning Unsupported Linux architecture
# endif

/* MacOS X */
#elif defined(__APPLE__)
# include "kernel/darwin/common.h"
# if defined(__amd64__)
#  include "kernel/darwin/amd64/syscalls.S"
# else /* !__amd64__ */
#  warning Unsupported Darwin architecture
#endif

/* NetBSD */
#elif defined(__NetBSD__)
# include "kernel/netbsd/common.h"
# if defined(__amd64__)
#  include "kernel/netbsd/amd64/syscalls.S"
# elif defined(__arm__)
#  include "kernel/netbsd/arm/syscalls.S"
# elif defined(__i386__)
#  include "kernel/netbsd/i386/syscalls.S"
# elif defined(__sparc64__)
#  include "kernel/netbsd/sparc64/syscalls.S"
# elif defined(__sparc__)
#  include "kernel/netbsd/sparc/syscalls.S"
# else /* !__amd64__ && !__arm__ && !__i386__ && !__sparc__ && !__sparc64__ */
#  warning Unsupported NetBSD architecture
#  define SYSCALL(name)
# endif
# include "kernel/netbsd/common.S"

/* OpenBSD */
#elif defined(__OpenBSD__)
# include "kernel/openbsd/common.h"
# if defined(__arm__)
#  include "kernel/openbsd/arm/syscalls.S"
# elif defined(__i386__)
#  include "kernel/openbsd/i386/syscalls.S"
# else /* !__arm__ && !__i386__ */
#  warning Unsupported OpenBSD architecture
# endif

/* Solaris */
#elif defined(__sun__)
# include "kernel/solaris/common.h"
# if defined(__sparc__)
#  include "kernel/solaris/sparc/syscalls.S"
# else
#  warning Unsupported Solaris architecture
# endif

/* Whitix */
#elif defined(__Whitix__)
# include "kernel/whitix/common.h"
# if defined(__i386__)
#  include "kernel/whitix/i386/syscalls.S"
# else
#  warning Unsupported Whitix architecture
# endif

/* Unknown */
#else
# warning Unsupported platform
#endif
#ifndef SYSCALL
# define SYSCALL(name)
#endif


/* dirent.h */
#ifdef SYS_readdir
SYSCALL(readdir)
#endif
#ifdef SYS_getdents
SYSCALL(getdents)
#endif
#ifdef SYS_getdirentries
SYSCALL(getdirentries)
#endif

/* fcntl.h */
#ifdef SYS_creat
SYSCALL(creat)
#endif
#ifdef SYS_fcntl
SYSCALL(fcntl)
#endif
#ifdef SYS_flock
SYSCALL(flock)
#endif
#ifdef SYS_open
SYSCALL(open)
#endif

/* poll.h */
#ifdef SYS_poll
SYSCALL(poll)
#endif

/* signal.h */
#ifdef SYS_kill
SYSCALL(kill)
#endif
#ifdef SYS_signal
SYSCALL(signal)
#endif
#ifdef SYS_sigaction
SYSCALL(sigaction)
#endif
#ifdef SYS_sigprocmask
SYSCALL(sigprocmask)
#endif

/* stdio.h */
#ifdef SYS_rename
SYSCALL(rename)
#endif

/* stropts.h */
#ifdef SYS_ioctl
SYSCALL(ioctl)
#endif

/* sys/mman.h */
#ifdef SYS_mlock
SYSCALL(mlock)
#endif
#ifdef SYS_mmap
SYSCALL(mmap)
#endif
#ifdef SYS_mprotect
SYSCALL(mprotect)
#endif
#ifdef SYS_munlock
SYSCALL(munlock)
#endif
#ifdef SYS_munmap
SYSCALL(munmap)
#endif

/* sys/mount.h */
#ifdef SYS__mount
SYSCALL(_mount)
#endif
#ifdef SYS_mount
SYSCALL(mount)
#endif
#ifdef SYS_unmount
SYSCALL(unmount)
#endif

/* sys/reboot.h */
#ifdef SYS_reboot
SYSCALL(reboot)
#endif

/* sys/resource.h */
#ifdef SYS_getpriority
SYSCALL(getpriority)
#endif
#ifdef SYS_getrlimit
SYSCALL(getrlimit)
#endif
#ifdef SYS_getrusage
SYSCALL(getrusage)
#endif
#ifdef SYS_setpriority
SYSCALL(setpriority)
#endif
#ifdef SYS_setrlimit
SYSCALL(setrlimit)
#endif

/* sys/select.h */
#ifdef SYS_select
SYSCALL(select)
#endif

/* sys/shm.h */
#ifdef SYS_shmat
SYSCALL(shmat)
#endif
#ifdef SYS_shmctl
SYSCALL(shmctl)
#endif
#ifdef SYS_shmdt
SYSCALL(shmdt)
#endif
#ifdef SYS_shmget
SYSCALL(shmget)
#endif

/* sys/stat.h */
#ifdef SYS_chmod
SYSCALL(chmod)
#endif
#ifdef SYS_fchmod
SYSCALL(fchmod)
#endif
#ifdef SYS_fstat
SYSCALL(fstat)
#endif
#ifdef SYS_lstat
SYSCALL(lstat)
#endif
#ifdef SYS_mkdir
SYSCALL(mkdir)
#endif
#ifdef SYS_mkfifo
SYSCALL(mkfifo)
#endif
#ifdef SYS_mknod
SYSCALL(mknod)
#endif
#ifdef SYS_stat
SYSCALL(stat)
#endif
#ifdef SYS_umask
SYSCALL(umask)
#endif

/* sys/statvfs.h */
#ifdef SYS__statfs
SYSCALL(_statfs)
#endif
#ifdef SYS_statvfs
SYSCALL(statvfs)
#endif

/* sys/time.h */
#ifdef SYS_getitimer
SYSCALL(getitimer)
#endif
#ifdef SYS_gettimeofday
SYSCALL(gettimeofday)
#endif
#ifdef SYS_setitimer
SYSCALL(setitimer)
#endif
#ifdef SYS_utimes
SYSCALL(utimes)
#endif

/* sys/times.h */
#ifdef SYS_times
SYSCALL(times)
#endif

/* sys/utsname.h */
#ifdef SYS_uname
SYSCALL(uname)
#endif

/* sys/wait.h */
#ifdef SYS_wait
SYSCALL(wait)
#endif
#ifdef SYS_waitpid
SYSCALL(waitpid)
#elif defined(SYS_wait4)
SYSCALL(wait4)
#endif /* !(SYS_waitpid && SYS_wait4) */

/* time.h */
#ifdef SYS_clock_getres
SYSCALL(clock_getres)
#endif
#ifdef SYS_clock_gettime
SYSCALL(clock_gettime)
#endif
#ifdef SYS_clock_settime
SYSCALL(clock_settime)
#endif
#ifdef SYS_nanosleep
SYSCALL(nanosleep)
#endif

/* ucontext.h */
#ifdef SYS_getcontext
SYSCALL(getcontext)
#endif
#ifdef SYS_setcontext
SYSCALL(setcontext)
#endif

/* unistd.h */
#ifdef SYS_access
SYSCALL(access)
#endif
#ifdef SYS_alarm
SYSCALL(alarm)
#endif
#ifdef SYS_brk
SYSCALL(brk)
#endif
#ifdef SYS_chdir
SYSCALL(chdir)
#endif
#ifdef SYS_chown
SYSCALL(chown)
#endif
#ifdef SYS_close
SYSCALL(close)
#endif
#ifdef SYS_dup
SYSCALL(dup)
#endif
#ifdef SYS_dup2
SYSCALL(dup2)
#endif
#ifdef SYS_execve
SYSCALL(execve)
#endif
#ifdef SYS_fchdir
SYSCALL(fchdir)
#endif
#ifdef SYS_fchown
SYSCALL(fchown)
#endif
#ifdef SYS_fork
SYSCALL(fork)
#endif
#ifdef SYS_fsync
SYSCALL(fsync)
#endif
#ifdef SYS_ftruncate
SYSCALL(ftruncate)
#endif
#ifdef SYS__getcwd
SYSCALL(_getcwd)
#endif
#ifdef SYS_getegid
SYSCALL(getegid)
#endif
#ifdef SYS_geteuid
SYSCALL(geteuid)
#endif
#ifdef SYS_getgid
SYSCALL(getgid)
#endif
#ifdef SYS_getlogin_r
SYSCALL(getlogin_r)
#endif
#ifdef SYS_getpgrp
SYSCALL(getpgrp)
#endif
#ifdef SYS_getpid
SYSCALL(getpid)
#endif
#ifdef SYS_getppid
SYSCALL(getppid)
#endif
#ifdef SYS_getuid
SYSCALL(getuid)
#endif
#ifdef SYS_lchown
SYSCALL(lchown)
#endif
#ifdef SYS_link
SYSCALL(link)
#endif
#ifdef SYS_lseek
SYSCALL(lseek)
#endif
#ifdef SYS_nice
SYSCALL(nice)
#endif
#ifdef SYS_pipe
SYSCALL(pipe)
#endif
#ifdef SYS_read
SYSCALL(read)
#endif
#ifdef SYS_readlink
SYSCALL(readlink)
#endif
#ifdef SYS_rmdir
SYSCALL(rmdir)
#endif
#ifdef SYS_sbrk
SYSCALL(sbrk)
#endif
#ifdef SYS_setgid
SYSCALL(setgid)
#endif
#ifdef SYS_sethostname
SYSCALL(sethostname)
#endif
#ifdef SYS_setpgid
SYSCALL(setpgid)
#endif
#ifdef SYS_setregid
SYSCALL(setregid)
#endif
#ifdef SYS_setreuid
SYSCALL(setreuid)
#endif
#ifdef SYS_setsid
SYSCALL(setsid)
#endif
#ifdef SYS_setuid
SYSCALL(setuid)
#endif
#ifdef SYS_symlink
SYSCALL(symlink)
#endif
#ifdef SYS_sync
SYSCALL(sync)
#endif
#ifdef SYS_truncate
SYSCALL(truncate)
#endif
#ifdef SYS_unlink
SYSCALL(unlink)
#endif
#ifdef SYS_vfork
SYSCALL(vfork)
#endif
#ifdef SYS_write
SYSCALL(write)
#endif

/* utime.h */
#ifdef SYS_utime
SYSCALL(utime)
#endif
