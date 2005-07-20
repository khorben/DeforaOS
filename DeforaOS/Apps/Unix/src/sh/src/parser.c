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

	fprintf(stderr, "%s", "sh: ");
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fputc('\n', stderr);
	parser->token = NULL;
}

int parser_parse(Parser * parser)
{
	parser->token = scanner_next(&parser->scanner);
#ifdef DEBUG
	fprintf(stderr, "%s%p", "New token: ", parser->token);
	if(parser->token != NULL)
		fprintf(stderr, ", %d, \"%s\"", parser->token->code,
				parser->token->str);
	fputc('\n', stderr);
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
	fprintf(stderr, "%s%p", "New token: ", parser->token);
	if(parser->token != NULL)
		fprintf(stderr, ", %d, \"%s\"", parser->token->code,
				parser->token->str);
	fputc('\n', stderr);
#endif
}


/* tests */
int parser_test(Parser * parser, TokenCode tokencode)
{
	if(parser_code(parser) == tokencode)
		return 1;
	return 0;
}

int parser_test_set(Parser * parser, TokenCode codeset[])
{
	unsigned int i;

	for(i = 0; codeset[i] != TC_NULL; i++)
		if(codeset[i] == parser_code(parser))
			return 1;
	return 0;
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
		parser_error(parser, "%s%s", "expected ", sTokenCode[tokencode]);
	else
		parser_error(parser, "%s%d", "expected code #", tokencode);
	return 0;
}

int parser_check_set(Parser * parser, TokenCode codeset[])
{
	if(!parser_test_set(parser, codeset))
	{
		parser_error(parser, "expected set @%p", codeset);
		return 0;
	}
	return 1;
}

int parser_check_word(Parser * parser, char const * word)
{
	if(!parser_test_word(parser, word))
	{
		parser_error(parser, "expected word \"%s\"", word);
		return 0;
	}
	parser_scan(parser);
	return 1;
}


/* rules */
void parser_rule1(Parser * parser)
{
	int i;

	if(parser->token == NULL)
		return;
	for(i = TC_RW_IF; i <= TC_RW_IN; i++)
	{
		if(strcmp(parser->token->str, sTokenCode[i]) == 0)
		{
			parser->token->code = i;
			return;
		}
	}
	parser->token->code = TC_WORD;
}

void parser_rule7a(Parser * parser)
{
	char * p;

	for(p = parser->token->str; *p && *p != '='; p++);
	if(*p == '=')
		parser_rule7b(parser);
	else
		parser_rule1(parser);
}

void parser_rule7b(Parser * parser)
{
	char * p = parser->token->str;

	if(*p == '=')
	{
		parser->token->code = TC_WORD;
		return;
	}
	for(p++; *p && *p != '=' && ((*p >= 'a' && *p <= 'z')
			|| (*p >= 'A' && *p <= 'Z')
			|| *p == '_'); p++);
	if(!*p || *p == '=')
		parser->token->code = TC_ASSIGNMENT_WORD;
	else
		parser->token->code = TC_WORD;
}
