/* dirname.c */



#include <libgen.h>
#include <stdio.h>


/* usage */
static int usage(void)
{
	fprintf(stderr, "%s", "Usage: dirname string\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	/* check for errors */
	if(argc != 2)
		return usage();
	/* dirname */
	printf("%s\n", dirname(argv[1]));
	return 0;
}
