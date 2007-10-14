/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/reboot.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


/* poweroff */
/* functions */
/* poweroff */
static int _poweroff(void)
{
	sync();
#if defined(RF_HALT) /* DeforaOS */
	if(reboot(RF_POWEROFF) != 0)
#elif defined(RB_AUTOBOOT) /* NetBSD */
	if(reboot(RB_POWERDOWN, NULL) != 0)
#else
# warning Unsupported platform
	errno = ENOSYS;
#endif
	{
		perror("poweroff");
		return 1;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: poweroff\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		return _usage();
	if(optind != argc)
		return _usage();
	return _poweroff() ? 0 : 2;
}
