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



#ifndef MAILER_ACCOUNT_H
# define MAILER_ACCOUNT_H

# include <System.h>
# include "Mailer.h"


/* Account */
/* types */


/* functions */
Account * account_new(Mailer * mailer, char const * type, char const * title,
		GtkTreeStore * store);
void account_delete(Account * account);

/* accessors */
int account_get_enabled(Account * account);
void account_set_enabled(Account * account, int enabled);

AccountConfig * account_get_config(Account * account);
char const * account_get_name(Account * account);
char const * account_get_title(Account * account);
char const * account_get_type(Account * account);
int account_set_name(Account * account, char const * name);

/* useful */
int account_config_load(Account * account, Config * config);
int account_config_save(Account * account, Config * config);
int account_init(Account * account);
int account_quit(Account * account);

GtkTextBuffer * account_select(Account * account, Folder * folder,
		Message * message);
GtkTextBuffer * account_select_source(Account * account, Folder * folder,
		Message * message);

#endif /* !MAILER_ACCOUNT_H */
