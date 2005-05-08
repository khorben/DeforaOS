/* as.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "parser.h"
#include "as.h"


/* as */
static int as(int prefs, char * arch, char * format, char * infile,
		char * outfile)
{
	FILE * infp;
	Code * code;
	int ret;

	if((infp = fopen(infile, "r")) == NULL)
		return as_error(infile, 2);
	if((code = code_new(arch, format, outfile)) == NULL)
		ret = 2;
	else
	{
		ret = parser(prefs, code, infile, infp);
		code_delete(code, ret);
	}
	fclose(infp);
	return ret;
}

int as_error(char * msg, int ret)
{
	fprintf(stderr, "%s", "as: ");
	perror(msg);
	return ret;
}


/* usage */
static unsigned int _usage(void)
{
	fprintf(stderr, "%s", "Usage: as [-o file] file\n"
"  -a    target architecture (default: guessed)\n"
"  -f    target file format (default: ELF)\n"
"  -o    filename to use for output (default: \"" AS_FILENAME_DEFAULT "\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * outfile = AS_FILENAME_DEFAULT;
	char * arch = NULL;
	char * format = NULL;

	while((o = getopt(argc, argv, "a:f:o:")) != -1)
	{
		switch(o)
		{
			case 'a':
				arch = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			default:
				return _usage();
		}
	}
	if(argc - optind != 1)
		return _usage();
	return as(0, arch, format, argv[optind], outfile) ? 2 : 0;
}
