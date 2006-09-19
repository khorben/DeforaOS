/* open.c */



#include <unistd.h>
#include <stdio.h>
#include "mime.h"


/* open */
static int _open(char const * mime, char const * action, int filec,
		char * filev[])
{
	int i;
	Mime * m;
	int ret = 0;

	if((m = mime_new()) == NULL)
		return 1;
	for(i = 0; i < filec; i++)
		if(mime_action(m, action, filev[i]) != 0)
		{
			fprintf(stderr, "%s%s%s%s%s", "mime: ", filev[i],
					": Could not call action \"", action,
					"\"\n");
			ret = 1;
		}
	mime_delete(m);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: open [-m mime][-a action] file...\n"
"  -m	MIME type to force (default: auto-detected)\n"
"  -a	action to call (default: \"open\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * mime = NULL;
	char const * action = "open";

	while((o = getopt(argc, argv, "m:a:")) != -1)
		switch(o)
		{
			case 'm':
				mime = optarg;
				break;
			case 'a':
				action = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _open(mime, action, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
