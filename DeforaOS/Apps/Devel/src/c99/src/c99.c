/* $id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* types */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_E 0x2
#define PREFS_g 0x4
#define PREFS_s 0x8


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
}
