/* dirname.c */



#include <libgen.h>
#include <stdio.h>


/* dirname */
static int _dirname(char * arg)
{
	printf("%s\n", dirname(arg));
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: dirname string\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 2)
		return _usage();
	return _dirname(argv[1]);
}
