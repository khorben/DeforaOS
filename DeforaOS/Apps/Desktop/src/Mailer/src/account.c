/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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


/* Account */
Account * account_new(char const * type, char const * name)
{
	Account * account;
	char * filename;

	if((account = malloc(sizeof(*account))) == NULL)
		return NULL;
	if((account->name = strdup(name)) == NULL
			|| (filename = malloc(strlen(PLUGINDIR) + strlen(type)
					+ strlen(name) + 6)) == NULL)
	{
		account_delete(account);
		return NULL;
	}
	sprintf(filename, "%s/%s/%s.so", PLUGINDIR, type, name);
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
	account->title = NULL;
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
AccountFolder ** account_folders(Account * account)
{
	return account->plugin->folders();
}


/* MailHeader ** account_folder_headers(Account * account, char const * folder)
{
} */
