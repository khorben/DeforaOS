/* gputty.h */
/* Copyright (C) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
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



#ifndef _GPUTTY_H
# define _GPUTTY_H

# include "config.h"


/* GPuTTY */
/* types */
typedef struct _GPuTTY {
	/* Config */
	Config * config;

	/* widgets */
	GtkWidget * window;
	GtkWidget * vbox;
	/* hostname */
	GtkWidget * hn_frame;
	GtkWidget * hn_vbox;
	GtkWidget * hn_hbox;
	GtkWidget * hn_vbox1;
	GtkWidget * hn_lhostname;
	GtkWidget * hn_ehostname;
	GtkWidget * hn_vbox2;
	GtkWidget * hn_lport;
	GtkAdjustment * hn_sport_adj;
	GtkWidget * hn_sport;
	GtkWidget * hn_vbox3;
	GtkWidget * hn_lusername;
	GtkWidget * hn_eusername;
	/* sessions */
	GtkWidget * sn_frame;
	GtkWidget * sn_vbox1;
	GtkWidget * sn_lsessions;
	GtkWidget * sn_hbox;
	GtkWidget * sn_vbox2;
	GtkWidget * sn_esessions;
	GtkWidget * sn_clsessions;
	GtkWidget * sn_vbox3;
	GtkWidget * sn_load;
	GtkWidget * sn_save;
	GtkWidget * sn_delete;
	/* actions */
	GtkWidget * ac_hbox;
	GtkWidget * ac_about;
	GtkWidget * ac_quit;
	GtkWidget * ac_connect;
	/* about */
	GtkWidget * ab_window;
	GtkWidget * ab_close;
} GPuTTY;


/* functions */
GPuTTY * gputty_new(void);
void gputty_delete(GPuTTY * gputty);

#endif /* !_GPUTTY_H */
