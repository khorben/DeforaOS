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
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "mailer.h"
#include "account/account.h"
#include "../config.h"



/* constants */
#ifndef LIBDIR
# define LIBDIR PREFIX "/Libraries/"
#endif
#ifndef PLUGINDIR
# define PLUGINDIR LIBDIR "Mailer"
#endif
#define ACCOUNT "account"


/* Account */
Account * account_new(char const * type, char const * name)
{
	Account * account;
	char * filename;

	if((account = calloc(1, sizeof(*account))) == NULL)
		return NULL;
	if((account->name = strdup(name)) == NULL
			|| (filename = malloc(strlen(PLUGINDIR ACCOUNT)
					+ strlen(name) + 6)) == NULL)
	{
		account_delete(account);
		return NULL;
	}
	sprintf(filename, "%s/%s/%s.so", PLUGINDIR, ACCOUNT, type);
	if((account->handle = dlopen(filename, RTLD_LAZY)) == NULL
			|| (account->plugin = dlsym(account->handle,
					"account_plugin")) == NULL)
	{
		fprintf(stderr, "%s%s: %s\n", "Mailer: ", filename, dlerror());
		free(filename);
		account_delete(account);
		return NULL;
	}
	free(filename);
	account->title = strdup(name);
	account->enabled = 1;
	account->identity = NULL;
	return account;
}


void account_delete(Account * account)
{
	AccountConfig * p;

	if(account->plugin->config != NULL)
		for(p = account->plugin->config; p->name != NULL; p++)
			if(p->type == ACT_STRING || p->type == ACT_PASSWORD
					|| p->type == ACT_FILE)
				free(p->value);
	free(account->name);
	free(account->title);
	dlclose(account->handle);
	free(account);
}


/* accessors */
int account_set_title(Account * account, char const * title)
{
	if(account->title != NULL)
		free(account->title);
	if((account->title = strdup(title != NULL ? title : "")) == NULL)
		return mailer_error(NULL, "strdup", 1);
	return 0;
}


/* useful */
/* account_disable */
int account_disable(Account * account)
{
	account->enabled = 0;
	return 0;
}


/* account_enable */
int account_enable(Account * account)
{
	account->enabled = 1;
	return 0;
}


/* account_folders */
AccountFolder ** account_folders(Account * account)
{
	return account->plugin->folders();
}


/* MailHeader ** account_folder_headers(Account * account, char const * folder)
{
} */
