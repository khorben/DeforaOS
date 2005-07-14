/* unlink.c */



#include <unistd.h>
#include <stdio.h>


/* unlink */
static int _unlink(char * file)
{
	if(unlink(file) == -1)
	{
		fprintf(stderr, "%s", "unlink: ");
		perror(file);
		return 2;
	}
	return 0;
}


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
	return _unlink(argv[1]);
}
