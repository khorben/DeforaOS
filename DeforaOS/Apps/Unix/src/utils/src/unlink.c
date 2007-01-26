/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
	fprintf(stderr, "%s", "Usage: unlink file\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 2)
		return _usage();
	return _unlink(argv[1]);
}
