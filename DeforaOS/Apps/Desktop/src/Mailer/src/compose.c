/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "common.h"
#include "compose.h"


/* private */
/* constants */
#define SENDMAIL "/usr/sbin/sendmail"

static struct _menu _menu_file[] =
{
	{ "_New message", G_CALLBACK(on_compose_file_new), "stock_mail-compose",
		GDK_N },
	{ "", NULL, NULL, 0 },
	{ "_Save", G_CALLBACK(on_compose_file_save), GTK_STOCK_SAVE, GDK_S },
	{ "Save _as...", G_CALLBACK(on_compose_file_save_as), GTK_STOCK_SAVE_AS,
		0 },
	{ "", NULL, NULL, 0 },
	{ "_Send", G_CALLBACK(on_compose_file_send), "stock_mail-send", 0 },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(on_compose_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_edit[] =
{
	{ "_Undo", G_CALLBACK(on_compose_edit_undo), GTK_STOCK_UNDO, GDK_Z },
	{ "_Redo", G_CALLBACK(on_compose_edit_redo), GTK_STOCK_REDO, GDK_Y },
	{ "", NULL, NULL, 0 },
	{ "_Cut", G_CALLBACK(on_compose_edit_cut), GTK_STOCK_CUT, GDK_X },
	{ "_Copy", G_CALLBACK(on_compose_edit_copy), GTK_STOCK_COPY, GDK_C },
	{ "_Paste", G_CALLBACK(on_compose_edit_paste), GTK_STOCK_PASTE, GDK_P },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_view[] =
{
	/* FIXME CC and BCC should be toggle menu entries */
	{ "_CC field", G_CALLBACK(on_compose_view_cc), NULL, 0 },
	{ "_BCC field", G_CALLBACK(on_compose_view_bcc), NULL, 0 },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_compose_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_compose_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menubar _compose_menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};

static struct _toolbar _compose_toolbar[] =
{
	{ "Send", G_CALLBACK(on_compose_send), "stock_mail-send" },
	{ "", NULL, NULL },
	{ "Save", G_CALLBACK(on_compose_save), GTK_STOCK_SAVE },
	{ "", NULL, NULL },
	{ "Attach", G_CALLBACK(on_compose_attach), "stock_attach" },
	{ NULL, NULL, NULL }
};


/* public */
/* compose_new */
static GtkWidget * _new_text_view(Mailer * mailer);

Compose * compose_new(Mailer * mailer)
{
	Compose * compose;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkSizeGroup * group;
	GtkWidget * widget;

	if((compose = malloc(sizeof(*compose))) == NULL)
	{
		mailer_error(mailer, strerror(errno), 0);
		return NULL;
	}
	compose->mailer = mailer;
	compose->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(compose->window), "Mailer - Compose");
	gtk_window_set_default_size(GTK_WINDOW(compose->window), 512, 384);
	g_signal_connect(G_OBJECT(compose->window), "delete_event", G_CALLBACK(
				on_compose_closex), compose);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = common_new_menubar(GTK_WINDOW(compose->window),
			_compose_menubar, compose);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = common_new_toolbar(_compose_toolbar, compose);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* from */
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" From: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->from = gtk_combo_box_entry_new_text();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->from);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* to */
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" To: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->to = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->to);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* cc */
	compose->tb_cc = gtk_toolbar_new();
	widget = gtk_label_new(" CC: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_cc), toolitem, -1);
	compose->cc = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->cc);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_cc), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), compose->tb_cc, FALSE, FALSE, 0);
	/* bcc */
	compose->tb_bcc = gtk_toolbar_new();
	widget = gtk_label_new(" BCC: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_bcc), toolitem, -1);
	compose->bcc = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->bcc);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_bcc), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), compose->tb_bcc, FALSE, FALSE, 0);
	/* subject */
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" Subject: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->subject = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->subject);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* view */
	compose->view = _new_text_view(mailer);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), compose->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	compose->statusbar = gtk_statusbar_new();
	compose->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), compose->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(compose->window), vbox);
	gtk_widget_show_all(vbox);
	gtk_widget_hide(compose->tb_cc);
	gtk_widget_hide(compose->tb_bcc);
	gtk_widget_show(compose->window);
	return compose;
}

static GtkWidget * _new_text_view(Mailer * mailer)
{
	static const char signature[] = "/.signature";
	static const char prefix[] = "\n-- \n";
	GtkWidget * textview;
	char const * font;
	PangoFontDescription * desc;
	char const * homedir;
	char * filename;
	gchar * buf;
	size_t cnt;
	GtkTextBuffer * buffer;

	textview = gtk_text_view_new();
	/* font */
	if((font = mailer_get_config(mailer, "messages_font")) != NULL)
	{
		desc = pango_font_description_from_string(font);
		gtk_widget_modify_font(textview, desc);
		pango_font_description_free(desc);
	}
	/* signature */
	if((homedir = getenv("HOME")) == NULL)
		return textview;
	if((filename = malloc(strlen(homedir) + sizeof(signature))) == NULL)
		return textview;
	sprintf(filename, "%s%s", homedir, signature);
	if(g_file_get_contents(filename, &buf, &cnt, NULL) != TRUE)
	{
		free(filename);
		return textview;
	}
	free(filename);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(buffer, prefix, sizeof(prefix) - 1);
	gtk_text_buffer_insert_at_cursor(buffer, buf, cnt);
	return textview;
}


/* compose_delete */
void compose_delete(Compose * compose)
{
	gtk_widget_hide(compose->window);
	free(compose);
}


/* accessors */
Mailer * compose_get_mailer(Compose * compose)
{
	return compose->mailer;
}


/* useful */
/* compose_save */
void compose_save(Compose * compose)
{
	/* FIXME implement */
}


/* compose_send */
static char * _send_headers(Compose * compose);
static char * _send_body(GtkWidget * view);
static int _send_mail(Compose * compose, char * msg, size_t msg_len);
static int _mail_child(int fd[2]);

void compose_send(Compose * compose)
{
	char * msg;
	size_t msg_len;
	char * body;
	size_t body_len;
	char * p;

	if((msg = _send_headers(compose)) == NULL)
		return;
	if((body = _send_body(compose->view)) == NULL)
	{
		free(msg);
		return;
	}
	msg_len = strlen(msg);
	body_len = strlen(body);
	if((p = realloc(msg, msg_len + body_len + 8)) == NULL)
		mailer_error(compose->mailer, strerror(errno), 0);
	else
	{
		msg = p;
		snprintf(&msg[msg_len], body_len + 8, "\r\n%s\r\n.\r\n", body);
		msg_len += body_len + 7;
		_send_mail(compose, msg, msg_len);
	}
	g_free(body);
}

static int _send_mail(Compose * compose, char * msg, size_t msg_len)
{
	int fd[2];
	GtkWidget * hbox;
	GtkWidget * widget;

	if(pipe(fd) != 0 || (compose->pid = fork()) == -1)
		return mailer_error(compose->mailer, strerror(errno), 1);
	if(compose->pid == 0)
		return _mail_child(fd);
	if(close(fd[0]) != 0 || fcntl(fd[1], F_SETFL, O_NONBLOCK) == -1)
		mailer_error(compose->mailer, strerror(errno), 0);
	compose->snd_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(compose->snd_window),
			"Sending mail...");
	gtk_window_set_resizable(GTK_WINDOW(compose->snd_window), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(compose->snd_window),
			GTK_WINDOW(compose->window));
	g_signal_connect(G_OBJECT(compose->snd_window), "delete_event",
			G_CALLBACK(on_send_closex), compose);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Progression: "),
			FALSE, FALSE, 0);
	compose->snd_progress = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(compose->snd_progress),
			0.0);
	gtk_box_pack_start(GTK_BOX(hbox), compose->snd_progress, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_send_cancel), compose);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(compose->snd_window), 4);
	gtk_container_add(GTK_CONTAINER(compose->snd_window), hbox);
	gtk_widget_show_all(compose->snd_window);
	compose->fd = fd[1];
	compose->buf = msg;
	compose->buf_len = msg_len;
	compose->buf_pos = 0;
	compose->channel = g_io_channel_unix_new(fd[1]);
	g_io_add_watch(compose->channel, G_IO_OUT, on_send_write, compose);
	return 0;
}

static int _mail_child(int fd[2])
{
	if(close(fd[1]) != 0 || close(0) != 0 || dup2(fd[0], 0) == -1)
		perror("mailer");
	else
	{
		execl(SENDMAIL, "sendmail", "-bm", "-t", NULL);
		perror(SENDMAIL);
	}
	exit(2);
	return 0;
}

static char * _send_headers(Compose * compose)
{
	struct {
		char * hdr;
		GtkWidget * wgt;
	} widgets[] =
	{
		{ "To: ", compose->to },
		{ "Cc: ", compose->cc },
		{ "Bcc: ", compose->bcc },
		{ "Subject: ", compose->subject },
		{ NULL, NULL }
	};
	int i;
	char * msg = NULL;
	size_t msg_len = 0;
	char const * p;
	size_t len;
	size_t hdr_len;
	char * q;

	q = gtk_combo_box_get_active_text(GTK_COMBO_BOX(compose->from));
	if(*q != '\0')
	{
		msg_len = strlen(q) + 8;
		if((msg = malloc(msg_len + 1)) == NULL)
			return NULL;
		snprintf(msg, msg_len + 1, "%s%s\r\n", "From: ", q);
	}
	g_free(q);
	for(i = 0; widgets[i].hdr != NULL; i++)
	{
		p = gtk_entry_get_text(GTK_ENTRY(widgets[i].wgt));
		if((len = strlen(p)) == 0)
			continue;
		hdr_len = strlen(widgets[i].hdr);
		if((q = realloc(msg, msg_len + hdr_len + len + 3)) == NULL)
		{
			free(msg);
			mailer_error(compose->mailer, strerror(errno), 0);
			return NULL;
		}
		msg = q;
		snprintf(&msg[msg_len], hdr_len + len + 3, "%s%s\r\n",
				widgets[i].hdr, p);
		msg_len += hdr_len + len + 2;
	}
	if(msg != NULL)
		return msg;
	if((msg = strdup("")) == NULL)
		mailer_error(compose->mailer, strerror(errno), 0);
	return msg;
}

static char * _send_body(GtkWidget * view)
{
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;

	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	/* FIXME allocating the complete message is not optimal */
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tbuf), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tbuf), &end);
	return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tbuf), &start, &end,
			FALSE);
}
