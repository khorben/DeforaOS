/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
/* inetd is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * inetd is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * inetd; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdio.h>


/* discard */
static int _discard(void)
{
	int c;

	while((c = fgetc(stdin)) != EOF);
	return feof(stdin) ? 0 : 2;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: discard\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _discard();
}
