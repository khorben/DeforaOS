/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef SYSCALLS_H
# define SYSCALLS_H

# if defined(__linux__)
#  include "kernel/linux/common.h"
# elif defined(__NetBSD__)
#  include "kernel/netbsd/common.h"
# elif defined(__sun__)
#  include "kernel/solaris/common.h"
# else
#  warning Unsupported platform
# endif

#endif /* !SYSCALLS_H */
