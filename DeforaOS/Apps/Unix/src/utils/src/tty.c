/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* tty */
static int _tty_error(char const * message, int ret);
static int _tty(void)
{
	char * tty;

	if(isatty(0) == 1)
	{
		if((tty = ttyname(0)) == NULL)
			return _tty_error("ttyname", 2);
		printf("%s\n", tty);
		return 0;
	}
	printf("%s", "not a tty\n");
	return 1;
}

static int _tty_error(char const * message, int ret)
{
	fputs("tty: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: tty\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _tty();
}
