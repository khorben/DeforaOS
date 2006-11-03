/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include "account.h"


/* variables */
char const mbox_type[] = "MBOX";
char const mbox_name[] = "Local folder";

AccountConfig mbox_config[] =
{
	{ "mbox",	"Inbox file",		ACT_STRING,	NULL },
	{ "spool",	"Incoming mails file",	ACT_STRING,	NULL },
	{ "sent",	"Sent mails file",	ACT_STRING,	NULL },
	{ "draft",	"Draft mails file",	ACT_STRING,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* functions */
AccountFolder ** mbox_folders(void)
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
	mbox_type,
	mbox_name,
	mbox_config,
	mbox_folders
};
