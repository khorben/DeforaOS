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
	TokenList * tokenlist = NULL;
	TokenList * p = NULL;
	Token * token;

	if(string == NULL)
		return NULL;
	while(*string)
	{
		read_blanks(&string);
		read_comments(&string);
		if(*string == '\0')
			break;
		if((token = read_operator(&string)) != NULL)
		{
			p = tokenlist_append(p, token);
			if(tokenlist == NULL)
				tokenlist = p;
			continue;
		}
		if((token = read_word(&string)) != NULL)
		{
			p = tokenlist_append(p, token);
			if(tokenlist == NULL)
				tokenlist = p;
		}
	}
	fprintf(stderr, "tokenlist %p, p %p\n", tokenlist, p);
	fprintf(stderr, "*tokenlist %p, *p %p\n", *tokenlist, *p);
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

	while(t)
	{
		fprintf(stderr, "%p\n", t);
		fprintf(stderr, "%s %s %p %p\n", sTokenCode[t->code], t->string, t, t->next);
		t = t->next;
		usleep(1000000);
	}
}


/* tokenlist_delete */
void tokenlist_delete(TokenList * tokenlist)
{
	Token * p;

	while(tokenlist)
	{
		p = *tokenlist;
		tokenlist = &(p->next);
		token_delete(p);
	}
}


/* returns */
/* tokenlist_next */
TokenList * tokenlist_next(TokenList * tokenlist)
{
	return &(*tokenlist)->next;
}

/* Token */
/* tokenlist_first_token */
Token * tokenlist_first_token(TokenList * tokenlist)
{
	return *tokenlist;
}


/* useful */
TokenList * tokenlist_append(TokenList * tokenlist, Token * token)
{
	if(tokenlist == NULL)
		return &token;
	fprintf(stderr, "append\n");
	(*tokenlist) = token;
	return &(token->next);
}
