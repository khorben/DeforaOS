/* uniq.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define OPTS_c 1
#define OPTS_d 2
#define OPTS_u 4


/* uniq */
/* PRE	if in == NULL then out == NULL too
 * POST
 * 	0	success
 * 	else	error(s) occured */
static int _uniq_error(char * message, int ret);
static int _uniq_do(int opts, char * fields, int chars,
		FILE * infp, FILE * outfp);
static int _uniq(int opts, char * fields, int chars, char * in, char * out)
{
	FILE * infp = stdin;
	FILE * outfp = stdout;
	int ret;

	if(in != NULL && (infp = fopen(in, "r")) == NULL)
		return _uniq_error(in, 2);
	if(out != NULL && (outfp = fopen(out, "w")) == NULL)
	{
		fclose(infp);
		return _uniq_error(out, 2);
	}
	ret = _uniq_do(opts, fields, chars, infp, outfp);
	if(in == NULL)
	{
		fclose(infp);
		if(out == NULL)
			fclose(outfp);
	}
	return ret;
}

static int _uniq_error(char * message, int ret)
{
	fprintf(stderr, "%s", "uniq: ");
	perror(message);
	return ret;
}

static int _uniq_do(int opts, char * fields, int chars,
		FILE * infp, FILE * outfp)
{
	char * lastline = NULL;
	char * line = NULL;
	int len = 0;
	char * p;
	unsigned int cnt = 1;
#define BUF 80

	for(;;)
	{
		if((p = realloc(line, sizeof(char) * (len + BUF))) == NULL)
		{
			free(lastline);
			free(line);
			return _uniq_error("malloc", 2);
		}
		line = p;
		if(fgets(&line[len], BUF, infp) == NULL) /* FIXME */
		{
			free(lastline);
			free(line);
			return _uniq_error("fread", 2);
		}
		for(p = &line[len]; *p != '\0' && *p != '\n'; p++);
		if(*p == '\0')
		{
			len += BUF;
			continue;
		}
		*p = '\0';
		if(lastline == NULL)
		{
			lastline = line;
			line = NULL;
			len = 0;
			continue;
		}
		if(strcmp(lastline, line) == 0)
		{
			cnt++;
			continue;
		}
		if(cnt == 1 && !(opts & OPTS_d)) /* line is not repeated */
			printf("%s%s\n", opts & OPTS_c ? "1 " : "", lastline);
		else if(cnt > 1 && !(opts & OPTS_u)) /* line is repeated */
		{
			if(opts & OPTS_c)
				printf("%d ", cnt);
			printf("%s\n", lastline);
		}
		free(lastline);
		lastline = line;
		line = NULL;
		len = 0;
		cnt = 1;
	}
	free(lastline);
	free(line);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: uniq [-c|-d|-u][-f fields][-s char]\
[input_file [output_file]]\n");
	return 1;
}

int main(int argc, char * argv[])
{
	int opts = 0;
	char * fields = NULL;
	int chars = 0;
	char * in = NULL;
	char * out = NULL;
	int o;

	while((o = getopt(argc, argv, "cduf:s:")) != -1)
	{
		switch(o)
		{
			case 'c':
				opts |= OPTS_c;
				break;
			case 'd':
				opts |= OPTS_d;
				break;
			case 'u':
				opts |= OPTS_u;
				break;
			default:
				return _usage();
		}
	}
	if(argc - optind >= 1)
	{
		in = argv[optind];
		if(argc - optind == 2)
			out = argv[optind+1];
		else if(argc - optind > 2)
			return _usage();
	}
	return _uniq(opts, fields, chars, in, out);
}
