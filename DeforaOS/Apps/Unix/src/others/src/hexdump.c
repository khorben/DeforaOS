/* hexdump.c */



#include <unistd.h>
#include <stdio.h>
#include <ctype.h>


/* hexdump */
static int _hexdump_do(char * filename);
static int _hexdump(int filec, char * filev[])
{
	int ret = 0;
	int i;

	for(i = 0; i < filec; i++)
		ret |= _hexdump_do(filev[i]);
	return ret;
}

static int _hexdump_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "hexdump: ");
	perror(message);
	return ret;
}

static int _hexdump_do(char * filename)
{
	int ret = 0;
	FILE * fp;
	int c;
	int pos = 0;
	char buf[17] = "";

	if((fp = fopen(filename, "r")) == NULL)
		return _hexdump_error(filename, 1);
	while((c = fgetc(fp)) != EOF)
	{
		if(pos % 16 == 0)
			printf("%08x ", pos);
		else if(pos % 16 == 8)
			fputc(' ', stdout);
		printf(" %02hhx", c);
		buf[pos % 16] = isprint(c) ? c : '.';
		if(++pos % 16 == 0)
			printf("  |%s|\n", buf);
	}
	buf[pos % 16] = '\0';
	if(pos % 16 != 0)
	{
		if(pos % 16 < 8)
			fputc(' ', stdout);
		for(c = pos % 16; c < 16; c++)
			printf("%s", "   ");
		printf("  |%s|\n", buf);
	}
	if(!feof(fp))
		ret = _hexdump_error(filename, 1);
	fclose(fp);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: hexdump file...\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _hexdump(argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
