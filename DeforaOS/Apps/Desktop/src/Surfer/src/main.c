/* main.c */



#include <unistd.h>
#include <stdio.h>
#include "surfer.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: surfer [url]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Surfer * surfer;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		surfer = surfer_new(NULL);
	else if(optind+1 == argc)
		surfer = surfer_new(argv[optind]);
	else
		return _usage();
	if(surfer == NULL)
		return 2;
	gtk_main();
	surfer_delete(surfer);
	return 0;
}
