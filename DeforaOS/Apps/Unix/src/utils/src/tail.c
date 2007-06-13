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
#include <stdio.h>


/* tail */
static int _tail(void)
{
	/* FIXME implement */
	return 1;
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

	/* FIXME actually collect arguments */
	while((o = getopt(argc, argv, "fc:n:")) != -1)
		switch(o)
		{
			case 'f':
				break;
			case 'c':
				break;
			case 'n':
				break;
			default:
				return _usage();
		}
	return _tail() ? 0 : 2;
}
