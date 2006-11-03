/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
	GtkIconTheme * theme;
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

/* FIXME */
int mailer_account_add(Mailer * mailer);

#endif
