/* cpp.c */



#include <unistd.h>
#include <stdio.h>


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: cpp filename\n");
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
	return 2;
}
