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
#include "Mailer/account.h"


/* variables */
static char const _rss_type[] = "RSS";
static char const _rss_name[] = "RSS reader";

static AccountConfig _rss_config[] =
{
	{ "uri",	"Address",		ACT_STRING,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* functions */


AccountPlugin account_plugin =
{
	NULL,
	_rss_type,
	_rss_name,
	NULL,
	_rss_config,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};
