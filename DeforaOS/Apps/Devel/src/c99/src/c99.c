/* $id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdio.h>


/* types */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_E 0x2
#define PREFS_g 0x4
#define PREFS_s 0x8


/* c99 */
static int _c99_error(char const * message, int ret);
static int _c99_do(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile);
static int _c99(Prefs * prefs, char const * outfile, int filec, char * filev[])
{
	FILE * fp;
	int ret = 0;
	int i;

	if(outfile != NULL && (fp = fopen(outfile, "w")) == NULL)
		return _c99_error(outfile, 1);
	for(i = 0; i < filec; i++)
		ret |= _c99_do(prefs, outfile, fp, filev[i]);
	if(fp != NULL)
		fclose(fp);
	return ret;
}

static int _c99_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "c99: ");
	perror(message);
	return ret;
}

static int _c99_do(Prefs * prefs, char const * outfile, FILE * outfp,
		char * infile)
{
	FILE * infp;

	if((infp = fopen(infile, "r")) == NULL)
		return _c99_error(infile, 1);
	/* FIXME implement */
	fclose(infp);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: c99 [-c][-D name[=value]]...[-E][-g][-I directory][-L directory][-o outfile][-Ooptlevel][-s][-U name]... operand ...\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	char * outfile = NULL;
	int o;
	char oldo = '\0';

	for(; (o = getopt(argc, argv, "cD:EgI:L:o:O123sU:")) != -1; oldo = o)
		switch(o)
		{
			case 'c':
				prefs |= PREFS_c;
				break;
			case 'E':
				prefs |= PREFS_E;
				break;
			case 'g':
				prefs |= PREFS_g;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 's':
				prefs |= PREFS_s;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	if(prefs & PREFS_c && outfile != NULL && optind+1 != argc)
		return _usage();
	return _c99(&prefs, outfile, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
