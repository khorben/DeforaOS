/* scanner.c */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "scanner.h"


/* Scanner */
static void _prompt(void);
static int _next_file_prompt(Scanner * scanner);
static int _next_file(Scanner * scanner);
static int _next_string(Scanner * scanner);
void scanner_init(Scanner * scanner, Prefs * prefs, FILE * fp,
		char const * string)
{
	if((scanner->fp = fp) == NULL)
		scanner->next = _next_string;
	else if(*prefs & PREFS_i)
		scanner->next = _next_file_prompt;
	else
		scanner->next = _next_file;
	scanner->string = string;
}

static void _prompt(void)
{
	char * prompt = NULL;

	if((prompt = getenv("PS1")) == NULL)
		prompt = "$ ";
	fprintf(stderr, "%s", prompt);
}

static void _lineno(void)
{
	static unsigned int i = 1;
	char lineno[11];
	int len = sizeof(lineno)-1;

	if(snprintf(lineno, len, "%u", i++) >= len)
		lineno[len] = '\0';
	if(setenv("LINENO", lineno, 1) != 0)
		sh_error("setenv", 0);
}

static int _next_file_prompt(Scanner * scanner)
{
	static int c = '\n';

	if(c == '\n')
	{
		_prompt();
		_lineno();
	}
	if((c = fgetc(scanner->fp)) == EOF)
		fputc('\n', stderr);
	return c;
}

static int _next_file(Scanner * scanner)
{
	int c;

	if((c = fgetc(scanner->fp)) == '\n')
		_lineno();
	return c;
}

static int _next_string(Scanner * scanner)
{
	static int pos = 0;

	if(scanner->string[pos] == '\0')
		return EOF;
	return scanner->string[pos++];
}


static Token * _next_operator(Scanner * scanner, int * c);
static Token * _next_blank(Scanner * scanner, int * c);
static Token * _next_comment(Scanner * scanner, int * c);
static Token * _next_newline(Scanner * scanner, int * c);
static Token * _next_word(Scanner * scanner, int * c);
Token * scanner_next(Scanner * scanner)
{
	Token * t;
	static int c = EOF;

#ifdef DEBUG
	fprintf(stderr, "%s", "scanner_next()\n");
#endif
	if(c == EOF && (c = scanner->next(scanner)) == EOF)
		return token_new(TC_EOI, NULL);
	_next_blank(scanner, &c);
	_next_comment(scanner, &c);
	if((t = _next_operator(scanner, &c)) != NULL
			|| (t = _next_newline(scanner, &c)) != NULL
			|| (t = _next_word(scanner, &c)) != NULL)
		return t;
	return NULL;
}

static Token * _next_operator(Scanner * scanner, int * c)
{
	unsigned int i;
	unsigned int pos = 0;

	for(i = TC_OP_AND_IF; i <= TC_OP_GREAT;)
		if(*c != sTokenCode[i][pos])
			i++;
		else if(sTokenCode[i][++pos] == '\0')
			break;
	if(i > TC_OP_GREAT)
		return NULL;
	*c = scanner->next(scanner);
	return token_new(i, NULL);
}

static Token * _next_blank(Scanner * scanner, int * c)
{
	while(*c == '\t' || *c == ' ')
		*c = scanner->next(scanner);
	return NULL;
}

static Token * _next_comment(Scanner * scanner, int * c)
{
	if(*c != '#')
		return NULL;
	*c = scanner->next(scanner);
	while(*c != EOF && *c != '\0' && *c != '\r' && *c != '\n')
		*c = scanner->next(scanner);
	return NULL;
}

static Token * _next_newline(Scanner * scanner, int * c)
{
	if(*c != '\r' && *c != '\n')
		return NULL;
	*c = EOF;
	return token_new(TC_NEWLINE, NULL);
}

static Token * _next_word(Scanner * scanner, int * c)
{
	char eow[] = "\t \r\n&|;<>";
	char * str = NULL;
	unsigned int len = 0;
	unsigned int i;
	char * p;

	for(; *c != EOF; *c = scanner->next(scanner))
	{
		for(i = 0; eow[i] != '\0' && *c != eow[i]; i++);
		if(*c == eow[i])
			break;
		if((p = realloc(str, len+2)) == NULL)
		{
			sh_error("malloc", 0);
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = *c;
	}
	if(str == NULL)
		return NULL;
	str[len] = '\0';
	/* FIXME aliases need to be replaced now; I suggest changing the
	 * scanner->next() function for a while but I don't really like it,
	 * moreover it wouldn't parse the alias into tokens... */
	return token_new(TC_TOKEN, str);
}
