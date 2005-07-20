/* scanner.c */



#include <stdlib.h>
#include "scanner.h"


/* Scanner */
void scanner_init(Scanner * scanner, FILE * fp, char const * string)
{
	if((scanner->fp = fp) != NULL)
		scanner->str = NULL;
	else
		scanner->str = string;
	scanner->c = EOF;
}


/* useful */
static void _next_char(Scanner * scanner);
static Token * _read_operator(Scanner * scanner);
static Token * _read_blank(Scanner * scanner);
static Token * _read_comment(Scanner * scanner);
static Token * _read_word(Scanner * scanner);
Token * scanner_next(Scanner * scanner)
{
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s", "scanner_next()\n");
#endif
	if(scanner->c == EOF)
		_next_char(scanner);
	if(scanner->c == '\n') /* FIXME: only if interactive */
		scanner->c = EOF;
	if(scanner->c == EOF)
		return token_new(TC_EOI, NULL);
	/* '\' '\'' '"' */
	/* '$' '`' */
	if((t = _read_operator(scanner))
			|| _read_blank(scanner)
			|| _read_comment(scanner)
			|| (t = _read_word(scanner)))
		return t;
	return NULL;
}

static void _next_char(Scanner * scanner)
{
	static unsigned int pos = 0;

#ifdef DEBUG
	fprintf(stderr, "%s", "_next_char()\n");
#endif
	if(scanner->fp != NULL)
		scanner->c = fgetc(scanner->fp);
	else
	{
		if(scanner->str[pos] == '\0')
			scanner->c = EOF;
		else
			scanner->c = scanner->str[pos++];
	}
}

static Token * _read_operator(Scanner * scanner)
{
	int i;

	for(i = TC_OP_AND_IF; i <= TC_OP_GREAT; i++)
		if(scanner->c == sTokenCode[i][0])
			break;
	if(i > TC_OP_GREAT)
		return NULL;
	_next_char(scanner);
	return token_new(i, NULL);
}

static Token * _read_blank(Scanner * scanner)
{
	while(scanner->c == '\t' || scanner->c == ' ')
		_next_char(scanner);
	return NULL;
}

static Token * _read_comment(Scanner * scanner)
{
	if(scanner->c != '#')
		return NULL;
	_next_char(scanner);
	while(scanner->c != EOF || scanner->c != '\0'
			|| scanner->c != '\r' || scanner->c != '\n')
		_next_char(scanner);
	return NULL;
}

static Token * _read_word(Scanner * scanner)
{
	char eow[] = " \r\n&|;<>";
	char * str = NULL;
	unsigned int len = 0;
	unsigned int i;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "%s", "read_word()\n");
#endif
	for(; scanner->c != EOF; _next_char(scanner))
	{
		for(i = 0; eow[i] != '\0' && scanner->c != eow[i]; i++);
		if(scanner->c == eow[i])
			break;
		if((p = realloc(str, len+2)) == NULL)
		{
			perror("malloc"); /* FIXME */
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = scanner->c;
	}
	if(str == NULL)
		return NULL;
	str[len] = '\0';
	return token_new(TC_TOKEN, str);
}
