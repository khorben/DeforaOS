/* main.c */



#include <unistd.h>
#include <stdio.h>
#include "editor.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: editor [file]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Editor * e;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc && optind+1 != argc)
		return _usage();
	if((e = editor_new()) == NULL)
		return 2;
	if(argc - optind == 1)
		editor_open(e, argv[optind]);
	gtk_main();
	editor_delete(e);
	return 0;
}
