/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "account/account.h"
#include "../config.h"



/* constants */
#ifndef LIBDIR
# define LIBDIR PREFIX "/Libraries/"
#endif
#ifndef PLUGINDIR
# define PLUGINDIR LIBDIR "Mailer"
#endif


/* Account */
Account * account_new(char const * type, char const * name)
{
	Account * account;
	char * filename;

	if((account = malloc(sizeof(*account))) == NULL)
		return NULL;
	memset(account, 0, sizeof(*account));
	if((account->name = strdup(name)) == NULL
			|| (filename = malloc(strlen(PLUGINDIR) + strlen(type)
					+ strlen("/account/.so") + 1)) == NULL)
	{
		account_delete(account);
		return NULL;
	}
	sprintf(filename, "%s/account/%s.so", PLUGINDIR, type);
	if((account->handle = dlopen(filename, RTLD_NOW)) == NULL
			|| (account->plugin = dlsym(account->handle,
					"account_plugin")) == NULL)
	{
		fprintf(stderr, "%s%s: %s\n", "Mailer: ", filename, dlerror());
		free(filename);
		account_delete(account);
		return NULL;
	}
	free(filename);
	return account;
}


void account_delete(Account * account)
{
	if(account->handle != NULL)
		dlclose(account->handle);
	if(account->name != NULL)
		free(account->name);
	free(account);
}


/* useful */
AccountFolder ** account_folders(Account * account)
{
	return account->plugin->folders();
}


/* MailHeader ** account_folder_headers(Account * account, char const * folder)
{
} */
