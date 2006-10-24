/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include <string.h>
#include "account.h"


/* Account */
Account * account_new(char const * type, char const * name)
{
	Account * account;

	if((account = malloc(sizeof(*account))) == NULL)
		return NULL;
	if((account->name = strdup(name)) == NULL)
	{
		free(account);
		return NULL;
	}
	return account;
}


void account_delete(Account * account)
{
	free(account->name);
	free(account);
}


/* useful */
AccountFolder ** account_folders(Account * account)
{
	/* FIXME implement */
	static AccountFolder inbox = { AF_INBOX, "Inbox" };
	static AccountFolder drafts = { AF_DRAFTS, "Drafts" };
	static AccountFolder sent = { AF_SENT, "Sent" };
	static AccountFolder trash = { AF_TRASH, "Trash" };
	static AccountFolder * folders[] = { &inbox, &drafts, &sent, &trash,
		NULL };

	return folders;
}
