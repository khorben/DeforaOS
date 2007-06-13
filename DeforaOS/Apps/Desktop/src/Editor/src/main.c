/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* Editor is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Editor is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Editor; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <unistd.h>
#include <stdio.h>
#include "editor.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: editor [file]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Editor * e;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc && optind+1 != argc)
		return _usage();
	if((e = editor_new()) == NULL)
		return 2;
	if(argc - optind == 1)
		editor_open(e, argv[optind]);
	gtk_main();
	editor_delete(e);
	return 0;
}
