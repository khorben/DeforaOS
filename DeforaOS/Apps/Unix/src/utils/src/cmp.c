/* cmp.c */



#include <unistd.h>
extern int optind;
#include <stdio.h>
#include <string.h>


/* types */
typedef enum _OutputType {
	OT_NONE,
	OT_LONG,
	OT_DEFAULT
} OutputType;


/* cmp */
static int _cmp_files(OutputType ot, char * file1, char * file2,
		FILE * fp1, FILE * fp2);
static int _cmp(OutputType ot, char * file1, char * file2)
{
	FILE * fp1;
	FILE * fp2;

	if(strcmp("-", file1) == 0)
		fp1 = stdin;
	else if((fp1 = fopen(file1, "r")) == NULL)
	{
		perror("fopen");
		return 2;
	}
	if(strcmp("-", file2) == 0)
		fp2 = stdin;
	else if((fp2 = fopen(file2, "r")) == NULL)
	{
		perror("fopen");
		if(fp1 != stdin)
			fclose(fp1);
		return 2;
	}
	return _cmp_files(ot, file1, file2, fp1, fp2);
}

static int _cmp_files(OutputType ot, char * file1, char * file2,
		FILE * fp1, FILE * fp2)
{
	int c1;
	int c2;
	unsigned int byte = 1;
	unsigned int line = 1;
	int res = 0;

	while(1)
	{
		c1 = fgetc(fp1);
		c2 = fgetc(fp2);
		if(c1 == EOF && c2 == EOF)
			break;
		if(c1 == EOF || c2 == EOF)
		{
			if(ot != OT_NONE)
				fprintf(stderr, "%s%s\n",
						"cmp: EOF on ",
						c1 == EOF ? file1 : file2);
			res = 1;
			break;
		}
		if(c1 != c2)
		{
			res = 1;
			if(ot == OT_DEFAULT)
			{
				printf("%s %s differ: char %u, line %u\n",
						file1, file2, byte, line);
				break;
			}
			else if(ot == OT_LONG)
				printf("%d %o %o\n", byte, c1, c2);
			else
				break;
		}
		if(c1 == '\n')
			line++;
		byte++;
	}
	if(fp1 != stdin)
		fclose(fp1);
	if(fp2 != stdin)
		fclose(fp2);
	return res;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: cmp [-l|-s] file1 file2\n\
  -l    write the byte number and the differing byte for each difference\n\
  -s    write nothing for differing bytes\n");
	return 2;
}


/* main */
int main(int argc, char * argv[])
{
	OutputType ot = OT_DEFAULT;
	int o;

	while((o = getopt(argc, argv, "ls")) != -1)
	{
		switch(o)
		{
			case 'l':
				ot = OT_LONG;
				break;
			case 's':
				ot = OT_NONE;
				break;
			case '?':
				return _usage();
		}
	}
	if(argc - optind != 2)
		return _usage();
	return _cmp(ot, argv[optind], argv[optind+1]);
}
