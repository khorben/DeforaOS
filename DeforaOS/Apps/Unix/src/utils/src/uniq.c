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
static int _uniq_do(int opts, char * fields, unsigned int skip,
		FILE * infp, FILE * outfp);
static int _uniq(int opts, char * fields, unsigned int skip,
		char * in, char * out)
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
	ret = _uniq_do(opts, fields, skip, infp, outfp);
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

static void _do_count(int opts, unsigned int skip, char * line, FILE * fp);
static int _uniq_do(int opts, char * fields, unsigned int skip,
		FILE * infp, FILE * outfp)
{
#define BUF 80
	char * line = NULL;
	int len = 0;
	char * p;

	for(;;)
	{
		if((p = realloc(line, len + BUF + 1)) == NULL)
		{
			free(line);
			_uniq_error("malloc", 0);
			return 2;
		}
		line = p;
		if(fgets(&line[len], BUF + 1, infp) == NULL)
		{
			if(!feof(infp))
				_uniq_error("fread", 0);
			break;
		}
		for(p = &line[len]; *p != '\0' && *p != '\n'; p++);
		len += BUF;
		if(p == line + BUF)
			continue;
		if(*p == '\n')
			*p = '\0';
#ifdef DEBUG
		fprintf(stderr, "%s%s%s", "DEBUG: Got line \"", line, "\"\n");
#endif
		_do_count(opts, skip, line, outfp);
		line = NULL;
		len = 0;
	}
	_do_count(opts, skip, NULL, outfp);
	return 0;
}

static int _count_repeated(char * lastline, char * line, unsigned int skip);
static void _do_count(int opts, unsigned int skip, char * line, FILE * fp)
{
	static char * lastline = NULL;
	static unsigned int cnt = 1;

	if(lastline == NULL)
	{
		lastline = line;
		return;
	}
	if(line != NULL && _count_repeated(lastline, line, skip))
	{
		cnt++;
		return;
	}
	if(cnt == 1 && !(opts & OPTS_d)) /* line is not repeated */
		fprintf(fp, "%s%s\n", opts & OPTS_c ? "1 " : "", lastline);
	else if(cnt > 1 && !(opts & OPTS_u)) /* line is repeated */
	{
		if(opts & OPTS_c)
			fprintf(fp, "%d ", cnt);
		fprintf(fp, "%s\n", lastline);
	}
	free(lastline);
	lastline = line;
	cnt = 1;
}

/* PRE	line and lastline are valid strings
 * POST */
static int _count_repeated(char * lastline, char * line, unsigned int skip)
{
	if(strlen(lastline) < skip)
		return strlen(line) < skip;
	if(strlen(line) < skip)
		return 0;
	if(strcmp(&lastline[skip], &line[skip]) == 0)
		return 1;
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: uniq [-c|-d|-u][-f fields][-s char]\
[input_file [output_file]]\n\
  -c    precede each output line with a count of the repetitions for the line\n\
  -d    suppress the writing of lines that are not repeated\n\
  -s    ignore the first char characters when doing comparisons\n\
  -u    suppress the writing of lines that are repeated\n");
	return 1;
}

int main(int argc, char * argv[])
{
	int opts = 0;
	char * fields = NULL;
	int skip = 0;
	char * p;
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
			case 's':
				skip = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0' || skip < 0)
					return _usage();
				break;
			case 'u':
				opts |= OPTS_u;
				break;
			case 'f':
				fprintf(stderr, "%s%c%s", "uniq: -", o,
						": Not implemented yet\n");
				return _usage();
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
	return _uniq(opts, fields, skip, in, out);
}
