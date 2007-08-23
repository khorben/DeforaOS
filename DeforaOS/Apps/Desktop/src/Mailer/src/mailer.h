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



#ifndef MAILER_MAILER_H
# define MAILER_MAILER_H

# include <gtk/gtk.h>
# include "account/account.h"


/* Mailer */
/* types */
typedef struct _Mailer
{
	Account * available; /* XXX consider using another data type */
	unsigned int available_cnt;

	Account ** account;
	unsigned int account_cnt;
	Account * account_cur;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view_folders;
	GtkWidget * view_headers;
	GtkWidget * hdr_vbox;
	GtkWidget * hdr_subject;
	GtkWidget * hdr_from;
	GtkWidget * hdr_to;
	GtkWidget * hdr_date;
	GtkWidget * view_body;
	GtkWidget * statusbar;
	gint statusbar_id;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_accounts;
} Mailer;


/* functions */
Mailer * mailer_new(void);
void mailer_delete(Mailer * mailer);

/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret);

int mailer_account_add(Mailer * mailer, Account * account);

#endif
