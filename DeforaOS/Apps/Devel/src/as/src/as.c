/* as.c */



#include <unistd.h>
#include <stdio.h>
#include "parser.h"
#include "as.h"


/* as */
static int _as_error(char * msg, int ret);
static int _as_do(int prefs, char * infile, FILE * infp,
		char * outfile, FILE * outfp);
static int as(int prefs, char * infile, char * outfile)
{
	FILE * infp;
	FILE * outfp;
	int ret;

	if((infp = fopen(infile, "r")) == NULL)
		return _as_error(infile, 1);
	if(outfile == NULL)
		outfile = AS_FILENAME_DEFAULT;
	if((outfp = fopen(outfile, "w")) == NULL)
	{
		fclose(infp);
		return _as_error(outfile, 1);
	}
	ret = _as_do(prefs, infile, infp, outfile, outfp);
	fclose(infp);
	fclose(outfp);
	return ret;
}

static int _as_error(char * msg, int ret)
{
	fprintf(stderr, "%s", "as: ");
	perror(msg);
	return ret;
}

static int _as_do(int prefs, char * infile, FILE * infp,
		char * outfile, FILE * outfp)
{
	return parser(prefs, infile, infp, outfile, outfp);
}


/* usage */
static unsigned int _usage(void)
{
	fprintf(stderr, "%s", "Usage: as [-o file] file\n"
"  -o    filename to use for output (default: \"" AS_FILENAME_DEFAULT "\")\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * outfile = NULL;

	while((o = getopt(argc, argv, "o:")) != -1)
	{
		switch(o)
		{
			case 'o':
				outfile = optarg;
				break;
			default:
				return _usage();
		}
	}
	if(argc - optind != 1)
		return _usage();
	return as(0, argv[optind], outfile) ? 2 : 0;
}
