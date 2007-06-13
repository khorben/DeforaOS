/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of GPuTTY */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef GPUTTY_GPUTTY_H
# define GPUTTY_GPUTTY_H

# include <System.h>


/* GPuTTY */
/* defaults */
# define SSH "ssh"
# define SSH_PORT 22
# define XTERM "xterm"
# define GPUTTY_CONFIG_FILE ".gputty"


/* types */
typedef struct _GPuTTY
{
	/* Config */
	Config * config;

	int selection;

	/* widgets */
	GtkWidget * window;
	/* hostname */
	GtkWidget * hn_frame;
	GtkWidget * hn_ehostname;
	GtkWidget * hn_sport;
	GtkWidget * hn_eusername;
	/* sessions */
	GtkWidget * sn_frame;
	GtkWidget * sn_esessions;
	GtkWidget * sn_tlsessions;
	GtkWidget * sn_load;
	GtkWidget * sn_save;
	GtkWidget * sn_delete;
	/* actions */
	GtkWidget * ac_about;
	GtkWidget * ac_prefs;
	GtkWidget * ac_exit;
	GtkWidget * ac_connect;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_exterm;
	GtkWidget * pr_essh;
} GPuTTY;


/* functions */
GPuTTY * gputty_new(void);
void gputty_delete(GPuTTY * gputty);

#endif /* !GPUTTY_GPUTTY_H */
