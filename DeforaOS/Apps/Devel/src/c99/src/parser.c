/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#define DEBUG
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "tokenset.h"
#include "c99.h"


/* private */
/* prototypes */
static int _translation_unit(C99 * c99);
static int _external_declaration(C99 * c99);
static int _function_definition(C99 * c99);
static int _declaration_specifiers(C99 * c99);
static int _storage_class_specifier(C99 * c99);
static int _type_specifier(C99 * c99);
static int _type_qualifier(C99 * c99);
static int _function_specifier(C99 * c99);
static int _declarator(C99 * c99);
static int _declaration_list(C99 * c99);
static int _declaration(C99 * c99);
static int _compound_statement(C99 * c99);
static int _init_declarator_list(C99 * c99);


/* functions */
static int _translation_unit(C99 * c99)
	/* external-declaration { external-declaration } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(c99_scan(c99) == 0 && c99->token != NULL)
		ret |= _external_declaration(c99);
	return ret;
}


/* external_declaration */
static int _external_declaration(C99 * c99)
	/* function-definition | declaration */
{
	/* FIXME implement correctly */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return _function_definition(c99);
}


/* function_definition */
static int _function_definition(C99 * c99)
	/* declaration-specifiers declarator [ declaration-list ]
	 * compound-statement */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret |= _declaration_specifiers(c99);
	ret |= _declarator(c99);
	ret |= _declaration_list(c99); /* FIXME optional */
	ret |= _compound_statement(c99);
	return ret;
}


/* declaration-list */
static int _declaration_list(C99 * c99)
	/* declaration { declaration } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(token_in_set(c99->token, c99set_declaration))
		ret |= _declaration(c99);
	return ret;
}


/* declaration */
static int _declaration(C99 * c99)
	/* declaration-specifiers [ init-declarator-list ] */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret |= _declaration_specifiers(c99);
	ret |= _init_declarator_list(c99); /* FIXME optional */
	return ret;
}


/* declaration-specifiers */
static int _declaration_specifiers(C99 * c99)
	/* storage-class-specifier [ declaration-specifiers ]
	 * type-specifier [ declaration-specifiers ]
	 * type-qualifier [ declaration-specifiers ]
	 * function-specifier [ declaration-specifiers ] */
{
	int ret;

	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_storage_class_specifier))
	{
		ret |= _storage_class_specifier(c99);
		ret |= _declaration_specifiers(c99); /* FIXME optional */
	}
	else if(token_in_set(c99->token, c99set_type_specifier))
	{
		ret |= _type_specifier(c99);
		ret |= _declaration_specifiers(c99); /* FIXME optional */
	}
	else if(token_in_set(c99->token, c99set_type_qualifier))
	{
		ret |= _type_qualifier(c99);
		ret |= _declaration_specifiers(c99); /* FIXME optional */
	}
	else
	{
		ret |= _function_specifier(c99);
		ret |= _declaration_specifiers(c99); /* FIXME optional */
	}
	return ret;
}


/* storage-class-specifier */
static int _storage_class_specifier(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* type-specifier */
static int _type_specifier(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return c99_scan(c99);
}


/* type-qualifier */
static int _type_qualifier(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* function-specifier */
static int _function_specifier(C99 * c99)
	/* inline */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return c99_scan(c99);
}


/* declarator */
static int _declarator(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* compound-statement */
static int _compound_statement(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* init-declarator-list-opt */
static int _init_declarator_list(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* public */
/* functions */
/* useful */
/* c99_parse */
static int _parse_E(C99 * c99);

int c99_parse(C99 * c99)
{
	if(c99->flags & C99PREFS_E)
		return _parse_E(c99);
	return _translation_unit(c99);
}

static int _parse_E(C99 * c99)
{
	int ret;
	Token * token;
	int code;

	while((ret = cpp_scan(c99->cpp, &token)) == 0
			&& token != NULL)
	{
		if((code = token_get_code(token)) == CPP_CODE_META_ERROR
				|| code == CPP_CODE_META_WARNING)
			fprintf(stderr, "%s%s%s%s%u%s%s\n",
					code == CPP_CODE_META_ERROR
					? "Error" : "Warning", " in ",
					token_get_filename(token), ":",
					token_get_line(token), ": ",
					token_get_string(token));
		else if(code >= CPP_CODE_META_FIRST
				&& code <= CPP_CODE_META_LAST)
			fprintf(c99->outfp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), c99->outfp);
		token_delete(token);
	}
	return ret;
}
