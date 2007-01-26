/* echo.c */



#include <unistd.h>
#include <stdio.h>


/* echo */
static int _echo(int argc, char * argv[])
{
	int i;

	if(argc != 0)
	{
		printf("%s", argv[0]);
		for(i = 1; i < argc; i++)
			printf(" %s", argv[i]);
	}
	fputc('\n', stdout);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: echo [string...]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	return _echo(argc - 1, &argv[1]);
}
