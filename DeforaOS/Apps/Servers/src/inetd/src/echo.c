/* echo.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* echo */
/* PRE
 * POST	the requested files have been printed on standard output
 * 	returns:
 * 		0	successful
 * 		2	an error occured */
static int _echo(void)
{
	int c;

	while((c = fgetc(stdin)) != EOF)
	{
		if(fputc(c, stdout) == EOF)
			return feof(stdout) ? 0 : 2;
		if(c == '\n')
			fflush(stdout);
	}
	return feof(stdin) ? 0 : 2;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: echo\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _echo();
}
