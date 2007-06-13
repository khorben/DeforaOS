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


/* sleep */
static int _sleep(unsigned int time)
{
	sleep(time);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: sleep time\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	long time;
	char * p;

	if(argc != 2)
		return _usage();
	time = strtol(argv[1], &p, 10);
	if(argv[1][0] == '\0' || *p != '\0' || time < 0)
		return _usage();
	return _sleep(time);
}
