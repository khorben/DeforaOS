/* dirname.c */



#include <libgen.h>
#include <stdio.h>


/* main */
int main(int argc, char * argv[])
{
	/* parse command line arguments */
	if(argc != 2)
	{
		fprintf(stderr, "Usage: dirname string\n");
		return 1;
	}

	/* dirname */
	printf("%s\n", dirname(argv[1]));

	return 0;
}
