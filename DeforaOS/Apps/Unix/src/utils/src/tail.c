/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* utils is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * utils is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with utils; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


/* tail */
/* types */
typedef struct _Prefs
{
	int flags;
	int bytes;
	int lines;
} Prefs;
#define PREFS_f	0x1

static int _tail_error(char const * message, int ret);
static int _tail_do_bytes(Prefs * prefs, FILE * fp, char const * filename);
static int _tail_do_lines(Prefs * prefs, FILE * fp, char const * filename);

static int _tail(Prefs * prefs, char const * filename)
{
	int ret;
	FILE * fp = stdin;

	if(filename != NULL && (fp = fopen(filename, "r")) == NULL)
		return _tail_error(filename, 1);
	if(prefs->bytes != 0)
		ret = _tail_do_bytes(prefs, fp, filename != NULL
				? filename : "stdin");
	else
		ret = _tail_do_lines(prefs, fp, filename != NULL
				? filename : "stdin");
	if(filename != NULL && fclose(fp) != 0)
		return _tail_error(filename, 1);
	return ret;
}

static int _tail_error(char const * message, int ret)
{
	fputs("tail: ", stderr);
	perror(message);
	return ret;
}

static int _tail_do_bytes(Prefs * prefs, FILE * fp, char const * filename)
{
	char buf[256];
	size_t i;
	long c = prefs->bytes;

	if(fseek(fp, c > 0 ? c - 1 : c, c > 0 ? SEEK_SET : SEEK_END) != 0)
	{
		fputs("tail: Not implemented yet\n", stderr);
		return 1;
	}
	while((i = fread(buf, 1, sizeof(buf), fp)) > 0)
		if(fwrite(buf, 1, i, stdout) != i)
			return _tail_error("stdout", 1);
	if(!feof(fp))
		return _tail_error(filename, 1);
	return 0;
}

/* tail_do_lines */
static void _lines_rotate(char ** lines, int pos);
static int _lines_realloc(char ** lines, int pos, size_t column);
static void _lines_print(char ** lines, int pos);

static int _tail_do_lines(Prefs * prefs, FILE * fp, char const * filename)
{
	int ret = 0;
	char ** lines;
	int c;
	int pos = 0;
	size_t column = 0;

	if(prefs->lines == 0)
		return 0;
	if((lines = calloc(sizeof(*lines), prefs->lines)) == NULL)
		return _tail_error(filename, 1);
	while((c = fgetc(fp)) != EOF)
	{
		if(column == 0 && lines[pos] != NULL)
		{
			if(pos + 1 < prefs->lines)
				pos++;
			else
				_lines_rotate(lines, pos);
		}
		if(_lines_realloc(lines, pos, column) != 0)
			break;
		if(c != '\n')
		{
			lines[pos][column++] = c;
			continue;
		}
		lines[pos][column] = '\0';
		column = 0;
	}
	_lines_print(lines, pos);
	if(c != EOF || !feof(fp))
		ret = _tail_error(filename, 1);
	free(lines);
	return ret;
}

static void _lines_rotate(char ** lines, int pos)
{
	int i;

	free(lines[0]);
	for(i = 1; i <= pos; i++)
		lines[i - 1] = lines[i];
	lines[pos] = NULL;
}

static int _lines_realloc(char ** lines, int pos, size_t column)
{
	char * p;

	if((p = realloc(lines[pos], column + 1)) == NULL)
		return 1;
	lines[pos] = p;
	return 0;
}

static void _lines_print(char ** lines, int pos)
{
	int i;

	for(i = 0; i <= pos; i++)
	{
		fputs(lines[i], stdout);
		free(lines[i]);
		fputc('\n', stdout);
	}
}


/* usage */
static int _usage(void)
{
	fputs("Usage: tail [-f][-c number|-n number][file]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.lines = 10;
	while((o = getopt(argc, argv, "fc:n:")) != -1)
		switch(o)
		{
			case 'f':
				prefs.flags |= PREFS_f;
				break;
			case 'c':
				prefs.lines = 0;
				prefs.bytes = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'n':
				prefs.bytes = 0;
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind != argc && optind + 1 != argc)
		return _usage();
	return _tail(&prefs, argv[optind]) ? 0 : 2;
}
