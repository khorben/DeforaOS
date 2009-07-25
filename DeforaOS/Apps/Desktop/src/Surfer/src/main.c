/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <unistd.h>
#include <stdio.h>
#include "surfer.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: surfer [url]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Surfer * surfer;

#if defined(WITH_GTKHTML) || defined(WITH_WEBKIT) /* XXX */
	if(g_thread_supported() == FALSE)
		g_thread_init(NULL);
#endif /* WITH_GTKHTML */
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		surfer = surfer_new(NULL);
	else if(optind + 1 == argc)
		surfer = surfer_new(argv[optind]);
	else
		return _usage();
	if(surfer == NULL)
		return 2;
	gtk_main();
	return 0;
}
