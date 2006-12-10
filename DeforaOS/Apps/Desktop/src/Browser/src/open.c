/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */
/* This file is part of Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <unistd.h>
#include <stdio.h>
#include "mime.h"


/* open */
static int _open(char const * mime, char const * action, int filec,
		char * filev[])
{
	int i;
	Mime * m;
	int ret = 0;

	if((m = mime_new()) == NULL)
		return 1;
	for(i = 0; i < filec; i++)
		if(mime_action(m, action, filev[i]) != 0)
		{
			fprintf(stderr, "%s%s%s%s%s", "mime: ", filev[i],
					": Could not perform action \"", action,
					"\"\n");
			ret = 1;
		}
	mime_delete(m);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: open [-m mime][-a action] file...\n"
"  -m	MIME type to force (default: auto-detected)\n"
"  -a	action to call (default: \"open\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * mime = NULL;
	char const * action = "open";

	while((o = getopt(argc, argv, "m:a:")) != -1)
		switch(o)
		{
			case 'm':
				mime = optarg;
				break;
			case 'a':
				action = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _open(mime, action, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
