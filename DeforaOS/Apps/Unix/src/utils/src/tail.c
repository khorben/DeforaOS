/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* tail */
static int _tail(void)
{
	return 1;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: tail [-f][-c number|-n number][file]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

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
