/* logname.c */



#include <unistd.h>
#include <stdio.h>


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: logname\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	char * lgnm;

	if(argc != 1)
		return _usage();
	if((lgnm = getlogin()) == NULL)
	{
		fprintf(stderr, "%s", "logname: ");
		perror("getlogin");
		return 2;
	}
	printf("%s\n", lgnm);
	return 0;
}
