/* rm.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2
#define PREFS_R 0x4


/* rm */
static int _rm_error(char * message, int ret);
static int _rm_do(Prefs * prefs, char * file);
static int _rm(Prefs * prefs, int argc, char * argv[])
{
	int i;
	int ret = 0;

	for(i = 0; i < argc; i++)
		ret+=_rm_do(prefs, argv[i]);
	return ret ? 2 : 0;
}

static int _rm_error(char * message, int ret)
{
	fprintf(stderr, "%s", "rm: ");
	perror(message);
	return ret;
}

static int _rm_do(Prefs * prefs, char * file)
	/* FIXME */
{
	if(unlink(file) != 0)
		return _rm_error(file, 2);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: rm [-fiRr] file...\n\
  -f    Do not prompt for confirmation or output error messages\n\
  -i    Prompt for confirmation\n\
  -R    Remove file hierarchies\n\
  -r    Equivalent to -R\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "fiRr")) != -1)
	{
		switch(o)
		{
			case 'f':
				prefs = prefs - (prefs & PREFS_i);
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs = prefs - (prefs & PREFS_f);
				prefs |= PREFS_i;
				break;
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			case '?':
				return _usage();
		}
	}
	if(optind == argc)
		return _usage();
	return _rm(&prefs, argc-optind, &argv[optind]);
}
