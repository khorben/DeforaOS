/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
char const nntp_type[] = "NNTP";
char const nntp_name[] = "Newsgroups";

AccountConfig nntp_config[] =
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
	nntp_type,
	nntp_name,
	nntp_config,
	NULL,
	NULL,
	NULL,
	NULL
};
