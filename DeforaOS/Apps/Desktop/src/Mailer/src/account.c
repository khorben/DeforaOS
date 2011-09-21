/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "folder.h"
#include "mailer.h"
#include "message.h"
#include "account.h"
#include "../config.h"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif

#define ACCOUNT		"account"


/* Account */
/* private */
/* types */
struct _Account
{
	Mailer * mailer;

	char * type;
	char * title;
	GtkTreeStore * store;
	GtkTreeRowReference * row;
	Plugin * plugin;
	AccountPlugin * account;

	int enabled;
	AccountIdentity * identity;
	AccountPluginHelper helper;
};


/* prototypes */
/* accessors */
static gboolean _account_get_iter(Account * account, GtkTreeIter * iter);

/* useful */
static int _account_helper_error(Account * account, char const * message,
		int ret);
static void _account_helper_status(Account * account, char const * format, ...);
static char * _account_helper_authenticate(Account * account,
		char const * message);
static int _account_helper_confirm(Account * account, char const * message);
static Folder * _account_helper_folder_new(Account * account,
		AccountFolder * folder, Folder * parent, FolderType type,
		char const * name);
static void _account_helper_folder_delete(Folder * folder);
static Message * _account_helper_message_new(Account * account, Folder * folder,
		AccountMessage * message);
static void _account_helper_message_delete(Message * message);
static int _account_helper_message_set_body(Message * message, char const * buf,
		size_t cnt, int append);


/* constants */
static const AccountPluginHelper _account_plugin_helper =
{
	NULL,
	_account_helper_error,
	_account_helper_status,
	_account_helper_authenticate,
	_account_helper_confirm,
	_account_helper_folder_new,
	_account_helper_folder_delete,
	_account_helper_message_new,
	_account_helper_message_delete,
	message_set_header,
	_account_helper_message_set_body
};


/* public */
/* functions */
/* account_new */
Account * account_new(Mailer * mailer, char const * type, char const * title,
		GtkTreeStore * store)
{
	Account * account;
	GtkTreeIter iter;
	GtkTreePath * path;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: account_new(%p, \"%s\", \"%s\", %p)\n",
			(void*)mailer, type, title, (void*)store);
#endif
	/* FIXME copy the AccountConfig structure */
	if(type == NULL)
		return NULL;
	if((account = object_new(sizeof(*account))) == NULL)
		return NULL;
	memset(account, 0, sizeof(*account));
	account->mailer = mailer;
	account->type = string_new(type);
	if(title != NULL)
		account->title = string_new(title);
	account->store = store;
	account->plugin = plugin_new(LIBDIR, PACKAGE, "account", type);
	account->account = (account->plugin != NULL)
		? plugin_lookup(account->plugin, "account_plugin") : NULL;
	if(account->type == NULL || account->plugin == NULL
			|| account->account == NULL)
	{
		account_delete(account);
		return NULL;
	}
	if(store != NULL)
	{
		theme = gtk_icon_theme_get_default();
		pixbuf = gtk_icon_theme_load_icon(theme, "mailer-accounts", 16,
				0, NULL);
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, MFC_ACCOUNT, account, MFC_ICON,
				pixbuf, MFC_NAME, title, -1);
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
		account->row = gtk_tree_row_reference_new(GTK_TREE_MODEL(store),
				path);
		gtk_tree_path_free(path);
	}
	memcpy(&account->helper, &_account_plugin_helper,
			sizeof(account->helper));
	account->helper.account = account;
	account->account->helper = &account->helper;
	account->enabled = 1;
	account->identity = NULL;
	return account;
}


/* account_delete */
void account_delete(Account * account)
{
	AccountConfig * p;

	if(account->row != NULL)
		gtk_tree_row_reference_free(account->row);
	if(account->plugin != NULL)
	{
		if(account->account->destroy != NULL)
			account->account->destroy(account->account);
		if(account->account->config != NULL)
			for(p = account->account->config; p->name != NULL; p++)
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
	string_delete(account->title);
	string_delete(account->type);
	if(account->plugin != NULL)
		plugin_delete(account->plugin);
	object_delete(account);
}


/* accessors */
/* account_get_config */
AccountConfig * account_get_config(Account * account)
{
	return account->account->config;
}


/* account_get_enabled */
int account_get_enabled(Account * account)
{
	return account->enabled;
}


/* account_get_folders */
GtkTreeStore * account_get_folders(Account * account)
{
	return account->store;
}


/* account_get_name */
char const * account_get_name(Account * account)
{
	return account->account->name;
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
	AccountConfig * p = account->account->config;
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
			case ACT_PASSWORD:
				/* FIXME unscramble */
			case ACT_FILE:
			case ACT_STRING:
				free(p->value);
				p->value = strdup(value);
				break;
			case ACT_UINT16:
				l = strtol(value, &q, 0);
				if(value[0] != '\0' && *q == '\0')
					p->value = (void*)l;
				break;
			case ACT_BOOLEAN:
				p->value = (strcmp(value, "yes") == 0
						|| strcmp(value, "1") == 0)
					? (void *)1 : NULL;
				break;
			case ACT_NONE:
			case ACT_SEPARATOR:
				break;
		}
	}
	return 0;
}


/* account_config_save */
int account_config_save(Account * account, Config * config)
{
	AccountConfig * p = account->account->config;
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
			case ACT_PASSWORD:
				/* FIXME scramble */
			case ACT_FILE:
			case ACT_STRING:
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
			case ACT_BOOLEAN:
				if(config_set(config, account->title, p->name,
							(p->value != NULL) ? "1"
							: "0") != 0)
					return 1;
				break;
			case ACT_NONE:
			case ACT_SEPARATOR:
				break;
		}
	}
	return 0;
}


/* account_init */
int account_init(Account * account)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void*)account);
#endif
	if(account->account->init == NULL)
		return 0;
	return account->account->init(account->account);
}


/* account_select */
GtkTextBuffer * account_select(Account * account, Folder * folder,
		Message * message)
{
	AccountFolder * af;
	AccountMessage * am = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", %p)\n", __func__,
			account_get_name(account), folder_get_name(folder),
			(void *)message);
#endif
	if((af = folder_get_data(folder)) == NULL)
		return NULL;
	if(message != NULL && (am = message_get_data(message)) == NULL)
		return NULL;
	if(account->account->refresh != NULL
			&& account->account->refresh(account->account, af, am)
			!= 0)
		return NULL;
	return (message != NULL) ? message_get_body(message) : NULL;
}


/* account_select_source */
GtkTextBuffer * account_select_source(Account * account, Folder * folder,
		Message * message)
{
	GtkTextBuffer * ret;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %p)\n", __func__,
			folder_get_name(folder), (void*)message);
#endif
	if(account->account->get_source == NULL)
		return NULL;
	ret = gtk_text_buffer_new(NULL);
	if((p = account->account->get_source(account->account,
					folder_get_data(folder),
					message_get_data(message))) != NULL)
	{
		gtk_text_buffer_set_text(ret, p, -1);
		free(p);
	}
	return ret;
}


/* private */
/* functions */
/* accessors */
/* account_get_iter */
static gboolean _account_get_iter(Account * account, GtkTreeIter * iter)
{
	GtkTreePath * path;

	if((path = gtk_tree_row_reference_get_path(account->row)) == NULL)
		return FALSE;
	return gtk_tree_model_get_iter(GTK_TREE_MODEL(account->store), iter,
			path);
}


/* useful */
/* account_helper_error */
static int _account_helper_error(Account * account, char const * message,
		int ret)
{
	Mailer * mailer = (account != NULL) ? account->mailer : NULL;

	return mailer_error(mailer, message, ret);
}


/* account_helper_status */
static void _account_helper_status(Account * account, char const * format, ...)
{
	va_list ap;
	int res;
	size_t size;
	char * p = NULL;

	va_start(ap, format);
	res = vsprintf(NULL, format, ap);
	va_end(ap);
	if(res >= 0)
	{
		va_start(ap, format);
		size = res;
		if((p = malloc(++size)) != NULL)
			vsnprintf(p, size, format, ap);
		va_end(ap);
	}
	if(p != NULL)
		mailer_set_status(account->mailer, p);
	free(p);
}


/* account_helper_authenticate */
static char * _account_helper_authenticate(Account * account,
		char const * message)
{
	char * ret = NULL;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * widget;

	dialog = gtk_dialog_new();
	/* XXX translate this, enumerate the methods available */
	gtk_window_set_title(GTK_WINDOW(dialog), "Authentication");
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	widget = gtk_label_new(message);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	widget = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_widget_show_all(vbox);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
		ret = strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
	gtk_widget_destroy(dialog);
	return ret;
}


/* account_helper_confirm */
static int _account_helper_confirm(Account * account, char const * message)
{
	int ret;
	GtkWidget * dialog;

	/* XXX set mailer's main window as the parent? */
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Confirm");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	/* XXX translate this */
	gtk_window_set_title(GTK_WINDOW(dialog), "Confirm");
	ret = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) ? 0 : 1;
	gtk_widget_destroy(dialog);
	return ret;
}


/* account_helper_folder_new */
static Folder * _account_helper_folder_new(Account * account,
		AccountFolder * folder, Folder * parent, FolderType type,
		char const * name)
{
	Folder * ret = NULL;
	GtkTreeModel * model = GTK_TREE_MODEL(account->store);
	GtkTreeIter aiter;
	GtkTreeIter * paiter = NULL;
	GtkTreeIter piter;
	GtkTreeIter * ppiter = NULL;
	GtkTreeIter siter;
	GtkTreeIter * psiter = NULL;
	GtkTreeIter iter;
	gint i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p, %p, %u, \"%s\")\n", __func__,
			(void *)account, (void *)folder, (void *)parent, type,
			name);
#endif
	/* lookup the account */
	if(_account_get_iter(account, &aiter) == TRUE)
		paiter = &aiter;
	/* lookup the parent folder */
	if(parent != NULL && folder_get_iter(parent, &piter) == TRUE)
		ppiter = &piter;
	else
		ppiter = paiter;
	/* lookup the following folder in sort order */
	if(ppiter != NULL)
		for(i = 0; gtk_tree_model_iter_nth_child(model, &siter, ppiter,
					i) != FALSE; i++)
		{
			psiter = &siter;
			gtk_tree_model_get(model, &siter, MFC_FOLDER, &ret, -1);
			if(type == FT_INBOX && folder_get_type(ret) != FT_INBOX)
				break;
			if(type < folder_get_type(ret))
				break;
			if(folder_get_type(ret) == type && strcmp(name,
						folder_get_name(ret)) < 0)
				break;
			psiter = NULL;
		}
	/* insert the folder in the model */
	gtk_tree_store_insert_before(account->store, &iter, ppiter, psiter);
	/* actually register the folder */
	if((ret = folder_new(folder, type, name, account->store, &iter))
			== NULL)
		gtk_tree_store_remove(account->store, &iter);
	else
		gtk_tree_store_set(account->store, &iter, MFC_ACCOUNT, account,
				-1);
	return ret;
}


/* account_helper_folder_delete */
static void _account_helper_folder_delete(Folder * folder)
{
	/* FIXME remove from the account */
	folder_delete(folder);
}


/* account_helper_message_new */
static Message * _account_helper_message_new(Account * account, Folder * folder,
		AccountMessage * message)
{
	Message * ret;
	GtkListStore * store;
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	store = folder_get_messages(folder);
	gtk_list_store_append(store, &iter);
	if((ret = message_new(message, store, &iter)) == NULL)
		gtk_list_store_remove(store, &iter);
	else
	{
		gtk_list_store_set(store, &iter, MHC_ACCOUNT, account,
				MHC_FOLDER, folder, -1);
		mailer_set_status(account->mailer, NULL);
	}
	return ret;
}


/* account_helper_message_delete */
static void _account_helper_message_delete(Message * message)
{
	GtkListStore * store;
	GtkTreeIter iter;

	if((store = message_get_store(message)) != NULL
			&& message_get_iter(message, &iter) != FALSE)
		gtk_list_store_remove(store, &iter);
	message_delete(message);
}


/* account_helper_message_set_body */
static int _account_helper_message_set_body(Message * message, char const * buf,
		size_t cnt, int append)
{
	return message_set_body(message, buf, cnt, (append != 0) ? TRUE
			: FALSE);
}
