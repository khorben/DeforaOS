/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "surfer.h"
#include "callbacks.h"
#include "../config.h"


/* constants */
static char const * _authors[] =
{
	"Pierre 'khorben' Pronchery",
	NULL
};
static char const _license[] = "GPLv2";


/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_close(GtkWidget * widget, gpointer data)
{
	gtk_main_quit();
}


static void _surfer_fork(Surfer * surfer, char const * url);
void on_file_new_window(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;
	char * url = gtk_moz_embed_get_location(GTK_MOZ_EMBED(surfer->view));

	_surfer_fork(surfer, url);
	free(url);
}

static void _surfer_fork(Surfer * surfer, char const * url)
{
	pid_t pid;

	if((pid = fork()) == -1)
	{
		surfer_error(surfer, strerror(errno), 0);
		return;
	}
	if(pid != 0)
		return;
	execlp("surfer", "surfer", url, NULL);
	fprintf(stderr, "%s%s\n", "surfer: surfer: ", strerror(errno));
	exit(2);
}


void on_file_refresh(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_reload(GTK_MOZ_EMBED(surfer->view),
			GTK_MOZ_EMBED_FLAG_RELOADNORMAL);
}


void on_file_force_refresh(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_reload(GTK_MOZ_EMBED(surfer->view),
		       	GTK_MOZ_EMBED_FLAG_RELOADBYPASSCACHE);
}


/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


/* help menu */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif
void on_help_about(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;
	static GtkWidget * window = NULL;
	char const copyright[] = "Copyright (c) 2006 khorben";
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
		surfer_error(surfer, "Memory allocation failed", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				surfer->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
#else
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About " PACKAGE);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				surfer->window));
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(copyright), FALSE,
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
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
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
			GTK_SHADOW_ETCHED_IN);
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
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
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
#endif


/* toolbar */
void on_back(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_go_back(GTK_MOZ_EMBED(surfer->view));
}


void on_forward(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_go_forward(GTK_MOZ_EMBED(surfer->view));
}


void on_fullscreen(GtkToggleToolButton * button, gpointer data)
{
	Surfer * surfer = data;

	if(gtk_toggle_tool_button_get_active(button))
	{
		gtk_widget_hide(surfer->menubar);
		gtk_window_fullscreen(GTK_WINDOW(surfer->window));
	}
	else
	{
		gtk_widget_show(surfer->menubar);
		gtk_window_unfullscreen(GTK_WINDOW(surfer->window));
	}
}


void on_home(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_load_url(GTK_MOZ_EMBED(surfer->view),
			"http://www.defora.org/");
}


void on_path_activate(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;
	gchar * url;

	url = gtk_combo_box_get_active_text(GTK_COMBO_BOX(surfer->tb_path));
	gtk_moz_embed_load_url(GTK_MOZ_EMBED(surfer->view), url);
	g_free(url);
}


void on_refresh(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_reload(GTK_MOZ_EMBED(surfer->view),
		       	GTK_MOZ_EMBED_FLAG_RELOADNORMAL);
}


void on_stop(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	gtk_moz_embed_stop_load(GTK_MOZ_EMBED(surfer->view));
}


/* view */
void on_view_link_message(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char * url = gtk_moz_embed_get_link_message(view);

	if(surfer->statusbar_id)
		gtk_statusbar_remove(GTK_STATUSBAR(surfer->statusbar),
				gtk_statusbar_get_context_id(
					GTK_STATUSBAR(surfer->statusbar), ""),
				surfer->statusbar_id);
	surfer->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				surfer->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					surfer->statusbar), ""),
			url != NULL ? url : "Ready");
	free(url);
}


void on_view_location(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char * url;
	GtkWidget * widget;
	static int i = 0;

	url = gtk_moz_embed_get_location(view);
	widget = gtk_bin_get_child(GTK_BIN(surfer->tb_path));
	gtk_entry_set_text(GTK_ENTRY(widget), url);
	if(i == 8)
		gtk_combo_box_remove_text(GTK_COMBO_BOX(surfer->tb_path), 0);
	else
		i++;
	gtk_combo_box_append_text(GTK_COMBO_BOX(surfer->tb_path), url);
	free(url);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			gtk_moz_embed_can_go_back(view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			gtk_moz_embed_can_go_forward(view));
}


void on_view_net_start(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;

	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			gtk_moz_embed_can_go_back(view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			gtk_moz_embed_can_go_forward(view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_refresh), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
}


void on_view_net_stop(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;

	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			gtk_moz_embed_can_go_back(view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			gtk_moz_embed_can_go_forward(view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
	if(surfer->statusbar_id)
		gtk_statusbar_remove(GTK_STATUSBAR(surfer->statusbar),
				gtk_statusbar_get_context_id(
					GTK_STATUSBAR(surfer->statusbar), ""),
				surfer->statusbar_id);
	surfer->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				surfer->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					surfer->statusbar), ""), "Ready");
}


/* on_view_new_window */
static void _on_popup_destroy_browser(GtkMozEmbed * view, gpointer data);
static void _on_popup_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data);
static void _on_popup_title(GtkMozEmbed * view, gpointer data);
void on_view_new_window(GtkMozEmbed * view, GtkMozEmbed ** ret, guint mask,
		gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * newview;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_window_set_title(GTK_WINDOW(window), "Web surfer");
	if((mask & GTK_MOZ_EMBED_FLAG_WINDOWRESIZEON)
			!= GTK_MOZ_EMBED_FLAG_WINDOWRESIZEON)
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	if((mask & GTK_MOZ_EMBED_FLAG_MODAL)
			== GTK_MOZ_EMBED_FLAG_MODAL)
		gtk_window_set_transient_for(GTK_WINDOW(window),
				GTK_WINDOW(surfer->window));
	vbox = gtk_vbox_new(FALSE, 0);
	if((mask & GTK_MOZ_EMBED_FLAG_MENUBARON)
			== GTK_MOZ_EMBED_FLAG_MENUBARON)
	{
		/* FIXME implement */
	}
	newview = gtk_moz_embed_new();
	g_signal_connect(G_OBJECT(newview), "destroy_browser", G_CALLBACK(
				_on_popup_destroy_browser), window);
	g_signal_connect(G_OBJECT(surfer->view), "size_to", G_CALLBACK(
				_on_popup_resize), window);
	g_signal_connect(G_OBJECT(newview), "title", G_CALLBACK(
				_on_popup_title), window);
	/* FIXME other settings and callbacks */
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(newview), TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	*ret = GTK_MOZ_EMBED(newview);
}

static void _on_popup_destroy_browser(GtkMozEmbed * view, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_destroy(window);
}

static void _on_popup_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data)
{
	GtkWindow * window = data;

	gtk_window_resize(window, width, height);
}

static void _on_popup_title(GtkMozEmbed * view, gpointer data)
{
	GtkWindow * window = data;
	char * title;
	char buf[256];

	title = gtk_moz_embed_get_title(view);
	if(title[0] == '\0')
		gtk_window_set_title(window, "Web surfer");
	else
	{
		snprintf(buf, sizeof(buf), "%s%s", "Web surfer - ", title);
		gtk_window_set_title(window, buf);
	}
	free(title);
}


void on_view_progress(GtkMozEmbed * view, gint cur, gint max, gpointer data)
{
	Surfer * surfer = data;
	char buf[256];

	if(surfer->statusbar_id)
		gtk_statusbar_remove(GTK_STATUSBAR(surfer->statusbar),
				gtk_statusbar_get_context_id(
					GTK_STATUSBAR(surfer->statusbar), ""),
				surfer->statusbar_id);
	if(max > 1024 || max <= 0)
		snprintf(buf, sizeof(buf), "%s%u%s%u%s", "Transferring data (",
				cur / 1024, " on ", max / 1024,
				" KB received)");
	else
		snprintf(buf, sizeof(buf), "%s%u%s%u%s", "Transferring data (",
				cur, " on ", max, " bytes received)");
	surfer->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				surfer->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					surfer->statusbar), ""), buf);
}


void on_view_resize(GtkMozEmbed * view, gint width, gint height, gpointer data)
{
	Surfer * surfer = data;

	gtk_window_resize(GTK_WINDOW(surfer->window), width, height);
}


void on_view_title(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char * title;
	char buf[256];

	title = gtk_moz_embed_get_title(view);
	if(title[0] == '\0')
		gtk_window_set_title(GTK_WINDOW(surfer->window), "Web surfer");
	else
	{
		snprintf(buf, sizeof(buf), "%s%s", "Web surfer - ", title);
		gtk_window_set_title(GTK_WINDOW(surfer->window), buf);
	}
	free(title);
}
