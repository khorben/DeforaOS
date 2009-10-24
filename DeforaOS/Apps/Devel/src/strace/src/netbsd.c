/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel strace */
/* strace is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * strace is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with strace; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <stdlib.h>
#include "netbsd.h"


#ifdef __NetBSD__
/* variables */
char const * stracecall[] =
{
	NULL,
	"exit",
	"fork",
	"read",
	"write",
	"open",
	"close",
	"wait4",
	"oldcreat",
	"link",
	"unlink",
	"oldexecv",
	"chdir",
	"fchdir",
	"mknod",
	"chmod",
	"chown",
	"break",
	NULL,
	"oldlseek",
	"getpid"
};
#endif /* __NetBSD__ */
