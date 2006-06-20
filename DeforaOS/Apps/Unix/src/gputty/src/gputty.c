/* gputty.c */
/* Copyright (c) 2004 Pierre Pronchery */
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



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gputty.h"


/* GPuTTY */
static int _gputty_error(char const * message, int ret);
static char * _gputty_config_file(void);
/* callbacks */
static void gputty_on_about(GtkWidget * widget, gpointer data);
static void gputty_on_connect(GtkWidget * widget, gpointer data);
static void gputty_on_delete(GtkWidget * widget, gpointer data);
static void gputty_on_load(GtkWidget * widget, gpointer data);
static void gputty_on_options(GtkWidget * widget, gpointer data);
static void gputty_on_exit(GtkWidget * widget, gpointer data);
static void gputty_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void gputty_on_save(GtkWidget * widget, gpointer data);
static void gputty_on_session_activate(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void gputty_on_session_select(GtkTreeView * view, gpointer data);
GPuTTY * gputty_new(void)
{
	GPuTTY * g;
	char * p;
	char * q;
	int i;
	GtkCellRenderer * cell;
	GtkTreeViewColumn * column;

	if((g = malloc(sizeof(GPuTTY))) == NULL)
	{
		_gputty_error("malloc", 0);
		return NULL;
	}

	/* Config */
	if((g->config = config_new()) == NULL)
	{
		free(g);
		return NULL;
	}
	config_set(g->config, "", "ssh", SSH);
	config_set(g->config, "", "xterm", XTERM);
	p = malloc(sizeof(char) * 6);
	sprintf(p, "%hu", SSH_PORT);
	config_set(g->config, "", "port", p);
	free(p);
	config_load(g->config, "/etc/gputty");
	if((p = _gputty_config_file()) != NULL)
	{
		config_load(g->config, p);
		free(p);
	}

	g->selection = -1;

	/* widgets */
	g->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(g->window), "GPuTTY");
	gtk_container_set_border_width(GTK_CONTAINER(g->window), 4);
	g_signal_connect(G_OBJECT(g->window), "delete_event",
			G_CALLBACK(gputty_on_exitx), g);
	g->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(g->window), g->vbox);
	/* hostname */
	g->hn_frame = gtk_frame_new("Specify your connection");
	gtk_box_pack_start(GTK_BOX(g->vbox), g->hn_frame, FALSE, FALSE, 0);
	g->hn_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(g->hn_vbox), 4);
	gtk_container_add(GTK_CONTAINER(g->hn_frame), g->hn_vbox);
	g->hn_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox), g->hn_hbox, TRUE, TRUE, 0);
	/* hostname: hostname */
	g->hn_vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox1, TRUE, TRUE, 0);
	g->hn_lhostname = gtk_label_new("Host name");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox1), g->hn_lhostname, TRUE, TRUE, 0);
	g->hn_ehostname = gtk_entry_new();
	if((p = config_get(g->config, "", "hostname")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_ehostname), p);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox1), g->hn_ehostname, TRUE, TRUE, 0);
	/* hostname: port */
	g->hn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox2, TRUE, TRUE, 4);
	g->hn_lport = gtk_label_new("Port");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox2), g->hn_lport, TRUE, TRUE, 0);
	g->hn_sport_adj = (GtkAdjustment *)gtk_adjustment_new(22, 0, 65535, 1, 4, 4);
	g->hn_sport = gtk_spin_button_new(g->hn_sport_adj, 1, 0);
	if((p = config_get(g->config, "", "port")) != NULL)
	{
		i = strtol(p, &q, 10);
		if(*q == '\0' && i >= 0 && i <= 65535)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(g->hn_sport),
					i);
	}
	gtk_box_pack_start(GTK_BOX(g->hn_vbox2), g->hn_sport, TRUE, TRUE, 0);
	/* hostname: username */
	g->hn_vbox3 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->hn_hbox), g->hn_vbox3, TRUE, TRUE, 0);
	g->hn_lusername = gtk_label_new("User name");
	gtk_box_pack_start(GTK_BOX(g->hn_vbox3), g->hn_lusername, TRUE, TRUE,
			0);
	g->hn_eusername = gtk_entry_new();
	if((p = config_get(g->config, "", "username")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_eusername), p);
	gtk_box_pack_start(GTK_BOX(g->hn_vbox3), g->hn_eusername, TRUE, TRUE,
			0);
	/* sessions */
	g->sn_frame = gtk_frame_new("Manage sessions");
	g->sn_vbox1 = gtk_vbox_new(FALSE, 0);
	g->sn_hbox = gtk_hbox_new(FALSE, 0);
	/* sessions: list */
	g->sn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(g->sn_vbox2), 4);
	g->sn_esessions = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(g->sn_vbox2), g->sn_esessions, FALSE, FALSE,
			0);
	g->sn_swsessions = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(g->sn_swsessions),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	g->sn_lsessions = gtk_list_store_new(1, G_TYPE_STRING);
	g->sn_tlsessions = gtk_tree_view_new();
	gtk_tree_view_set_model(GTK_TREE_VIEW(g->sn_tlsessions),
			GTK_TREE_MODEL(g->sn_lsessions));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(g->sn_tlsessions),
			FALSE);
	g_signal_connect(G_OBJECT(g->sn_tlsessions), "cursor-changed",
			G_CALLBACK(gputty_on_session_select), g);
	g_signal_connect(G_OBJECT(g->sn_tlsessions), "row-activated",
			G_CALLBACK(gputty_on_session_activate), g);
	gtk_scrolled_window_add_with_viewport(
			GTK_SCROLLED_WINDOW(g->sn_swsessions),
			g->sn_tlsessions);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox2), g->sn_swsessions, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(g->sn_hbox), g->sn_vbox2, TRUE, TRUE, 0);
	/* sessions: buttons */
	g->sn_vbox3 = gtk_vbox_new(FALSE, 0);
	g->sn_load = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(G_OBJECT(g->sn_load), "clicked",
			G_CALLBACK(gputty_on_load), g);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_load, FALSE, FALSE, 4);
	g->sn_save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(g->sn_save), "clicked",
			G_CALLBACK(gputty_on_save), g);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_save, FALSE, FALSE, 0);
	g->sn_delete = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect(G_OBJECT(g->sn_delete), "clicked",
			G_CALLBACK(gputty_on_delete), g);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox3), g->sn_delete, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(g->sn_hbox), g->sn_vbox3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->sn_vbox1), g->sn_hbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->sn_frame), g->sn_vbox1);
	gtk_box_pack_start(GTK_BOX(g->vbox), g->sn_frame, TRUE, TRUE, 4);
	/* actions */
	g->ac_hbox = gtk_hbox_new(FALSE, 0);
#if GTK_CHECK_VERSION(2, 6, 0)
	g->ac_about = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
#else
	g->ac_about = gtk_button_new_with_mnemonic("_About");
#endif
	gtk_box_pack_start(GTK_BOX(g->ac_hbox), g->ac_about, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(g->ac_about), "clicked",
			G_CALLBACK(gputty_on_about), g);
	g->ac_options = gtk_button_new_from_stock(GTK_STOCK_PREFERENCES);
	gtk_box_pack_start(GTK_BOX(g->ac_hbox), g->ac_options, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(g->ac_options), "clicked",
			G_CALLBACK(gputty_on_options), g);
	g->ac_connect = gtk_button_new_with_mnemonic("_Connect");
	g_signal_connect(G_OBJECT(g->ac_connect), "clicked",
			G_CALLBACK(gputty_on_connect), g);
	gtk_box_pack_end(GTK_BOX(g->ac_hbox), g->ac_connect, FALSE, FALSE, 0);
	g->ac_exit = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_box_pack_end(GTK_BOX(g->ac_hbox), g->ac_exit, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(g->ac_exit), "clicked",
			G_CALLBACK(gputty_on_exit), g);
	gtk_box_pack_start(GTK_BOX(g->vbox), g->ac_hbox, FALSE, FALSE, 0);
	/* options */
	g->op_window = NULL;

	/* load sessions */
	for(i = 0; i <= 99; i++)
	{
		char buf[11];
		sprintf(buf, "session %d", i);
		if((p = config_get(g->config, buf, "name")) == NULL)
			break;
		gtk_list_store_append(g->sn_lsessions, &g->sn_ilsessions);
		gtk_list_store_set(g->sn_lsessions, &g->sn_ilsessions, 0, p,
				-1);
	}
	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Sessions", cell,
			"text", NULL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(g->sn_tlsessions),
			GTK_TREE_VIEW_COLUMN(column));

	/* show window */
	gtk_widget_show_all(g->window);
	return g;
}

static int _gputty_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "GPuTTY: ");
	perror(message);
	return ret;
}

static char * _gputty_config_file(void)
{
	char * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	if((filename = malloc(strlen(homedir) + 1 + strlen(GPUTTY_CONFIG_FILE)
					+ 1)) == NULL)
		return NULL;
	sprintf(filename, "%s/%s", homedir, GPUTTY_CONFIG_FILE);
	return filename;
}


void gputty_delete(GPuTTY * gputty)
{
	config_delete(gputty->config);
	free(gputty);
}


/* callbacks */
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_close(GtkWidget * widget, gpointer data);
static void _about_credits(GtkWidget * widget, gpointer data);
static void _about_license(GtkWidget * widget, gpointer data);
#endif
static void gputty_on_about(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	char const * authors[] = { "Pierre 'khorben' Pronchery", NULL };
	char const comment[] =
"GPuTTY is a clone of PuTTY for Open Source desktops.\n"
"This software mainly relies on:\n"
"- Glib\n"
"- Gtk+\n"
"Credits go to all Free Software contributors.";
	char const copyright[] = "Copyright (c) 2004-2006 khorben";
	char const website[] = "http://people.defora.org/~khorben/projects/gputty/";

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
#if GTK_CHECK_VERSION(2, 6, 0)
	window = gtk_about_dialog_new();
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), "GPuTTY");
	/* FIXME automatic version */
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), "0.9.8");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(window), comment);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), "GPLv2");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(window), website);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), authors);
	gtk_widget_show(window);
#else
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About GPuTTY");
	{
		GtkWidget * vbox;
		GtkWidget * hbox;
		GtkWidget * button;

		vbox = gtk_vbox_new(FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("GPuTTY 0.9.8"),
				FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(comment),
				FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(copyright),
				FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(website),
				FALSE, FALSE, 2);
		hbox = gtk_hbox_new(TRUE, 0);
		button = gtk_button_new_with_mnemonic("C_redits");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_credits), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		button = gtk_button_new_with_mnemonic("_License");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_license), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_close), window);
		gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
		gtk_container_add(GTK_CONTAINER(window), vbox);
	}
	gtk_widget_show_all(window);
#endif
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_close(GtkWidget * widget, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
}

static void _about_credits(GtkWidget * widget, gpointer data)
{
}

static void _about_license(GtkWidget * widget, gpointer data)
{
}
#endif

static void gputty_on_connect(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	pid_t pid;
	char * xterm = NULL;
	char * ssh = NULL;
	char const * hostname;
	char port[6];
	char * useropt = NULL;
	char const * username = NULL;

	xterm = config_get(g->config, "", "xterm");
	ssh = config_get(g->config, "", "ssh");
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(snprintf(port, 6, "%d", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g->hn_sport))) >= 6)
		port[5] = '\0';
	if(hostname[0] == '\0')
		return;
	if((pid = fork()) == -1)
	{
		_gputty_error("fork", 0);
		return;
	}
	else if(pid == 0)
	{
		username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
		if(username[0] != '\0')
			useropt = "-l";
		execlp(xterm, xterm, "-e",
				ssh, hostname,
				"-p", port,
				useropt, username,
				NULL);
		fprintf(stderr, "%s", "GPuTTY: ");
		exit(_gputty_error(xterm, 2));
	}
}

static void gputty_on_delete(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	int i;
	char buf1[11];
	char buf2[11];
	char * p;

	if(g->selection == -1)
		return;
	i = g->selection;
	gtk_list_store_remove(g->sn_lsessions, &g->sn_ilsessions);
	for(; i < 100; i++)
	{
		sprintf(buf1, "session %d", i);
		sprintf(buf2, "session %d", i+1);
		if((p = config_get(g->config, buf2, "name")) == NULL)
		{
			config_set(g->config, buf1, "name", NULL);
			break;
		}
		config_set(g->config, buf1, "name", p);
		config_set(g->config, buf1, "hostname",
				config_get(g->config, buf2, "hostname"));
		config_set(g->config, buf1, "username",
				config_get(g->config, buf2, "username"));
	}
}

static void gputty_on_load(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char buf[11];
	char * p;
	char * q;
	int port;

	if(g->selection < 0 || g->selection >= 100)
		return;
	sprintf(buf, "session %d", g->selection);
	if((p = config_get(g->config, buf, "hostname")) == NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_ehostname), "");
	else
		gtk_entry_set_text(GTK_ENTRY(g->hn_ehostname), p);
	if((p = config_get(g->config, buf, "username")) == NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_eusername), "");
	else
		gtk_entry_set_text(GTK_ENTRY(g->hn_eusername), p);
	if((p = config_get(g->config, buf, "port")) == NULL)
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(g->hn_sport),
				SSH_PORT);
		return;
	}
	port = strtol(p, &q, 10);
	if(*q == '\0' && port >= 0 && port <= 65535)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(g->hn_sport), port);
}

static void gputty_on_options_cancel(GtkWidget * widget, gpointer data);
static void gputty_on_options_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void gputty_on_options_ok(GtkWidget * widget, gpointer data);
static void gputty_on_options(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char * p;

	if(g->op_window == NULL)
	{
		g->op_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_container_set_border_width(GTK_CONTAINER(g->op_window), 4);
		gtk_window_set_title(GTK_WINDOW(g->op_window),
				"GPuTTY Preferences");
		g_signal_connect(G_OBJECT(g->op_window), "delete_event",
				G_CALLBACK(gputty_on_options_closex), g);
		g->op_vbox = gtk_vbox_new(FALSE, 0);
		/* xterm */
		g->op_hbox1 = gtk_hbox_new(TRUE, 0);
		g->op_lxterm = gtk_label_new("Terminal emulator: ");
		gtk_box_pack_start(GTK_BOX(g->op_hbox1), g->op_lxterm,
				FALSE, FALSE, 0);
		g->op_exterm = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(g->op_hbox1), g->op_exterm,
				FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(g->op_vbox), g->op_hbox1,
				FALSE, FALSE, 0);
		/* ssh */
		g->op_hbox2 = gtk_hbox_new(TRUE, 0);
		g->op_lssh = gtk_label_new("SSH client: ");
		gtk_box_pack_start(GTK_BOX(g->op_hbox2), g->op_lssh,
				FALSE, FALSE, 0);
		g->op_essh = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(g->op_hbox2), g->op_essh,
				FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(g->op_vbox), g->op_hbox2,
				FALSE, FALSE, 4);
		/* buttons */
		g->op_hbox3 = gtk_hbox_new(FALSE, 0);
		g->op_ok = gtk_button_new_from_stock(GTK_STOCK_OK);
		g_signal_connect(G_OBJECT(g->op_ok), "clicked",
				G_CALLBACK(gputty_on_options_ok), g);
		gtk_box_pack_end(GTK_BOX(g->op_hbox3), g->op_ok,
				FALSE, FALSE, 0);
		g->op_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
		g_signal_connect(G_OBJECT(g->op_cancel), "clicked",
				G_CALLBACK(gputty_on_options_cancel), g);
		gtk_box_pack_end(GTK_BOX(g->op_hbox3), g->op_cancel,
				FALSE, FALSE, 4);
		gtk_box_pack_start(GTK_BOX(g->op_vbox), g->op_hbox3,
				FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(g->op_window), g->op_vbox);
		gtk_widget_show_all(g->op_window);
	}
	if((p = config_get(g->config, "", "xterm")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->op_exterm), p);
	if((p = config_get(g->config, "", "ssh")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->op_essh), p);
	gtk_widget_show(g->op_window);
}

static void gputty_on_options_cancel(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;

	gtk_widget_hide(g->op_window);
}

static void gputty_on_options_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	GPuTTY * g = data;

	gtk_widget_hide(g->op_window);
}

static void gputty_on_options_ok(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;

	gtk_widget_hide(g->op_window);
	config_set(g->config, "", "xterm", gtk_entry_get_text(GTK_ENTRY(
					g->op_exterm)));
	config_set(g->config, "", "ssh", gtk_entry_get_text(GTK_ENTRY(
					g->op_essh)));
}

static void gputty_on_exit(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char buf[11];
	char * filename;

	if(g->config == NULL)
	{
		fprintf(stderr, "%s", "gputty: not saving configuration\n");
		gtk_main_quit();
		return;
	}
	config_set(g->config, "", "hostname", gtk_entry_get_text(GTK_ENTRY(
					g->hn_ehostname)));
	config_set(g->config, "", "username", gtk_entry_get_text(GTK_ENTRY(
					g->hn_eusername)));
	snprintf(buf, sizeof(buf), "%d", gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON(g->hn_sport)));
	config_set(g->config, "", "port", buf);
	if((filename = _gputty_config_file()) == NULL
			|| config_save(g->config, filename) != 0)
	{
		fprintf(stderr, "%s%s", "gputty: an error occured while",
				" saving configuration\n");
		if(filename != NULL)
			free(filename);
	}
	gtk_main_quit();
}

static void gputty_on_exitx(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gputty_on_exit(widget, data);
}

static void gputty_on_save(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char const * session;
	char const * hostname;
	int port;
	char const * username;
	int row = g->selection;
	char buf[11];
	char buf2[11];

	session = gtk_entry_get_text(GTK_ENTRY(g->sn_esessions));
	if(session[0] == '\0')
		return;
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(hostname[0] == '\0')
		return;
	username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
	port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g->hn_sport));
	if(g->selection == -1)
	{
		gtk_list_store_append(g->sn_lsessions, &g->sn_ilsessions);
		gtk_list_store_set(g->sn_lsessions, &g->sn_ilsessions, 0,
				session, -1);
	}
	else
		gtk_list_store_set(g->sn_lsessions, &g->sn_ilsessions, 0,
				session, -1);
	if(row >= 100 || row < 0)
		return;
	sprintf(buf, "session %d", row);
	sprintf(buf2, "%d", port);
	config_set(g->config, buf, "name", session);
	config_set(g->config, buf, "hostname", hostname);
	config_set(g->config, buf, "username", username);
	config_set(g->config, buf, "port", buf2);
}

static void gputty_on_session_activate(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	GPuTTY * g = data;
	gint * p;

	if((p = gtk_tree_path_get_indices(path)) == NULL || *p < 0)
		return;
	g->selection = *p;
	gputty_on_load(GTK_WIDGET(view), data);
	gputty_on_connect(GTK_WIDGET(view), data);
}

static void gputty_on_session_select(GtkTreeView * view, gpointer data)
{
	GPuTTY * g = data;
	GtkTreeSelection * sel;
	GList * list;
	int * p;

	g->selection = -1;
	sel = gtk_tree_view_get_selection(view);
	if(!gtk_tree_selection_get_selected(sel, NULL, &g->sn_ilsessions)
		|| (list = gtk_tree_selection_get_selected_rows(sel, NULL))
			== NULL)
		return;
	if((p = gtk_tree_path_get_indices(list->data)) != NULL)
		g->selection = *p;
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}


/* main */
int main(int argc, char * argv[])
{
	GPuTTY * gputty;

	gtk_init(&argc, &argv);
	if((gputty = gputty_new()) == NULL)
		return 1;
	gtk_main();
	gputty_delete(gputty);
	return 0;
}
