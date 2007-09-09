/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"


/* Parser */
/* private */
/* types */
struct _Parser
{
	char * pathname;
	FILE * fp;
	unsigned long line;
	unsigned long col;
};


/* public */
/* parser_new */
Parser * parser_new(char const * pathname)
{
	Parser * parser;

	if((parser = malloc(sizeof(*parser))) == NULL)
		return NULL;
	parser->pathname = strdup(pathname);
	parser->fp = fopen(pathname, "r");
	if(parser->pathname == NULL || parser->fp == NULL)
	{
		parser_delete(parser);
		return NULL;
	}
	parser->line = 0;
	parser->col = 0;
	return parser;
}


/* parser_delete */
void parser_delete(Parser * parser)
{
	fclose(parser->fp);
	free(parser->pathname);
	free(parser);
}


/* accessors */
char const * parser_get_filename(Parser * parser)
{
	return parser->pathname;
}
