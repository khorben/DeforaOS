/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
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
#include <stdio.h>
#include <string.h>


/* echo */
/* PRE
 * POST	the requested files have been printed on standard output
 * 	returns:
 * 		0	successful
 * 		2	an error occured */
static int _echo(void)
{
	int c;

	while((c = fgetc(stdin)) != EOF)
	{
		if(fputc(c, stdout) == EOF)
			return feof(stdout) ? 0 : 2;
		if(c == '\n')
			fflush(stdout);
	}
	return feof(stdin) ? 0 : 2;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: echo\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _echo();
}
