/* parser.c */



#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "grammar.h"
#include "parser.h"


/* Parser */
Parser * parser_new(FILE * fp)
{
	Parser * parser;

	if((parser = malloc(sizeof(Parser))) == NULL)
		return NULL;
	scanner_init(&parser->scanner, fp, NULL);
	parser->token = NULL;
	return parser;
}

Parser * parser_new_from_string(char const * string)
{
	Parser * parser;

	if((parser = malloc(sizeof(Parser))) == NULL)
		return NULL;
	scanner_init(&parser->scanner, NULL, string);
	parser->token = NULL;
	return parser;
}


void parser_delete(Parser * parser)
{
	if(parser->token != NULL)
		token_delete(parser->token);
	free(parser);
}


/* returns */
TokenCode parser_code(Parser * parser)
{
	if(parser->token == NULL)
		return TC_NULL;
	return parser->token->code;
}


/* useful */
void parser_error(Parser * parser, char const * format, ...)
{
	va_list vl;

	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	parser->token = NULL;
}

int parser_parse(Parser * parser)
{
	parser->token = scanner_next(&parser->scanner);
#ifdef DEBUG
	fprintf(stderr, "New token: %p", parser->token);
	if(parser->token != NULL)
		fprintf(stderr, ", %d, \"%s\"",
				parser->token->code, parser->token->str);
	fprintf(stderr, "\n");
#endif
	if(parser_code(parser) == TC_EOI)
		return -1;
	return complete_command(parser);
}

void parser_scan(Parser * parser)
{
	if(parser->token == NULL)
		return;
	token_delete(parser->token);
	parser->token = scanner_next(&parser->scanner);
#ifdef DEBUG
	fprintf(stderr, "New token: %p\n", parser->token);
#endif
}


/* tests */
int parser_test(Parser * parser, TokenCode tokencode)
{
	if(parser_code(parser) == tokencode)
		return 1;
	return 0;
}

static void _parser_distinct(Parser * parser);
int parser_test_set(Parser * parser, TokenCode codeset[])
{
	unsigned int i;

	for(i = 0; codeset[i] != TC_NULL; i++)
	{
		if(codeset[i] == TC_WORD && parser_code(parser) == TC_TOKEN)
			_parser_distinct(parser);
		if(codeset[i] == parser_code(parser))
			return 1;
	}
	return 0;
}

static void _parser_distinct(Parser * parser)
{
	int i;

	for(i = TC_RW_IF; i <= TC_RW_IN; i++)
	{
		if(strcmp(parser->token->str, sTokenCode[i]) == 0)
		{
			parser->token->code = i;
			return;
		}
	}
	/* FIXME assignment words */
	parser->token->code = TC_WORD;
}

int parser_test_word(Parser * parser, char const * word)
{
	if(parser_code(parser) != TC_WORD
			|| strcmp(word, parser->token->str) != 0)
		return 0;
	return 1;
}


/* checks */
int parser_check(Parser * parser, TokenCode tokencode)
{
	if(parser_test(parser, tokencode))
	{
		parser_scan(parser);
		return 1;
	}
	if(sTokenCode[tokencode] != NULL)
		parser_error(parser, "%s%s", "Expected ", sTokenCode[tokencode]);
	else
		parser_error(parser, "%s%d", "Expected code #", tokencode);
	return 0;
}

int parser_check_set(Parser * parser, TokenCode codeset[])
{
	if(!parser_test_set(parser, codeset))
	{
		parser_error(parser, "Expected set @%p", codeset);
		return 0;
	}
	return 1;
}

int parser_check_word(Parser * parser, char const * word)
{
	if(!parser_test_word(parser, word))
	{
		parser_error(parser, "Expected word \"%s\"", word);
		return 0;
	}
	parser_scan(parser);
	return 1;
}
