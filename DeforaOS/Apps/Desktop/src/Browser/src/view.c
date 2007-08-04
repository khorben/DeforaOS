/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "mime.h"
#include "../config.h"

#define min(a, b) ((a) < (b)) ? (a) : (b)


/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
	unsigned int accel;
};

struct _menubar
{
	const char * name;
	const struct _menu * menu;
};


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};
static char const _copyright[] = "Copyright (c) 2007 Pierre Pronchery";
static char const _license[] = "GPL version 2"; /* FIXME complete */


/* View */
/* private */
/* types */
typedef struct _View
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * ab_window;
} View;


/* functions */
static View * _view_new(char const * path);
static void _view_delete(View * view);

/* useful */
static int _view_error(View * view, char const * message, int ret);

/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void _on_file_close(GtkWidget * widget, gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data);


/* constants */
static const struct _menu _menu_file[] =
{
	{ "_Close", G_CALLBACK(_on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static const struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(_on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(_on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static const struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* variables */
static Mime * _mime = NULL;
static unsigned int _view_cnt = 0;


/* functions */
/* view_new */
static GtkWidget * _new_menubar(View * view);
static GtkWidget * _new_image(View * view, char const * path);
static GtkWidget * _new_text(View * view, char const * path);

static View * _view_new(char const * path)
{
	View * view;
	struct stat st;
	char const * type;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * widget;

	if((view = malloc(sizeof(*view))) == NULL)
		return NULL; /* FIXME handle error */
	if(lstat(path, &st) != 0)
	{
		_view_error(NULL, strerror(errno), 0);
		free(view);
		return NULL;
	}
	if(_mime == NULL)
		_mime = mime_new();
	if((type = mime_type(_mime, path)) == NULL)
	{
		_view_error(NULL, "Unknown file type", 0);
		free(view);
		return NULL;
	}
	view->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", "View - ", path);
	gtk_window_set_title(GTK_WINDOW(view->window), buf);
	g_signal_connect(view->window, "delete-event", G_CALLBACK(_on_closex),
			view);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = _new_menubar(view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	if(strncmp(type, "image/", 6) == 0)
		widget = _new_image(view, path);
	else if(strncmp(type, "text/", 5) == 0)
	{
		widget = _new_text(view, path);
		gtk_window_set_default_size(GTK_WINDOW(view->window), 600, 400);
	}
	else
	{
		_view_error(NULL, "Unable to view file type", 0);
		gtk_widget_destroy(view->window);
		free(view);
		return NULL;
	}
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(view->window), vbox);
	gtk_widget_show_all(view->window);
	_view_cnt++;
	return view;
}

static GtkWidget * _new_menubar(View * view)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;
	struct _menu const * p;

	tb_menubar = gtk_menu_bar_new();
	group = gtk_accel_group_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			p = &_menubar[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == NULL)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback), view);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			if(p->accel != 0)
				gtk_widget_add_accelerator(menuitem, "activate",
						group, p->accel,
						GDK_CONTROL_MASK,
						GTK_ACCEL_VISIBLE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	gtk_window_add_accel_group(GTK_WINDOW(view->window), group);
	return tb_menubar;
}

static GtkWidget * _new_image(View * view, char const * path)
{
	GtkWidget * window;
	GdkPixbuf * pixbuf;
	GtkWidget * widget;
	int pw;
	int ph;
	GdkScreen * screen;
	int sw;
	int sh;

	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	pixbuf = gdk_pixbuf_new_from_file_at_size(path, -1, -1, NULL);
	widget = gtk_image_new_from_pixbuf(pixbuf);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(window),
			widget);
	pw = gdk_pixbuf_get_width(pixbuf) + 4;
	ph = gdk_pixbuf_get_height(pixbuf) + 4;
	screen = gdk_screen_get_default();
	sw = gdk_screen_get_width(screen);
	sh = gdk_screen_get_height(screen);
	gtk_widget_set_size_request(window, min(pw, sw), min(ph, sh));
	return window;
}

static GtkWidget * _new_text(View * view, char const * path)
{
	GtkWidget * widget;
	GtkWidget * text;
	PangoFontDescription * desc;
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	char buf[BUFSIZ];
	size_t len;

	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text), FALSE);
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(text, desc);
	pango_font_description_free(desc);
	gtk_container_add(GTK_CONTAINER(widget), text);
	if((fp = fopen(path, "r")) == NULL)
	{
		_view_error(view, strerror(errno), 0);
		return widget;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, buf, len);
	}
	fclose(fp);
	return widget;
}


/* view_delete */
static void _view_delete(View * view)
{
	gtk_widget_destroy(view->window);
	free(view);
	_view_cnt--;
}


/* useful */
/* view_error */
static void _error_response(GtkDialog * dialog, gint arg, gpointer data);

static int _view_error(View * view, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(view != NULL ? GTK_WINDOW(view->window)
			: NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), NULL);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkDialog * dialog, gint arg, gpointer data)
{
	View * view = data;

	if(view != NULL)
		_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
}


/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	View * view = data;

	_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
	return FALSE;
}


static void _on_file_close(GtkWidget * widget, gpointer data)
{
	View * view = data;

	_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
}


/* on_help_about */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif

static void _on_help_about(GtkWidget * widget, gpointer data)
{
	View * view = data;
	GtkWidget * window;
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(view->ab_window != NULL)
	{
		gtk_widget_show(view->ab_window);
		return;
	}
	view->ab_window = gtk_about_dialog_new();
	window = view->ab_window;
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				view->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), "View");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(browser->ab_window != NULL)
	{
		gtk_widget_show(browser->ab_window);
		return;
	}
	browser->ab_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = browser->ab_window;
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Browser");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_copyright), FALSE,
			FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 4);
	button = gtk_button_new_with_mnemonic("C_redits");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_with_mnemonic("_License");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_license), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif

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
	gtk_window_set_title(GTK_WINDOW(window), "License");
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


/* usage */
static int _usage(void)
{
	fputs("Usage: view file...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int i;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	for(i = optind; i < argc; i++)
		_view_new(argv[i]);
	gtk_main();
	return 0;
}
