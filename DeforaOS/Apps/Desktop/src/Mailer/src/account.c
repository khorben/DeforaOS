/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "mailer.h"
#include "account.h"


/* constants */
#define ACCOUNT "account"


/* Account */
/* private */
struct _Account
{
	char * type;
	char * title;
	int enabled;
	AccountIdentity * identity;
	void * handle;
	AccountPlugin * plugin;
	GtkTextBuffer * buffer;
};


/* public */
/* functions */
/* account_new */
Account * account_new(char const * type, char const * title,
		AccountPluginHelper * helper)
{
	Account * account;
	char const path[] = PLUGINDIR "/" ACCOUNT;
	char * filename;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_new(\"%s\", \"%s\")\n", type, title);
#endif
	if(type == NULL)
		return NULL;
	if((account = calloc(1, sizeof(*account))) == NULL)
		return NULL;
	account->type = strdup(type);
	if(title != NULL)
		account->title = strdup(title);
	if(account->type == NULL || (filename = malloc(sizeof(path)
					+ strlen(type) + 4)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		account_delete(account);
		return NULL;
	}
	snprintf(filename, sizeof(path) + strlen(type) + 4, "%s/%s.so", path,
			type);
	if((account->handle = dlopen(filename, RTLD_LAZY)) == NULL
			|| (account->plugin = dlsym(account->handle,
					"account_plugin")) == NULL)
	{
		error_set_code(1, "%s: %s", filename, dlerror());
		free(filename);
		account_delete(account);
		return NULL;
	}
	free(filename);
	account->plugin->helper = helper;
	account->enabled = 1;
	account->identity = NULL;
	account->buffer = gtk_text_buffer_new(NULL);
	return account;
}


/* account_delete */
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
	free(account->title);
	free(account->type);
	if(account->handle != NULL)
		dlclose(account->handle);
	g_object_unref(G_OBJECT(account->buffer));
	free(account);
}


/* accessors */
/* account_get_config */
AccountConfig * account_get_config(Account * account)
{
	return account->plugin->config;
}


/* account_get_enabled */
int account_get_enabled(Account * account)
{
	return account->enabled;
}


/* account_get_name */
char const * account_get_name(Account * account)
{
	return account->plugin->name;
}


/* account_get_store */
GtkListStore * account_get_store(Account * account, AccountFolder * folder)
{
	/* FIXME implement */
	if(folder == NULL)
		return NULL;
	return folder->store;
}


/* account_get_title */
char const * account_get_title(Account * account)
{
	return account->title;
}


/* account_get_type */
char const * account_get_type(Account * account)
{
	return account->type;
}


/* account_set_enabled */
void account_set_enabled(Account * account, int enabled)
{
	account->enabled = enabled ? 1 : 0;
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
	AccountConfig * p = account->plugin->config;
	char const * value;
	char * q;
	long l;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_config_load(%p)\n", (void*)config);
#endif
	if(p == NULL)
		return 0;
	for(; p->name != NULL; p++)
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
			case ACT_BOOLEAN: /* FIXME implement the rest */
			case ACT_NONE:
				break;
		}
	}
	return 0;
}


/* account_config_save */
int account_config_save(Account * account, Config * config)
{
	AccountConfig * p = account->plugin->config;
	uint16_t u16;
	char buf[6];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_config_save(%p)\n", (void*)config);
#endif
	if(config_set(config, account->title, "type", account->type) != 0)
		return 1;
	if(p == NULL)
		return 0;
	for(; p->name != NULL; p++)
	{
		switch(p->type)
		{
			case ACT_FILE:
			case ACT_STRING:
			case ACT_PASSWORD: /* FIXME scramble */
				if(config_set(config, account->title, p->name,
							p->value) != 0)
					return 1;
				break;
			case ACT_UINT16:
				u16 = (uint16_t)p->value;
				snprintf(buf, sizeof(buf), "%hu", u16);
				if(config_set(config, account->title, p->name,
							buf) != 0)
					return 1;
				break;
			case ACT_BOOLEAN: /* FIXME implement the rest */
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
	fprintf(stderr, "DEBUG: %s(%p, %p)\n", __func__, (void*)store,
			(void*)parent);
#endif
	if(account->plugin->init == NULL)
		return 0;
	return account->plugin->init(store, parent, account->buffer);
}


/* account_select */
GtkTextBuffer * account_select(Account * account, AccountFolder * folder,
		AccountMessage * message)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %p)\n", __func__, folder->name,
			(void*)message);
#endif
	if(account->plugin->select == NULL)
		return NULL;
	return account->plugin->select(folder, message);
}
