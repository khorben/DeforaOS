/* link.c */



#include <unistd.h>
#include <stdio.h>


/* link */
static int _link(char * file1, char * file2)
{
	if(link(file1, file2) == -1)
	{
		perror("link");
		return 2;
	}

	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: link file1 file2\n");
	return 1;
}


/* main */
int main(int argc, char* argv[])
{
	if(argc != 3)
		return _usage();
	return _link(argv[1], argv[2]);
}
