/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
AccountFolder ** rss_folders(void)
{
	/* FIXME implement */
	static AccountFolder feed = { AF_INBOX, "Feed" };
	static AccountFolder * folders[] = { &feed, NULL };

	return folders;
}


AccountPlugin account_plugin =
{
	rss_type,
	rss_name,
	rss_config,
	rss_folders
};
