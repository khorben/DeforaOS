/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Loader */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <elf.h>


/* private */
/* prototypes */
static int _error(char const * error1, char const * error2, int ret);
static int _ldd(char const * filename);
#undef ELFSIZE
#define ELFSIZE 32
#include "elf.c"
#undef ELFSIZE
#define ELFSIZE 64
#include "elf.c"
static int _usage(void);


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret = 0;
	int o;
	int i;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	for(i = optind; i < argc; i++)
		ret |= _ldd(argv[i]);
	return (ret == 0) ? 0 : 2;
}


/* private */
/* functions */
/* ldd */
static int _ldd(char const * filename)
{
	int ret = 1;
	FILE * fp;
	union
	{
		unsigned char e_ident[EI_NIDENT];
		Elf32_Ehdr ehdr32;
		Elf64_Ehdr ehdr64;
	} elf;

	if((fp = fopen(filename, "r")) == NULL)
		return _error(filename, strerror(errno), 1);
	if(fread(&elf, sizeof(elf), 1, fp) == 1)
	{
		if(memcmp(elf.e_ident, ELFMAG, SELFMAG) != 0)
			ret = -_error(filename, "Not an ELF file", 1);
		else if(elf.e_ident[EI_CLASS] == ELFCLASS32)
			ret = _do_ldd32(filename, fp, &elf.ehdr32);
		else if(elf.e_ident[EI_CLASS] == ELFCLASS64)
			ret = _do_ldd64(filename, fp, &elf.ehdr64);
		else
			ret = -_error(filename, "Could not determine ELF class",
					1);
	}
	else if(ferror(fp) != 0)
		ret = -_error(filename, strerror(errno), 1);
	else
		ret = -_error(filename, "Not an ELF file", 1);
	if(fclose(fp) != 0)
		ret = -_error(filename, strerror(errno), 1);
	return ret;
}


/* error */
static int _error(char const * error1, char const * error2, int ret)
{
	fprintf(stderr, "%s: %s%s%s\n", "ldd", error1, (error2 != NULL) ? ": "
			: "", (error2 != NULL) ? error2 : "");
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ldd filename...\n", stderr);
	return 1;
}
