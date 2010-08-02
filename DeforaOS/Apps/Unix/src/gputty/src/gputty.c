/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2004-2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of GPuTTY */
static char const _license[] =
"GPuTTY is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation; either version 3 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"GPuTTY is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with GPuTTY; if not, write to the Free Software\n"
"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n";



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gputty.h"
#include "../config.h"


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* GPuTTY */
/* private */
/* types */
typedef enum _GPuTTYColumn
{
	GC_COL_NAME = 0,
	GC_COUNT
} GPuTTYColumn;
#define GC_LAST GC_COUNT


/* prototypes */
static int _gputty_error(char const * message, int ret);
static char * _gputty_config_file(void);

/* callbacks */
static void _on_about(GtkWidget * widget, gpointer data);
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void _on_connect(GtkWidget * widget, gpointer data);
static void _on_delete(GtkWidget * widget, gpointer data);
static void _on_load(GtkWidget * widget, gpointer data);
static void _on_exit(GtkWidget * widget, gpointer data);
static void _on_move_down(GtkWidget * widget, gpointer data);
static void _on_move_up(GtkWidget * widget, gpointer data);
static void _on_preferences(GtkWidget * widget, gpointer data);
static void _on_save(GtkWidget * widget, gpointer data);
static void _on_session_activate(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void _on_session_select(GtkTreeSelection * selection, gpointer data);


/* functions */
/* gputty_error */
static int _gputty_error(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* gputty_config_file */
static char * _gputty_config_file(void)
{
	static char filename[PATH_MAX];
	char * p;

	if((p = getenv("HOME")) == NULL)
		return NULL;
	if(snprintf(filename, sizeof(filename), "%s/%s", p, GPUTTY_CONFIG_FILE)
			>= (int)sizeof(filename)) /* XXX cast */
		return NULL;
	return filename;
}


/* callbacks */
/* on_about */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif

static void _on_about(GtkWidget * widget, gpointer data)
{
	GPuTTY * gputty = data;
	static GtkWidget * window = NULL;
	char const comment[] =
"GPuTTY is a clone of PuTTY for Open Source desktops.\n"
"This software mainly relies on:\n"
"- Glib\n"
"- Gtk+\n"
"Credits go to all Free Software contributors.";
	char const website[] = "http://gputty.sourceforge.net/";
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		_gputty_error("malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				gputty->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(window), comment);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(window), website);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-3", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	gtk_widget_show(window);
#else
	GtkWidget * vbox;
	GtkWidget * hbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About " PACKAGE);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				gputty->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 2);
	widget = gtk_label_new(PACKAGE " " VERSION);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 2);
	widget = gtk_label_new(comment);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 2);
	widget = gtk_label_new(_copyright);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 2);
	widget = gtk_label_new(website);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 0);
	widget = gtk_button_new_with_mnemonic("C_redits");
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_with_mnemonic("_License");
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_about_on_license), window);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
#endif
}

static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
}

static void _about_on_credits(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	GtkWidget * hbox;
	size_t i;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Credits");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, "", 0);
	for(i = 0; _authors[i] != NULL; i++)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, _authors[i], strlen(
					_authors[i]));
	}
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new("Written by"));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _about_on_license(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkWidget * hbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Credits");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */

static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	_on_exit(widget, data);
	return TRUE;
}

static void _on_connect(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	pid_t pid;
	char const * xterm = NULL;
	char * termopt = "-e";
	char const * ssh = NULL;
	char const * hostname;
	char port[6];
	char * useropt = NULL;
	char const * username = NULL;

	xterm = config_get(g->config, "", "xterm");
	ssh = config_get(g->config, "", "ssh");
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(snprintf(port, sizeof(port), "%d", gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(g->hn_sport))) >= 6)
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
		if(strcmp(xterm, "gnome-terminal") == 0)
			termopt = "-x";
		execlp(xterm, xterm, termopt, ssh, hostname, "-p", port,
				useropt, username, NULL);
		exit(_gputty_error(xterm, 2));
	}
}

static void _on_delete(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	int i;
	GtkTreeModel * model;
	GtkTreePath * path;
	GtkTreeIter iter;
	char buf1[11];
	char buf2[11];
	char const * p;

	if((i = g->selection) == -1)
		return;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(g->sn_tlsessions));
	path = gtk_tree_path_new_from_indices(i, -1);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	for(; i < 100; i++)
	{
		/* XXX store information in the ListStore instead */
		snprintf(buf1, sizeof(buf1), "session %d", i);
		snprintf(buf2, sizeof(buf2), "session %d", i + 1);
		if((p = config_get(g->config, buf2, "name")) == NULL)
		{
			config_set(g->config, buf1, "name", NULL);
			break;
		}
		config_set(g->config, buf1, "name", p);
		config_set(g->config, buf1, "hostname",
				config_get(g->config, buf2, "hostname"));
		config_set(g->config, buf1, "port",
				config_get(g->config, buf2, "port"));
		config_set(g->config, buf1, "username",
				config_get(g->config, buf2, "username"));
	}
}

static void _on_load(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char buf[11];
	char const * p;
	char * q;
	int port;

	if(g->selection < 0 || g->selection >= 100)
		return;
	snprintf(buf, sizeof(buf), "session %d", g->selection);
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

static void _on_exit(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char buf[11];
	char * filename;

	if(g->config == NULL)
	{
		fputs(PACKAGE ": not saving configuration\n", stderr);
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
		fputs(PACKAGE ": an error occured while saving configuration\n",
				stderr);
	gtk_main_quit();
}


/* on_move_down */
static void _move_switch(GPuTTY * g, int a, int b);

static void _on_move_down(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	int i;

	if((i = g->selection) == -1)
		return;
	_move_switch(g, i, i + 1);
}

static void _move_switch(GPuTTY * g, int a, int b)
	/* FIXME this code is ugly the ListStore should store everything */
{
	GtkTreeModel * model;
	GtkTreePath * path;
	GtkTreeIter iter;
	char buf1[11];
	char buf2[11];
	char const * p;
	char * name = NULL;
	char * hostname = NULL;
	char * port = NULL;
	char * username = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(g->sn_tlsessions));
	snprintf(buf1, sizeof(buf1), "session %d", a);
	snprintf(buf2, sizeof(buf2), "session %d", b);
	if((p = config_get(g->config, buf2, "name")) == NULL)
		return;
	if((p = config_get(g->config, buf1, "name")) == NULL)
		return;
	name = strdup(p);
	if((p = config_get(g->config, buf1, "hostname")) != NULL)
		hostname = strdup(p);
	if((p = config_get(g->config, buf1, "port")) != NULL)
		port = strdup(p);
	if((p = config_get(g->config, buf1, "username")) != NULL)
		username = strdup(p);
	path = gtk_tree_path_new_from_indices(b, -1);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, GC_COL_NAME, name, -1);
	gtk_tree_path_free(path);
	config_set(g->config, buf1, "name", config_get(g->config, buf2,
				"name"));
	config_set(g->config, buf1, "hostname", config_get(g->config, buf2,
				"hostname"));
	config_set(g->config, buf1, "port", config_get(g->config, buf2,
				"port"));
	config_set(g->config, buf1, "username", config_get(g->config, buf2,
				"username"));
	config_set(g->config, buf2, "name", name);
	config_set(g->config, buf2, "hostname", hostname);
	config_set(g->config, buf2, "port", port);
	config_set(g->config, buf2, "username", username);
	path = gtk_tree_path_new_from_indices(a, -1);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, GC_COL_NAME,
			config_get(g->config, buf1, "name"), -1);
	gtk_tree_path_free(path);
	free(name);
	free(hostname);
	free(port);
	free(username);
}


/* on_move_up */
static void _on_move_up(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	int i;

	if((i = g->selection) == -1)
		return;
	_move_switch(g, i - 1, i);
}


/* on_preferences */
static void _on_preferences_cancel(GtkWidget * widget, gpointer data);
static void _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_preferences_ok(GtkWidget * widget, gpointer data);

static void _on_preferences(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * frame;
	GtkWidget * vbox2;
	GtkSizeGroup * group;
	char const * p;

	if(g->pr_window != NULL)
	{
		gtk_widget_show(g->pr_window);
		return;
	}
	g->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(g->pr_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(g->pr_window), "GPuTTY preferences");
	gtk_window_set_transient_for(GTK_WINDOW(g->pr_window), GTK_WINDOW(
				g->window));
	gtk_container_set_border_width(GTK_CONTAINER(g->pr_window), 4);
	g_signal_connect(G_OBJECT(g->pr_window), "delete-event", G_CALLBACK(
				_on_preferences_closex), g);
	vbox = gtk_vbox_new(FALSE, 0);
	/* external */
	frame = gtk_frame_new("External programs");
	vbox2 = gtk_vbox_new(FALSE, 0);
	/* xterm */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Terminal emulator: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	g->pr_exterm = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), g->pr_exterm, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	/* ssh */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("SSH client: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	g->pr_essh = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), g->pr_essh, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 4);
	/* buttons */
	hbox = gtk_hbox_new(FALSE, 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_ok), g);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_cancel), g);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(g->pr_window), vbox);
	if((p = config_get(g->config, "", "xterm")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->pr_exterm), p);
	if((p = config_get(g->config, "", "ssh")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->pr_essh), p);
	gtk_widget_show_all(g->pr_window);
}

static void _on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char const * p;

	gtk_widget_hide(g->pr_window);
	/* FIXME factorize code */
	if((p = config_get(g->config, "", "xterm")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->pr_exterm), p);
	if((p = config_get(g->config, "", "ssh")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->pr_essh), p);
}

static void _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	GPuTTY * g = data;

	gtk_widget_hide(g->pr_window);
}

static void _on_preferences_ok(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;

	gtk_widget_hide(g->pr_window);
	config_set(g->config, "", "xterm", gtk_entry_get_text(GTK_ENTRY(
					g->pr_exterm)));
	config_set(g->config, "", "ssh", gtk_entry_get_text(GTK_ENTRY(
					g->pr_essh)));
}

static void _on_save(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	char const * session;
	char const * hostname;
	int port;
	char const * username;
	char buf[11];
	char buf2[11];
	GtkTreeSelection * sel;
	GtkTreeIter iter;
	GtkTreeModel * model;
	int row = g->selection;
	GtkTreePath * path;
	int * p;

	session = gtk_entry_get_text(GTK_ENTRY(g->sn_esessions));
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(session[0] == '\0' || hostname[0] == '\0')
		return;
	username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
	port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(g->hn_sport));
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g->sn_tlsessions));
	if(gtk_tree_selection_get_selected(sel, &model, &iter) != TRUE)
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, GC_COL_NAME, session,
			-1);
	if(row < 0)
	{
		path = gtk_tree_model_get_path(model, &iter);
		if((p = gtk_tree_path_get_indices(path)) != NULL)
			row = *p;
		gtk_tree_path_free(path);
	}
	if(row < 0 || row >= 100)
	{
		fprintf(stderr, "%s%s%s", "gputty: Can't save session (",
				row < 0 ? "negative session"
				: "100 sessions maximum", ")\n");
		return;
	}
	snprintf(buf, sizeof(buf), "session %d", row);
	snprintf(buf2, sizeof(buf2), "%d", port);
	config_set(g->config, buf, "name", session);
	config_set(g->config, buf, "hostname", hostname);
	config_set(g->config, buf, "username", username);
	config_set(g->config, buf, "port", buf2);
}

static void _on_session_activate(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	GPuTTY * g = data;
	gint * p;

	if((p = gtk_tree_path_get_indices(path)) == NULL || *p < 0)
		return;
	g->selection = *p;
	_on_load(GTK_WIDGET(view), data);
	_on_connect(GTK_WIDGET(view), data);
}

static void _on_session_select(GtkTreeSelection * selection, gpointer data)
{
	GPuTTY * g = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GList * list;
	int * p;

	g->selection = -1;
	if(!gtk_tree_selection_get_selected(selection, &model, &iter)
			|| (list = gtk_tree_selection_get_selected_rows(
					selection, NULL)) == NULL)
		return;
	if((p = gtk_tree_path_get_indices(list->data)) != NULL)
		g->selection = *p;
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}


/* public */
/* functions */
/* gputty_new */
GPuTTY * gputty_new(void)
{
	GPuTTY * g;
	char buf[11];
	char const * p;
	char * q;
	unsigned int i;
	GtkWidget * vbox;
	GtkWidget * vbox2;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkListStore * model;
	GtkTreeIter iter;
	GtkTreeSelection * sel;
	GtkAccelGroup * group;

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
	snprintf(buf, sizeof(buf), "%hu", SSH_PORT);
	config_set(g->config, "", "port", buf);
	/* FIXME use ETCDIR */
	config_load(g->config, "/etc/gputty.conf");
	if((p = _gputty_config_file()) != NULL)
		config_load(g->config, p);

	g->selection = -1;

	/* widgets */
	group = gtk_accel_group_new();
	g->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(g->window), "GPuTTY");
	gtk_container_set_border_width(GTK_CONTAINER(g->window), 4);
	g_signal_connect(G_OBJECT(g->window), "delete-event", G_CALLBACK(
				_on_closex), g);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(g->window), vbox);
	/* hostname */
	g->hn_frame = gtk_frame_new("Connection settings");
	gtk_box_pack_start(GTK_BOX(vbox), g->hn_frame, FALSE, FALSE, 0);
	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	gtk_container_add(GTK_CONTAINER(g->hn_frame), vbox2);
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, TRUE, TRUE, 0);
	/* hostname: hostname */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	widget = gtk_label_new("Hostname");
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	g->hn_ehostname = gtk_entry_new();
	if((p = config_get(g->config, "", "hostname")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_ehostname), p);
	g_signal_connect(G_OBJECT(g->hn_ehostname), "activate", G_CALLBACK(
				_on_connect), g);
	gtk_box_pack_start(GTK_BOX(vbox2), g->hn_ehostname, TRUE, TRUE, 0);
	/* hostname: port */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	widget = gtk_label_new("Port");
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	g->hn_sport = gtk_spin_button_new_with_range(0, 65535, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(g->hn_sport), SSH_PORT);
	if((p = config_get(g->config, "", "port")) != NULL)
	{
		i = strtol(p, &q, 10);
		if(*q == '\0' && i <= 65535)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(g->hn_sport),
					i);
	}
	gtk_box_pack_start(GTK_BOX(vbox2), g->hn_sport, TRUE, TRUE, 0);
	/* hostname: username */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	widget = gtk_label_new("User name");
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	g->hn_eusername = gtk_entry_new();
	if((p = config_get(g->config, "", "username")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(g->hn_eusername), p);
	g_signal_connect(G_OBJECT(g->hn_eusername), "activate", G_CALLBACK(
				_on_connect), g);
	gtk_box_pack_start(GTK_BOX(vbox2), g->hn_eusername, TRUE, TRUE, 0);
	/* sessions */
	g->sn_frame = gtk_frame_new("Manage sessions");
	vbox2 = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->sn_frame), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox), g->sn_frame, TRUE, TRUE, 0);
	/* sessions: list */
	vbox2 = gtk_vbox_new(FALSE, 4);
	g->sn_esessions = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox2), g->sn_esessions, FALSE, FALSE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	model = gtk_list_store_new(GC_COUNT, G_TYPE_STRING);
	g->sn_tlsessions = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(
				g->sn_tlsessions), -1, "Session",
			gtk_cell_renderer_text_new(), "text", GC_COL_NAME,
			NULL);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(g->sn_tlsessions),
			FALSE);
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(g->sn_tlsessions));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(
				_on_session_select), g);
	g_signal_connect(G_OBJECT(g->sn_tlsessions), "row-activated",
			G_CALLBACK(_on_session_activate), g);
	gtk_container_add(GTK_CONTAINER(widget), g->sn_tlsessions);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	/* sessions: buttons */
	vbox2 = gtk_vbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_load), g);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_save), g);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_delete),
			g);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_move_up),
			g);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_move_down),
			g);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 0);
	/* actions */
	hbox = gtk_hbox_new(FALSE, 4);
#if GTK_CHECK_VERSION(2, 6, 0)
	widget = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
#else
	widget = gtk_button_new_with_mnemonic("_About");
#endif
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_about), g);
	widget = gtk_button_new_from_stock(GTK_STOCK_PREFERENCES);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences), g);
	widget = gtk_button_new_with_mnemonic("_Connect");
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_connect),
			g);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_widget_add_accelerator(widget, "clicked", group, GDK_Q,
			GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(_on_exit), 
			g);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	/* preferences */
	g->pr_window = NULL;

	/* load sessions */
	for(i = 0; i < 100; i++)
	{
		snprintf(buf, sizeof(buf), "session %u", i);
		if((p = config_get(g->config, buf, "name")) == NULL)
			break;
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, GC_COL_NAME, p, -1);
	}

	/* show window */
	gtk_widget_show_all(g->window);
	gtk_window_add_accel_group(GTK_WINDOW(g->window), group);
	return g;
}


void gputty_delete(GPuTTY * gputty)
{
	gtk_widget_destroy(gputty->window);
	config_delete(gputty->config);
	free(gputty);
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
