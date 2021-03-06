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



#ifndef SYSCALLS_H
# define SYSCALLS_H

/* FreeBSD */
# if defined(__FreeBSD__)
#  include "kernel/freebsd/common.h"
/* Linux */
# elif defined(__linux__)
#  include "kernel/linux/common.h"
/* MacOS X */
# elif defined(__APPLE__)
#  include "kernel/darwin/common.h"
/* NetBSD */
# elif defined(__NetBSD__)
#  include "kernel/netbsd/common.h"
#  include "kernel/netbsd/sys/sysctl.h"
/* OpenBSD */
# elif defined(__OpenBSD__)
#  include "kernel/openbsd/common.h"
/* Solaris */
# elif defined(__sun__)
#  include "kernel/solaris/common.h"
/* Whitix */
# elif defined(__Whitix__)
#  include "kernel/whitix/common.h"
# else
#  warning Unsupported platform
# endif

#endif /* !SYSCALLS_H */
