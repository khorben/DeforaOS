/* unlink.c */



#include <unistd.h>
#include <stdio.h>


/* usage */
static int usage(void)
{
	fprintf(stderr, "%s", "Usage: unlink file\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	/* check for errors */
	if(argc != 2)
		return usage();
	/* unlink */
	if(unlink(argv[1]) == -1)
	{
		perror("unlink");
		return 2;
	}
	return 0;
}
