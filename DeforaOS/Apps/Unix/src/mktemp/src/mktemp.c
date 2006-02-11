/* mktemp.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* mktemp */
static int _mktemp_error(char * message, int ret);
static int _mktemp(char * template)
{
	int fd;

	if((template = strdup(template)) == NULL)
		return _mktemp_error("strdup", 1);
	if((fd = mkstemp(template)) == -1)
		return _mktemp_error(template, 1);
	if(close(fd) != 0)
		return _mktemp_error(template, 1);
	printf("%s\n", template);
	free(template);
	return 0;
}

static int _mktemp_error(char * message, int ret)
{
	fprintf(stderr, "%s", "mktemp: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: mktemp [-d] [template]\n\
  -d	make a directory instead of a file\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * template = "/tmp/tmp.XXXXXX";

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			case '?':
				return _usage();
		}
	if(optind < argc - 1)
		return _usage();
	if(optind == argc - 1)
		return _mktemp(argv[optind]);
	return _mktemp(template) == 0 ? 0 : 2;
}
