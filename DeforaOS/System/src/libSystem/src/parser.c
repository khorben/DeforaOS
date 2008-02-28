/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* TODO:
 * - switch parser_scan() and parser_scan_filter() to avoid confusion */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "System.h"
#include "token.h"


/* Parser */
/* private */
/* types */
typedef struct _ParserFilterData ParserFilterData;
typedef struct _ParserCallbackData ParserCallbackData;

struct _Parser
{
	/* parsing sources */
	char * filename;
	FILE * fp;

	/* tracking the position */
	unsigned int line;
	unsigned int col;
	int last;
	unsigned int lookahead;

	ParserFilter scanner;

	ParserFilterData * filters;
	size_t filters_cnt;
	ParserCallbackData * callbacks;
	size_t callbacks_cnt;
};

struct _ParserFilterData
{
	ParserFilter filter;
	void * data;
};

struct _ParserCallbackData
{
	ParserCallback callback;
	void * data;
};


/* prototypes */
static int _parser_scanner_file(int * c, void * data);


/* functions */
/* parser_scan_filter */
int parser_scan_filter(Parser * parser)
{
	int c;
	size_t i;
	ParserFilterData * pfd;
	int l;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_scan_filter(%p)\n", parser);
#endif
	if(parser->lookahead)
		parser->lookahead--;
	else if(parser->scanner(&c, parser) != 0)
		return EOF; /* FIXME report error */
	for(i = 0; i < parser->filters_cnt; i++)
	{
		pfd = &parser->filters[i];
		if((l = pfd->filter(&c, pfd->data)) < 0)
			return EOF;
		parser->lookahead += l;
	}
	return c;
}


/* parser_scanner_file */
static int _parser_scanner_file(int * c, void * data)
{
	Parser * parser = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_scanner_file()\n");
#endif
	if(parser->last == '\n')
	{
		parser->line++;
		parser->col = 1;
	}
	else if(parser->last != EOF)
		parser->col++;
	if((*c = fgetc(parser->fp)) == EOF
			&& !feof(parser->fp))
		return -1;
	parser->last = *c;
	return 0;
}


/* public */
/* functions */
/* parser_new */
Parser * parser_new(char const * pathname)
{
	Parser * parser;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_new(\"%s\")\n", pathname);
#endif
	if((parser = object_new(sizeof(*parser))) == NULL)
		return NULL;
	parser->filename = strdup(pathname);
	parser->fp = fopen(pathname, "r");
	parser->line = 1;
	parser->col = 1;
	parser->last = EOF;
	parser->lookahead = 0;
	parser->scanner = _parser_scanner_file;
	parser->filters = NULL;
	parser->filters_cnt = 0;
	parser->callbacks = NULL;
	parser->callbacks_cnt = 0;
	if(parser->filename == NULL
			|| parser->fp == NULL)
	{
		parser_delete(parser);
		return NULL;
	}
	return parser;
}


/* parser_delete */
int parser_delete(Parser * parser)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_delete(%p) \"%s\"\n", parser,
			parser->filename);
#endif
	if(parser->fp != NULL
			&& fclose(parser->fp) != 0)
		ret = error_set_code(1, "%s: %s", parser->filename,
				strerror(errno));
	free(parser->filename);
	free(parser->filters);
	free(parser->callbacks);
	object_delete(parser);
	return ret;
}


/* accessors */
/* parser_get_filename */
char const * parser_get_filename(Parser * parser)
{
	return parser->filename;
}


/* parser_get_token */
int parser_get_token(Parser * parser, Token ** token)
{
	int ret = 0; /* XXX not sure */
	size_t i;
	ParserCallbackData * pcd;
	int c;

	if((*token = token_new(parser->filename, parser->line, parser->col))
			== NULL)
		return 1;
	c = (parser->last == EOF) ? parser_scan_filter(parser) : parser->last;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_get_token() %c\n", c);
#endif
	for(i = 0; i < parser->callbacks_cnt; i++)
	{
		pcd = &parser->callbacks[i];
		if((ret = pcd->callback(parser, *token, c, pcd->data)) <= 0)
			break;
	}
	if(ret == 0 && i != parser->callbacks_cnt)
		return 0; /* there is a token and no error */
	token_delete(*token);
	*token = NULL;
	return (ret >= 0 && c == EOF) ? 0 : 1;
}


/* useful */
/* parser_add_callback */
int parser_add_callback(Parser * parser, ParserCallback callback, void * data)
{
	ParserCallbackData * p = parser->callbacks;

	if((p = realloc(p, sizeof(*p) * (parser->callbacks_cnt + 1))) == NULL)
		return 1;
	parser->callbacks = p;
	p = &parser->callbacks[parser->callbacks_cnt++];
	p->callback = callback;
	p->data = data;
	return 0;
}


/* parser_add_filter */
int parser_add_filter(Parser * parser, ParserFilter filter, void * data)
{
	ParserFilterData * p = parser->filters;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_add_filter(%p, %p, %p)\n", parser,
			filter, data);
#endif
	if((p = realloc(p, sizeof(*p) * (parser->filters_cnt + 1))) == NULL)
		return 1;
	parser->filters = p;
	p = &parser->filters[parser->filters_cnt++];
	p->filter = filter;
	p->data = data;
	return 0;
}


/* parser_delete_callback */
int parser_delete_callback(Parser * parser, ParserCallback callback)
	/* FIXME untested */
{
	size_t i;
	ParserCallbackData * p;

	for(i = 0; i < parser->callbacks_cnt; i++)
		if(parser->callbacks[i].callback == callback)
			break;
	if(i != parser->callbacks_cnt)
		return 1;
	p = &parser->callbacks[i];
#ifdef DEBUG
	fprintf(stderr, "DEBUG: memmove(%p, %p, %zu)\n", p, p + 1,
			sizeof(*p) * (parser->callbacks_cnt - i));
#endif
	memmove(p, p + 1, sizeof(*p) * (parser->callbacks_cnt - i));
	parser->callbacks_cnt--;
	return 0;
}


/* parser_delete_filter */
int parser_delete_filter(Parser * parser, ParserFilter filter)
	/* FIXME untested */
{
	size_t i;
	ParserFilterData * p;

	for(i = 0; i < parser->filters_cnt; i++)
		if(parser->filters[i].filter == filter)
			break;
	if(i != parser->filters_cnt)
		return 1;
	p = &parser->filters[i];
#ifdef DEBUG
	fprintf(stderr, "DEBUG: memmove(%p, %p, %zu)\n", p, p + 1,
			sizeof(*p) * (parser->callbacks_cnt - i));
#endif
	memmove(p, p + 1, sizeof(*p) * (parser->filters_cnt - i));
	parser->filters_cnt--;
	return 0;
}


/* parser_scan */
int parser_scan(Parser * parser)
{
	int c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: parser_scan(%p)\n", parser);
#endif
	if(parser->scanner(&c, parser) != 0)
		return EOF;
	return c;
}
