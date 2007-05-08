/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* unlink */
static int _unlink(char * file)
{
	if(unlink(file) == -1)
	{
		fputs("unlink: ", stderr);
		perror(file);
		return 2;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: unlink file\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 2)
		return _usage();
	return _unlink(argv[1]);
}
