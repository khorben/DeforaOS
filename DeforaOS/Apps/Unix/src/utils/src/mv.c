/* $Id$ */



#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2


/* mv */
static int _mv(Prefs * prefs, int filec, char * filev[])
{
	fputs("mv: Not implemented\n", stderr);
	return 1;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mv [-fi] source_file... target_file\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;

	memset(&prefs, 0, sizeof(Prefs));
	prefs |= PREFS_f;
	while((o = getopt(argc, argv, "fi")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & PREFS_i;
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs -= prefs & PREFS_f;
				prefs |= PREFS_i;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	return _mv(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
