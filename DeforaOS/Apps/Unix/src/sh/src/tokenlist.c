/* tokenlist.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tokenlist.h"


/* TokenList */
/* variables */
TokenList * null_tokenlist = &null_token;


/* tokenlist_new */
static void read_blanks(char ** string);
static void read_comments(char ** string);
static Token * read_operator(char ** string);
static Token * read_word(char ** string);
static void tokenlist_debug(TokenList * tokenlist);
TokenList * tokenlist_new(char * string)
{
	Token * first = NULL;
	Token * last = NULL;
	Token * p;
	TokenList * tokenlist;

	if((tokenlist = malloc(sizeof(TokenList))) == NULL)
		return NULL;
	if(string == NULL)
		return NULL;
	while(*string)
	{
		read_blanks(&string);
		read_comments(&string);
		if(*string == '\0')
			break;
		if((p = read_operator(&string)) != NULL)
		{
			if(first == NULL)
			{
				first = p;
				last = p;
			}
			else
			{
				last->next = p;
				last = p;
			}
			continue;
		}
		if((p = read_word(&string)) != NULL)
		{
			if(first == NULL)
			{
				first = p;
				last = p;
			}
			else
			{
				last->next = p;
				last = p;
			}
		}
	}
	tokenlist = &first;
	tokenlist_debug(tokenlist);
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
	char c = ' ';
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

static void tokenlist_debug(TokenList * tokenlist)
{
	Token * t = *tokenlist;

	fprintf(stderr, "tokenlist: %p, *tokenlist %p\n",
			tokenlist, *tokenlist);
	while(t != NULL)
	{
		token_debug(t);
		t = t->next;
	}
}


/* tokenlist_delete */
void tokenlist_delete(TokenList * tokenlist)
{
	Token * token;
	Token * p;

	if(tokenlist == NULL)
		return;
	token = *tokenlist;
	while(token != NULL)
	{
		p = token->next;
		token_delete(token);
		token = p;
	}
	free(tokenlist);
}


/* returns */
/* tokenlist_next */
TokenList * tokenlist_next(TokenList * tokenlist)
{
	Token * t = *tokenlist;

	if(t->next == NULL)
		return null_tokenlist;
	return &(t->next);
}

/* Token */
/* tokenlist_first_token */
Token * tokenlist_first_token(TokenList * tokenlist)
{
	return *tokenlist;
}


/* useful */
/* tokenlist_append */
/*TokenList * tokenlist_append(TokenList * tokenlist, Token * token)
{
	if(tokenlist == NULL)
		return &token;
	(*tokenlist)->next = token;
	return &token;
}*/
