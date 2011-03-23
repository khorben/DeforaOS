/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
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



#include <System.h>
#include <unistd.h>
#include <stdio.h>
#include "As/as.h"
#include "common.h"
#include "../config.h"


/* as */
/* private */
/* constants */
# define AS_FILENAME_DEFAULT "a.out"


/* functions */
/* as */
static int _as(char const * arch, char const * format, char const * infile,
		char const * outfile)
{
	int ret = 0;
	As * as;

	if((as = as_new(arch, format)) == NULL)
		return error_print(PACKAGE);
	if(as_parse(as, infile, outfile) != 0)
		ret = error_print(PACKAGE);
	as_delete(as);
	return ret;
}


/* usage */
static unsigned int _usage(void)
{
	As * as;
	char const * arch;
	char const * format;

	if((as = as_new(NULL, NULL)) == NULL)
		return error_print(PACKAGE);
	arch = as_get_arch(as);
	format = as_get_format(as);
	fprintf(stderr, "%s%s%s%s%s",
"Usage: as [-a arch][-f format][-o file] file\n"
"       as -l\n"
"  -a	target architecture (default: ", arch != NULL ? arch : "guessed", ")\n"
"  -f	target file format (default: ", format, ")\n"
"  -o	filename to use for output (default: " AS_FILENAME_DEFAULT ")\n"
"  -l	list available architectures and formats\n");
	as_delete(as);
	return 1;
}


/* public */
/* main */
int main(int argc, char * argv[])
{
	int o;
	char * outfile = AS_FILENAME_DEFAULT;
	char const * arch = NULL;
	char const * format = NULL;

	while((o = getopt(argc, argv, "a:f:o:l")) != -1)
	{
		switch(o)
		{
			case 'a':
				arch = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'l':
				o = 0;
				if(as_plugin_list(ASPT_ARCH) != 0)
					o = error_print(PACKAGE);
				else
					putchar('\n');
				if(as_plugin_list(ASPT_FORMAT) != 0)
					o = error_print(PACKAGE);
				return (o == 0) ? 0 : 2;
			default:
				return _usage();
		}
	}
	if(argc - optind != 1)
		return _usage();
	return (_as(arch, format, argv[optind], outfile) == 0) ? 0 : 2;
}
