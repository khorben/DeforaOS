/* tokenlist.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tokenlist.h"


/* TokenList */
/* tokenlist_new */
TokenList * tokenlist_new(void)
{
	TokenList * tokenlist;

	if((tokenlist = malloc(sizeof(TokenList))) == NULL)
	{
		perror("malloc");
		return NULL;
	}
	tokenlist->tokens = NULL;
	tokenlist->size = 0;
	return tokenlist;
}


/* tokenlist_new_from_string */
static void read_blanks(char ** string);
static void read_comments(char ** string);
static Token * read_operator(char ** string);
static Token * read_word(char ** string);
TokenList * tokenlist_new_from_string(char * string)
{
	TokenList * tokenlist;
	Token * token;

	if(string == NULL)
		return NULL;
	if((tokenlist = tokenlist_new()) == NULL)
		return NULL;
	while(*string)
	{
		read_blanks(&string);
		read_comments(&string);
		if(*string == '\0')
			break;
		if((token = read_operator(&string)) != NULL)
		{
			tokenlist_append(tokenlist, token);
			continue;
		}
		if((token = read_word(&string)) != NULL)
			tokenlist_append(tokenlist, token);
	}
	return tokenlist;
}

static void read_blanks(char ** string)
{
#ifdef DEBUG
	fprintf(stderr, "read_blanks()\n");
#endif
	while(**string == ' ' || **string == '\t')
		(*string)++;
}

static void read_comments(char ** string)
{
#ifdef DEBUG
	fprintf(stderr, "read_comments()\n");
#endif
	if(**string == '#')
		while(**string && **string != '\n')
			(*string)++;
}

static Token * read_operator(char ** string)
{
	int len;
	int i;

#ifdef DEBUG
	fprintf(stderr, "read_operator()\n");
#endif
	for(i = TC_OP_AND_IF; i <= TC_OP_GREAT; i++)
	{
		len = strlen(sTokenCode[i]);
		if(strncmp(sTokenCode[i], *string, len) == 0)
		{
			fprintf(stderr, "Operator %s\n", sTokenCode[i]);
			(*string) += len;
			return token_new(i, sTokenCode[i]);
		}
	}
	return NULL;
}

static Token * read_word(char ** string)
{
	char sDelimiters[] = " \t#|&;<>()${}";
	int len;
	char c;
	int i;
	Token * token;

#ifdef DEBUG
	fprintf(stderr, "read_word()\n");
#endif
	for(len = 0; c != '\0'; len++)
	{
		c = (*string)[len];
		for(i = 0; sDelimiters[i]; i++)
		{
			if(c == sDelimiters[i])
			{
				c = '\0';
				break;
			}
		}
		if(c == '\0')
			break;
	}
	if(len == 0)
		return NULL;
	c = (*string)[len];
	(*string)[len] = '\0';
	token = token_new(TC_TOKEN, *string);
	(*string)[len] = c;
	(*string) += len;
	return token;
}


/* tokenlist_delete */
void tokenlist_delete(TokenList * tokenlist)
{
	int i;

	if(tokenlist == NULL)
		return;
	for(i = 0; i < tokenlist->size; i++)
		token_delete(tokenlist->tokens[i]);
	free(tokenlist);
}


/* useful */
void tokenlist_append(TokenList * tokenlist, Token * token)
{
	Token ** p;
	int size;

	size = tokenlist->size + 1;
	if((p = realloc(tokenlist->tokens, size * sizeof(Token*))) == NULL)
	{
		perror("realloc");
		token_delete(token); /* FIXME can be dangerous */
		return;
	}
	tokenlist->tokens = p;
	p[tokenlist->size] = token;
	tokenlist->size = size;
}
