/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <stdlib.h>
#include "Mailer.h"


/* variables */
static char const _nntp_type[] = "NNTP";
static char const _nntp_name[] = "Newsgroups";

static AccountConfig _nntp_config[] =
{
	{ "username",	"Username",		ACT_STRING,	NULL	},
	{ "password",	"Password",		ACT_PASSWORD,	NULL	},
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL	},
	{ "port",	"Server port",		ACT_UINT16,	119	},
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	0	},
	{ NULL,		NULL,			ACT_NONE,	NULL	}
};


/* functions */


AccountPlugin account_plugin =
{
	NULL,
	_nntp_type,
	_nntp_name,
	NULL,
	_nntp_config,
	NULL,
	NULL,
	NULL,
	NULL
};
