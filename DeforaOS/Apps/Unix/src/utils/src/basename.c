/* basename.c */



#include <libgen.h>
#include <stdio.h>
#include <string.h>


/* main */
int main(int argc, char * argv[])
{
	char * str;

	/* parse command line arguments */
	if(argc != 2 && argc != 3)
	{
		fprintf(stderr, "%s", "Usage: basename string [suffix]\n");
		return 1;
	}

	/* basename */
	str = basename(argv[1]);
	if(argc == 3)
	{
		int slen;
		int alen;

		slen = strlen(argv[1]);
		alen = strlen(argv[2]);
		if(alen < slen && strcmp(argv[2], &str[slen - alen]) == 0)
			str[slen - alen] = '\0';
	}
	printf("%s\n", str);

	return 0;
}
