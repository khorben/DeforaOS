/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MAILER_ACCOUNT_H
# define MAILER_ACCOUNT_H

# include <gtk/gtk.h>
# include <System.h>


/* AccountIdentity */
typedef struct _AccountIdentity
{
	char * from;
	char * email;
} AccountIdentity;


/* AccountConfig */
typedef enum _AccountConfigType
{
	ACT_NONE = 0,
	ACT_STRING,
	ACT_PASSWORD,
	ACT_FILE,
	ACT_UINT16,
	ACT_BOOLEAN
} AccountConfigType;

typedef struct _AccountConfig
{
	char * name;
	char * title;
	AccountConfigType type;
	void * value;
} AccountConfig;


/* AccountMessage */
typedef struct _AccountMessage AccountMessage;


/* AccountFolderType */
typedef enum _AccountFolderType
{
	AFT_INBOX = 0,
	AFT_DRAFTS,
	AFT_SENT,
	AFT_TRASH,
	AFT_FOLDER
} AccountFolderType;
# define AFT_LAST AFT_FOLDER
# define AFT_COUNT (AFT_LAST + 1)

typedef struct _AccountFolder
{
	AccountFolderType type;
	char * name;
	GtkListStore * store;
	void * data;
} AccountFolder;


/* AccountPlugin */
typedef struct _AccountPlugin
{
	char const * type;
	char const * name;
	AccountConfig * config;
	int (*init)(GtkTreeStore * store, GtkTreeIter * parent,
			GtkTextBuffer * buffer);
	int (*quit)(void);
	int (*select)(AccountFolder * folder, AccountMessage * message);
} AccountPlugin;


/* Account */
/* types */
typedef struct _Account
{
	char * name;
	char * title;
	int enabled;
	AccountIdentity * identity;
	void * handle;
	AccountPlugin * plugin;
	GtkTextBuffer * buffer;
} Account;


/* functions */
Account * account_new(char const * type, char const * name);
void account_delete(Account * account);

/* accessors */
int account_get_enabled(Account * account);
void account_set_enabled(Account * account, int enabled);

GtkListStore * account_get_store(Account * account, AccountFolder * folder);

char const * account_get_title(Account * account);
int account_set_title(Account * account, char const * title);

/* useful */
int account_config_load(Account * account, Config * config);
int account_config_save(Account * account, Config * config);
int account_init(Account * account, GtkTreeStore * store, GtkTreeIter * parent);
int account_quit(Account * account);

int account_select(Account * account, AccountFolder * folder,
		AccountMessage * message);

#endif /* !MAILER_ACCOUNT_H */
