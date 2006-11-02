/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include "account.h"


/* variables */
char const imap4_name[] = "IMAP4 server";

AccountConfig imap4_config[] =
{
	{ "username",	"Username",		ACT_STRING,	NULL },
	{ "password",	"Password",		ACT_PASSWORD,	NULL },
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL },
	{ "port",	"Server port",		ACT_UINT16,	NULL },
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	NULL },
	{ "sent",	"Sent mails folder",	ACT_NONE,	NULL },
	{ "draft",	"Draft mails folder",	ACT_NONE,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* functions */
AccountFolder ** imap4_folders(void)
{
	/* FIXME implement */
	static AccountFolder inbox =	{ AF_INBOX,	"Inbox"	};
	static AccountFolder drafts =	{ AF_DRAFTS,	"Drafts"};
	static AccountFolder sent =	{ AF_SENT,	"Sent"	};
	static AccountFolder trash =	{ AF_TRASH,	"Trash"	};
	static AccountFolder * folders[] =
	{
		&inbox, &drafts, &sent, &trash, NULL
	};

	return folders;
}


AccountPlugin account_plugin =
{
	imap4_name,
	imap4_config,
	imap4_folders
};
