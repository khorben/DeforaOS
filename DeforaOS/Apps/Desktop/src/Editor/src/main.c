/* main.c */



#include <unistd.h>
#include <stdio.h>
#include "editor.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: editor\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Editor * e;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	gtk_init(&argc, &argv);
	if((e = editor_new()) == NULL)
		return 2;
	gtk_main();
	editor_delete(e);
	return 0;
}
