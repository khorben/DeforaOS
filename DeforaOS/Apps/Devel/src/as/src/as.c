/* as.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
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


/* useful */
/* as_error */
int as_error(char * msg, int ret)
{
	fprintf(stderr, "%s", "as: ");
	perror(msg);
	return ret;
}


/* plug-ins helpers */
void * as_plugin_new(char * type, char * name)
{
	char * filename;
	void * handle;

#ifndef PREFIX
# define PREFIX "."
#endif
	if((filename = malloc(strlen(PREFIX) + 1 + strlen(type) + 1
					+ strlen(name) + strlen(".so") + 1))
				== NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	sprintf(filename, "%s/%s/%s%s", PREFIX, type, name, ".so");
	if((handle = dlopen(filename, RTLD_NOW)) == NULL)
		as_error(filename, 0);
	free(filename);
	return handle;
}


/* as_plugin_delete */
void as_plugin_delete(void * plugin)
{
	dlclose(plugin);
}


/* usage */
static unsigned int _usage(void)
{
	fprintf(stderr, "%s", "Usage: as [-o file] file\n"
"  -a    target architecture (default: guessed)\n"
"  -f    target file format (default: elf)\n"
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
