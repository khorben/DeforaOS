/* prefs.c */



#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include "prefs.h"


/* prefs */
int prefs_parse(struct prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(struct prefs));
	while((o = getopt(argc, argv, "abCefhimnuvxo:cs")) != -1)
	{
		switch(o)
		{
			case 'c':
				prefs->c = 1;
				prefs->s = 0;
				break;
			case 'i':
				prefs->i = 1;
				break;
			case 's':
				prefs->c = 0;
				prefs->s = 1;
				break;
			case '?':
				return 1;
#ifdef DEBUG
			default:
				fprintf(stderr, "-%c: Not yet implemented\n", o);
				return 1;
#endif
		}
	}
	return 0;
}
