/* sleep.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* sleep */
static int _sleep(unsigned int time)
{
	sleep(time);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: sleep time\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	long time;
	char * p;

	if(argc != 2)
		return _usage();
	time = strtol(argv[1], &p, 10);
	if(argv[1][0] == '\0' || *p != '\0' || time < 0)
		return _usage();
	return _sleep(time);
}
