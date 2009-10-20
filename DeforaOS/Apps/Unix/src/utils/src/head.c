/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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


/* head */
static int _head_error(char const * message, int ret);
static int _head_do(int flgn, char const * filename);

static int _head(int flgn, int filec, char * filev[])
{
	int ret = 0;
	int i;

	if(filec == 0)
		return _head_do(flgn, NULL);
	for(i = 0; i < filec; i++)
		if(_head_do(flgn, filev[i]) != 0)
			ret = 1;
	return ret;
}

static int _head_error(char const * message, int ret)
{
	fputs("head: ", stderr);
	perror(message);
	return ret;
}

static int _head_do(int flgn, char const * filename)
{
	FILE * fp;
	int n = 0;
	int c;

	if(filename == NULL)
		fp = stdin;
	else if((fp = fopen(filename, "r")) == NULL)
		return _head_error(filename, 1);
	while((c = fgetc(fp)) != EOF && n < flgn)
	{
		if(c == '\n')
			n++;
		fwrite(&c, sizeof(char), 1, stdout);
	}
	if(filename != NULL && fclose(fp) != 0)
		return _head_error(filename, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: head [-n number][file...]\n\
  -n	Print first number lines on standard output\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flgn = 10;
	int o;
	char * p;

	while((o = getopt(argc, argv, "n:")) != -1)
		switch(o)
		{
			case 'n':
				flgn = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	return _head(flgn, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
