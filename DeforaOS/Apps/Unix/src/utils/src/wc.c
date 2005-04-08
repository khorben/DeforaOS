/* wc.c */



#include <unistd.h>
#include <ctype.h>
#include <stdio.h>


/* macros */
#define IS_SET(flags, bit) ((flags & bit) == bit)


/* types */
typedef enum _wc_flag {
	WF_ALL = 0,
	WF_C = 1,
	WF_M = 2,
	WF_L = 4,
	WF_W = 8
} wc_flag;


/* wc */
static int _wc_do(int flags,
		unsigned int * cm, unsigned int * l, unsigned int * w,
		char * filename);
static void _wc_print(int flags,
		unsigned int cm, unsigned int l, unsigned int w,
		char * filename);
static int _wc(int flags, int argc, char * argv[])
{
	int res = 0;
	unsigned int cm = 0, l = 0, w = 0;
	int i;

	if(argc == 0)
		return _wc_do(flags, &cm, &l, &w, NULL);
	if(argc == 1)
		return _wc_do(flags, &cm, &l, &w, argv[0]);
	for(i = 0; i < argc; i++)
		if(_wc_do(flags, &cm, &l, &w, argv[i]) != 0)
			res = 2;
	_wc_print(flags, cm, l, w, "total");
	return res;
}

static int _wc_do(int flags,
		unsigned int * cm, unsigned int * l, unsigned int * w,
		char * filename)
{
	FILE * fp;
	unsigned int lcm = 0, ll = 0, lw = 0;
	int c;
	int oldc = ' ';

	if(filename == NULL)
		fp = stdin;
	else if((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "%s", "wc: ");
		perror(filename);
		return 2;
	}
	while((c = fgetc(fp)) != EOF)
	{
		if(c == '\n')
			ll++;
		if(isspace(oldc) && isalnum(c))
			lw++;
		oldc = c;
		lcm++; /* FIXME */
	}
	_wc_print(flags, lcm, ll, lw, filename);
	if(filename != NULL)
		fclose(fp);
	*cm += lcm;
	*l += ll;
	*w += lw;
	return 0;
}

static void _wc_print(int flags,
		unsigned int cm, unsigned int l, unsigned int w,
		char * filename)
{
	if(flags == WF_ALL)
		printf("%d %d %d", l, w, cm);
	if(IS_SET(flags, WF_C) || IS_SET(flags, WF_M))
		printf("%d", cm);
	if(IS_SET(flags, WF_L))
		printf("%s%d", (IS_SET(flags, WF_C) || IS_SET(flags, WF_M))
				? " " : "",
				l);
	if(IS_SET(flags, WF_W))
		printf("%s%d", flags != WF_W ? " " : "", w);
	if(filename != NULL)
		printf(" %s", filename);
	printf("\n");
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: wc [-c|-m][-lw][file...]\n\
  -c    write to the standard output the number of bytes\n\
  -m    write to the standard output the number of characters\n\
  -l    write to the standard output the number of lines\n\
  -w    write to the standard output the number of words\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int flags = WF_ALL;

	while((o = getopt(argc, argv, "cmlw")) != -1)
		switch(o)
		{
			case 'c':
				if(IS_SET(flags, WF_M))
					flags = flags - WF_M;
				flags = flags | WF_C;
				break;
			case 'm':
				if(IS_SET(flags, WF_C))
					flags = flags - WF_C;
				flags = flags | WF_M;
				break;
			case 'l':
				flags = flags | WF_L;
				break;
			case 'w':
				flags = flags | WF_W;
				break;
			case '?':
				return _usage();
		}
	return _wc(flags, argc - optind, &argv[optind]);
}
