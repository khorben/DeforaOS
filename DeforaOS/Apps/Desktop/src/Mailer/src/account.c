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


/* constants */
#define ACCOUNT "account"


/* Account */
/* public */
/* functions */
Account * account_new(char const * type, char const * name)
{
	Account * account;
	char * filename;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_new(\"%s\", \"%s\")\n", type, name);
#endif
	if(type == NULL || name == NULL || strlen(name) == 0)
		return NULL;
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

	if(account->plugin != NULL)
	{
		if(account->plugin->quit != NULL)
			account->plugin->quit();
		if(account->plugin->config != NULL)
			for(p = account->plugin->config; p->name != NULL; p++)
				switch(p->type)
				{
					case ACT_STRING:
					case ACT_PASSWORD:
					case ACT_FILE:
						free(p->value);
						break;
					default:
						break;
				}
	}
	free(account->name);
	free(account->title);
	if(account->handle != NULL)
		dlclose(account->handle);
	free(account);
}


/* accessors */
/* account_get_store */
GtkListStore * account_get_store(Account * account, AccountFolder * folder)
{
	/* FIXME implement */
	if(folder == NULL)
		return NULL;
	return folder->store;
}


/* account_set_title */
int account_set_title(Account * account, char const * title)
{
	if(account->title != NULL)
		free(account->title);
	if((account->title = strdup(title != NULL ? title : "")) == NULL)
		return mailer_error(NULL, "strdup", 1);
	return 0;
}


/* useful */
/* account_config_load */
int account_config_load(Account * account, Config * config)
{
	AccountConfig * p;
	char * value;
	char * q;
	long l;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_config_load(%p)\n", config);
#endif
	for(p = account->plugin->config; p->name != NULL; p++)
	{
		if((value = config_get(config, account->title, p->name))
				== NULL)
			continue;
		switch(p->type)
		{
			case ACT_FILE:
			case ACT_STRING:
			case ACT_PASSWORD: /* FIXME unscramble */
				free(p->value);
				p->value = strdup(value);
				break;
			case ACT_UINT16:
				l = strtol(value, &q, 0);
				if(value[0] != '\0' && *q == '\0')
					p->value = (void*)l;
				break;
				/* FIXME implement the rest */
			case ACT_BOOLEAN:
			case ACT_NONE:
				break;
		}
	}
	return 0;
}


/* account_init */
int account_init(Account * account, GtkTreeStore * store, GtkTreeIter * parent)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_init(%p, %p)\n", store, parent);
#endif
	if(account->plugin->init == NULL)
		return 0;
	return account->plugin->init(store, parent);
}


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
