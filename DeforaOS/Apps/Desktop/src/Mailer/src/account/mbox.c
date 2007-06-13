/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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
char const mbox_type[] = "MBOX";
char const mbox_name[] = "Local folder";

AccountConfig mbox_config[] =
{
	{ "mbox",	"Inbox file",		ACT_FILE,	NULL },
	{ "spool",	"Incoming mails file",	ACT_FILE,	NULL },
	{ "sent",	"Sent mails file",	ACT_FILE,	NULL },
	{ "draft",	"Draft mails file",	ACT_FILE,	NULL },
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
