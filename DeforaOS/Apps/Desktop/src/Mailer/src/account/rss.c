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
#include "account.h"


/* variables */
char const rss_type[] = "RSS";
char const rss_name[] = "RSS reader";

AccountConfig rss_config[] =
{
	{ "uri",	"Address",		ACT_STRING,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* functions */


AccountPlugin account_plugin =
{
	rss_type,
	rss_name,
	rss_config,
	NULL,
	NULL,
	NULL
};
