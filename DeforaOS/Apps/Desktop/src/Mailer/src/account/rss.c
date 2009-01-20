/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* Mailer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Mailer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



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
