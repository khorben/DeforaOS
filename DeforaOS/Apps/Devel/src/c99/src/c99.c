/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_E 0x2
#define PREFS_g 0x4
#define PREFS_s 0x8


/* c99 */
static int _c99_error(char const * message, int ret);
static int _c99_do(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile);
static int _c99(Prefs * prefs, char const * outfile, int filec, char * filev[])
{
	FILE * fp;
	int ret = 0;
	int i;

	if(outfile != NULL && (fp = fopen(outfile, "w")) == NULL)
		return _c99_error(outfile, 1);
	for(i = 0; i < filec; i++)
		ret |= _c99_do(prefs, outfile, fp, filev[i]);
	if(fp != NULL)
		fclose(fp);
	return ret;
}

static int _c99_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "c99: ");
	perror(message);
	return ret;
}

static int _c99_do_c(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp);
static int _c99_do_E(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp);
static int _c99_do_o(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp);
static int _c99_do(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile)
{
	FILE * infp;
	int ret;

	if((infp = fopen(infile, "r")) == NULL)
		return _c99_error(infile, 1);
	if(*prefs & PREFS_c)
		ret = _c99_do_c(prefs, outfile, outfp, infile, infp);
	else if(*prefs & PREFS_E)
		ret = _c99_do_E(prefs, outfile, outfp, infile, infp);
	else
		ret = _c99_do_o(prefs, outfile, outfp, infile, infp);
	/* FIXME implement */
	fclose(infp);
	return ret;
}

static int _c99_do_c(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp)
{
	size_t len;
	char * o = NULL;

	if(outfile == NULL)
	{
		if((len = strlen(infile)) < 3 || infile[len-2] != '.'
				|| infile[len-1] != 'c')
		{
			errno = EINVAL;
			return _c99_error(infile, 1);
		}
		if((o = strdup(infile)) == NULL)
			return _c99_error("strdup", 1);
		o[len-1] = 'o';
		if((outfp = fopen(o, "w")) == NULL)
		{
			free(o);
			return _c99_error(o, 1);
		}
	}
	/* FIXME implement */
	if(o != NULL)
	{
		fclose(outfp);
		free(o);
	}
	return 0;
}

static int _c99_do_E(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp)
{
	/* FIXME implement */
	return 1;
}

static int _c99_do_o(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile, FILE * infp)
{
	/* FIXME implement */
	return 1;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: c99 [-c][-D name[=value]]...[-E][-g][-I directory][-L directory][-o outfile][-Ooptlevel][-s][-U name]... operand ...\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	char * outfile = NULL;
	int o;
	char oldo = '\0';

	for(; (o = getopt(argc, argv, "cD:EgI:L:o:O123sU:")) != -1; oldo = o)
		switch(o)
		{
			case 'c':
				prefs |= PREFS_c;
				break;
			case 'E':
				prefs |= PREFS_E;
				break;
			case 'g':
				prefs |= PREFS_g;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 's':
				prefs |= PREFS_s;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	if(prefs & PREFS_c && outfile != NULL && optind+1 != argc)
		return _usage();
	return _c99(&prefs, outfile, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
