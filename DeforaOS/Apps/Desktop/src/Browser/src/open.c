/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdio.h>
#include "mime.h"


/* open */
static int _open(char const * type, char const * action, int filec,
		char * filev[])
{
	int i;
	Mime * mime;
	int ret = 0;

	if((mime = mime_new()) == NULL)
		return 1;
	for(i = 0; i < filec; i++)
	{
		if(type == NULL)
		{
			if(mime_action(mime, action, filev[i]) == 0)
				continue;
		}
		else if(mime_action_type(mime, action, filev[i], type) == 0)
			continue;
		fprintf(stderr, "%s%s%s%s%s", "mime: ", filev[i], ": Could not"
				" perform action \"", action, "\"\n");
		ret = 1;
	}
	mime_delete(mime);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: open [-m mime type][-a action] file...\n"
"  -m	MIME type to force (default: auto-detected)\n"
"  -a	action to call (default: \"open\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * action = "open";
	char const * type = NULL;

	while((o = getopt(argc, argv, "a:m:")) != -1)
		switch(o)
		{
			case 'a':
				action = optarg;
				break;
			case 'm':
				type = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _open(type, action, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
