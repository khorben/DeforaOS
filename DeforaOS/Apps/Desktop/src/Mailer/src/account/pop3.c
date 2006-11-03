/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include "account.h"


/* variables */
char const pop3_type[] = "POP3";
char const pop3_name[] = "POP3 server";

AccountConfig pop3_config[] =
{
	{ "username",	"Username",		ACT_STRING,	NULL },
	{ "password",	"Password",		ACT_PASSWORD,	NULL },
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL },
	{ "port",	"Server port",		ACT_UINT16,	NULL },
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	NULL },
	{ "delete",	"Delete read mails on server",
						ACT_BOOLEAN,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* functions */
AccountFolder ** pop3_folders(void)
{
	/* FIXME implement */
	static AccountFolder inbox =	{ AF_INBOX,	"Inbox"	};
	static AccountFolder trash =	{ AF_TRASH,	"Trash"	};
	static AccountFolder * folders[] =
	{
		&inbox, &trash, NULL
	};

	return folders;
}


AccountPlugin account_plugin =
{
	pop3_type,
	pop3_name,
	pop3_config,
	pop3_folders
};
