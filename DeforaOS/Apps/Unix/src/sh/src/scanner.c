/* scanner.c */



#include <stdlib.h>
#include "scanner.h"


/* Scanner */
void scanner_init(Scanner * scanner, FILE * fp, char const * string)
{
	if(fp != NULL)
	{
		scanner->fp = fp;
		scanner->str = NULL;
	}
	else
	{
		scanner->fp = NULL;
		scanner->str = string;
	}
	scanner->c = EOF;
}


/* useful */
static void _scanner_next_char(Scanner * scanner);
static void _scanner_next_char(Scanner * scanner);
static Token * _read_operator(Scanner * scanner);
static void _read_blank(Scanner * scanner);
static void _read_comment(Scanner * scanner);
static Token * _read_word(Scanner * scanner);
Token * scanner_next(Scanner * scanner)
{
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s", "scanner_next()\n");
#endif
	if(scanner->c == EOF)
		_scanner_next_char(scanner);
	if(scanner->c == EOF)
		return token_new(TC_EOI, NULL);
	/* '\' '\'' '"' */
	/* '$' '`' */
	if((t = _read_operator(scanner)) != NULL)
		return t;
	_read_blank(scanner);
	_read_comment(scanner);
	if((t = _read_word(scanner)) != NULL)
		return t;
	if(scanner->c == '\n')
		scanner->c = EOF;
	return NULL;
/*	if(scanner->c == '\0' || scanner->c == '\r' || scanner->c == '\n')
		return NULL;*/
}

static void _scanner_next_char(Scanner * scanner)
{
	static unsigned int pos = 0;

#ifdef DEBUG
	fprintf(stderr, "%s", "_scanner_next_char()\n");
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
	{
		if(scanner->c == sTokenCode[i][0])
			break;
	}
	if(i > TC_OP_GREAT)
		return NULL;
	_scanner_next_char(scanner);
	return token_new(i, NULL);
}

static void _read_blank(Scanner * scanner)
{
	while(scanner->c == '\t' || scanner->c == ' ')
		_scanner_next_char(scanner);
}

static void _read_comment(Scanner * scanner)
{
	if(scanner->c != '#')
		return;
	_scanner_next_char(scanner);
	while(scanner->c != EOF || scanner->c != '\0'
			|| scanner->c != '\r' || scanner->c != '\n')
		_scanner_next_char(scanner);
}

static Token * _read_word(Scanner * scanner)
{
	char eow[] = " \r\n;";
	char * str = NULL;
	char * p;
	int len = 1;
	int i;

	for(;;)
	{
		if(scanner->c == EOF)
			break;
		for(i = 0; eow[i] != '\0' && scanner->c != eow[i]; i++);
		if(scanner->c == eow[i])
			break;
		if((p = realloc(str, len+1)) == NULL)
		{
			free(str);
			return NULL;
		}
		str = p;
		str[len-1] = scanner->c;
		len++;
		_scanner_next_char(scanner);
	}
	if(str == NULL)
		return NULL;
	str[len-1] = '\0';
	return token_new(TC_TOKEN, str);
}
