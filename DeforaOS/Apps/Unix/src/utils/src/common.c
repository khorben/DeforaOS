/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#ifdef COMMON_MODE
# include <ctype.h>

/* mode
 * PRE	*mask exists
 * POST	mode is converted a mask
 * 	1	an error happened */
static int _mode_alpha(char const * mode, mode_t * mask);
static int _mode_digit(char const * mode, mode_t * mask);
static int _mode(char const * mode, mode_t * mask)
	/* FIXME need a way to tell whether to =, & or | the resulting mask */
{
	if(*mode == '\0')
		return 1;
	if(_mode_digit(mode, mask) == 0)
		return 0;
	return _mode_alpha(mode, mask);
}

static int _mode_digit(char const * mode, mode_t * mask)
{
	char const * p;

	for(p = mode; isdigit(*p) && *p < '8'; p++);
	if(*p == '\0')
	{
		*mask = strtol(mode, NULL, 8);
		return 0;
	}
	return 1;
}

static int _symbolic_mode(char const ** mode);
static int _mode_alpha(char const * mode, mode_t * mask)
{
	/* FIXME doesn't give any result */
	return _symbolic_mode(&mode);
}

static void _clause(char const ** mode);
static int _symbolic_mode(char const ** mode)
	/* clause { ',' clause } */
{
	_clause(mode);
	while(**mode == ',')
	{
		(*mode)++;
		_clause(mode);
	}
	return **mode == '\0' ? 0 : 1;
}

static void _wholist(char const ** mode);
static void _actionlist(char const ** mode);
static void _clause(char const ** mode)
	/* [wholist] actionlist */
{
	char c = **mode;

	if(c == 'u' || c == 'g' || c == 'o' || c == 'a')
		_wholist(mode);
	_actionlist(mode);
}

static void _who(char const ** mode);
static void _wholist(char const ** mode)
	/* who { who } */
{
	char c;

	_who(mode);
	for(c = ** mode; c == 'u' || c == 'g' || c == 'o' || c == 'a';
			c = **mode)
		_who(mode);
}

static void _who(char const ** mode)
	/* 'u' | 'g' | 'o' | 'a' */
{
	/* FIXME */
	(*mode)++;
}

static void _action(char const ** mode);
static void _actionlist(char const ** mode)
	/* action { action } */
{
	char c;

	for(c = **mode; c == '+' || c == '-' || c == '='; c = **mode)
		_action(mode);
}

static void _op(char const ** mode);
static void _permlist(char const ** mode);
static void _permcopy(char const ** mode);
static void _action(char const ** mode)
	/* op [(permlist | permcopy)] */
{
	char c;

	_op(mode);
	for(c = **mode; c != '\0'; c = **mode)
	{
		if(c == 'u' || c == 'g' || c == 'o')
			_permcopy(mode);
		else if(c == 'r' || c == 'w' || c == 'x' || c == 'X' || c == 's'
				|| c == 't')
			_permlist(mode);
		else
			break;
	}
}

static void _permcopy(char const ** mode)
	/* 'u' | 'g' | 'o' */
{
	/* FIXME */
	(*mode)++;
}

static void _op(char const ** mode)
	/* '+' | '-' | '=' */
{
	/* FIXME */
	(*mode)++;
}

static void _perm(char const ** mode);
static void _permlist(char const ** mode)
	/* perm { perm } */
{
	char c;

	_perm(mode);
	for(c = **mode; c == 'r' || c == 'w' || c == 'x' || c == 'X'
			|| c == 's' || c == 't'; c = **mode)
		_perm(mode);
}

static void _perm(char const ** mode)
	/* 'r' | 'w' | 'x' | 'X' | 's' | 't' */
{
	/* FIXME */
	(*mode)++;
}
#endif /* COMMON_MODE */
