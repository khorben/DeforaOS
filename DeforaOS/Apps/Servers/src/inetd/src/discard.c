/* discard.c */



#include <stdio.h>


/* discard */
static int _discard(void)
{
	int c;

	while((c = fgetc(stdin)) != EOF);
	return feof(stdin) ? 0 : 2;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: discard\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _discard();
}
