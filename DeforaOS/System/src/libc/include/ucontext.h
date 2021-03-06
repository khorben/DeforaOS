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



#ifndef LIBC_UCONTEXT_H
# define LIBC_UCONTEXT_H

# include "compat/ucontext.h"


/* types */
# ifndef ucontext_t
#  define ucontext_t ucontext_t
typedef struct _ucontext_t ucontext_t;
# endif


/* functions */
int getcontext(ucontext_t * context);
int setcontext(const ucontext_t * context);

#endif /* !LIBC_UCONTEXT_H */
