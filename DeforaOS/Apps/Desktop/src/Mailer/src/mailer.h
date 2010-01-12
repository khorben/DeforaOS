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



#ifndef MAILER_MAILER_H
# define MAILER_MAILER_H

# include <System.h>
# include <gtk/gtk.h>
# include "account/account.h"
# include "../config.h"


/* Mailer */
/* defaults */
# ifndef PLUGINDIR
#  define PLUGINDIR		LIBDIR "/Mailer"
# endif

# define MAILER_CONFIG_FILE	".mailer"

# define MAILER_MESSAGES_FONT	"Monospace 8"


/* types */
enum
{
	MF_COL_ACCOUNT = 0, MF_COL_ENABLED, MF_COL_DELETE, MF_COL_FOLDER,
	MF_COL_ICON, MF_COL_NAME
};
# define MF_COL_LAST MF_COL_NAME
# define MF_COL_COUNT (MF_COL_LAST + 1)

enum
{
	MH_COL_ACCOUNT = 0, MH_COL_FOLDER, MH_COL_MESSAGE, MH_COL_SUBJECT,
	MH_COL_FROM, MH_COL_TO, MH_COL_DATE
};
# define MH_COL_LAST MH_COL_DATE
# define MH_COL_COUNT (MH_COL_LAST + 1)

typedef struct _Mailer
{
	Account * available; /* XXX consider using another data type */
	unsigned int available_cnt;

	Account ** account;
	unsigned int account_cnt;
	Account * account_cur;
	AccountFolder * folder_cur;

	/* configuration */
	Config * config;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view_folders;
	GtkWidget * view_headers;
	GtkWidget * hdr_vbox;
	GtkWidget * hdr_subject;
	GtkWidget * hdr_from;
	GtkWidget * hdr_to;
	GtkWidget * hdr_date;
	GtkTextBuffer * view_buffer;
	GtkWidget * view_body;
	GtkWidget * statusbar;
	gint statusbar_id;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_accounts;
	GtkWidget * pr_messages_font;
} Mailer;


/* functions */
Mailer * mailer_new(void);
void mailer_delete(Mailer * mailer);

/* accessors */
char const * mailer_get_config(Mailer * mailer, char const * variable);
char * mailer_get_config_filename(Mailer * mailer);

/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret);

int mailer_account_add(Mailer * mailer, Account * account);
#if 0 /* FIXME deprecate? */
int mailer_account_disable(Mailer * mailer, Account * account);
int mailer_account_enable(Mailer * mailer, Account * account);
#endif
/* FIXME implement
int mailer_account_remove(Mailer * mailer, Account * account); */

#endif
