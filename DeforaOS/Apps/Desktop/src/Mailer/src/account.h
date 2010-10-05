/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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

# include <System.h>
# include "Mailer.h"


/* Account */
/* types */
typedef struct _Account Account;


/* functions */
Account * account_new(char const * type, char const * name,
		AccountPluginHelper * helper);
void account_delete(Account * account);

/* accessors */
int account_get_enabled(Account * account);
void account_set_enabled(Account * account, int enabled);

GtkListStore * account_get_store(Account * account, AccountFolder * folder);

AccountConfig * account_get_config(Account * account);
char const * account_get_name(Account * account);
char const * account_get_title(Account * account);
char const * account_get_type(Account * account);
int account_set_title(Account * account, char const * title);

/* useful */
int account_config_load(Account * account, Config * config);
int account_config_save(Account * account, Config * config);
int account_init(Account * account, GtkTreeStore * store, GtkTreeIter * parent);
int account_quit(Account * account);

GtkTextBuffer * account_select(Account * account, AccountFolder * folder,
		AccountMessage * message);

#endif /* !MAILER_ACCOUNT_H */
