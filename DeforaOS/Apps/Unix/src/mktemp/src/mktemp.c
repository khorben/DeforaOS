/* mktemp.c */



#include <unistd.h>
extern int optind;
#include <stdlib.h>
#include <stdio.h>


/* mktemp */
static int _mktemp(char * template)
{
	int fd;

	if((template = strdup(template)) == NULL)
	{
		perror("strdup");
		return 1;
	}
	if((fd = mkstemp(template)) == -1)
	{
		fprintf(stderr, "%s", "mktemp: ");
		perror(template);
		return 1;
	}
	close(fd);
	printf("%s\n", template);
	free(template);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: mktemp [-d] [template]\n\
  -d    Make a directory instead of a file\n");
	return 1;
}

/* main */
int main(int argc, char * argv[])
{
	int o;
	char * template = "/tmp/tmp.XXXXXX";

	while((o = getopt(argc, argv, "")) != -1)
	{
		switch(o)
		{
			case '?':
				return _usage();
		}
	}
	if(optind < argc - 1)
		return _usage();
	if(optind == argc - 1)
		return _mktemp(argv[optind]);
	return _mktemp(template);
}
