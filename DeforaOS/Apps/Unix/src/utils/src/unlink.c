/* unlink.c */



#include <unistd.h>
#include <stdio.h>


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: unlink file\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 2)
		return _usage();
	if(unlink(argv[1]) == -1)
	{
		perror("unlink");
		return 2;
	}
	return 0;
}
