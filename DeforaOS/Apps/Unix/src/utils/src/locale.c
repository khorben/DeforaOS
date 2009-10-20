/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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


/* prefs */
typedef int Prefs;
#define LOCALE_PREFS_a 0x1
#define LOCALE_PREFS_m 0x2
#define LOCALE_PREFS_c 0x4
#define LOCALE_PREFS_k 0x8


/* locale */
static int _locale_locales(void);
static int _locale_charsets(void);
static int _locale_default(void);
static int _locale_do(Prefs * p, char const * locale);

static int _locale(Prefs * p, int argc, char * argv[])
{
	int ret = 0;
	int i;

	if(*p & LOCALE_PREFS_a)
		return _locale_locales();
	if(*p & LOCALE_PREFS_m)
		return _locale_charsets();
	if(*p == 0 && argc == 0)
		return _locale_default() ? 2 : 0;
	for(i = 0; i < argc; i++)
		ret += _locale_do(p, argv[i]);
	return ret ? 2 : 0;
}

static int _locale_locales(void)
{
	puts("C\nPOSIX");
	return 0;
}

static int _locale_charsets(void)
{
	puts("ISO_10646");
	return 0;
}

static int _locale_default(void)
{
	char * vars[] = { "LANG", NULL };
	char ** p;
	char * e;

	for(p = vars; *p != NULL; p++)
	{
		printf("%s=", *p);
		if((e = getenv(*p)) == NULL)
			puts("\"\"");
		else
			puts(e);
	}
	if((e = getenv("LC_ALL")) == NULL)
		e = "";
	printf("%s%s\n", "LC_ALL=", e);
	return 0;
}

static int _locale_do(Prefs * p, char const * locale)
{
	/* FIXME implement */
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: locale [-a | -m]\n\
      locale [-ck]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p = 0;
	int o;

	while((o = getopt(argc, argv, "amck")) != -1)
		switch(o)
		{
			case 'a':
				p -= (p & (LOCALE_PREFS_m | LOCALE_PREFS_c
							| LOCALE_PREFS_k));
				p |= LOCALE_PREFS_a;
				break;
			case 'm':
				p -= (p & (LOCALE_PREFS_a | LOCALE_PREFS_c
							| LOCALE_PREFS_k));
				p |= LOCALE_PREFS_m;
				break;
			case 'c':
				p -= (p & (LOCALE_PREFS_a | LOCALE_PREFS_m));
				p |= LOCALE_PREFS_c;
				break;
			case 'k':
				p -= (p & (LOCALE_PREFS_a | LOCALE_PREFS_m));
				p |= LOCALE_PREFS_k;
				break;
			default:
				return _usage();
		}
	return _locale(&p, argc - optind, &argv[optind]);
}
