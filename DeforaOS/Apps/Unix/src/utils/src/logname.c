/* logname.c */



#include <unistd.h>
#include <stdio.h>


/* usage */
int usage(void)
{
	fprintf(stderr, "Usage: logname\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	char * lgnm;

	if(argc != 1)
		return usage();
	if((lgnm = getlogin()) == NULL)
	{
		perror("getlogin");
		return 2;
	}
	printf("%s\n", lgnm);
	return 0;
}
