/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "compose.h"
#include "../config.h"
#include "common.c"
#define _(string) gettext(string)
#define N_(string) (string)


/* Compose */
/* private */
/* types */
struct _Compose
{
	Mailer * mailer;

	/* sending mail */
	pid_t pid;
	int fd;
	char * buf;
	size_t buf_len;
	size_t buf_pos;
	GIOChannel * channel;
	GtkWidget * snd_window;
	GtkWidget * snd_progress;

	/* widgets */
	GtkWidget * window;
	GtkWidget * from;
	GtkWidget * to;
	GtkWidget * tb_cc;
	GtkWidget * cc;
	GtkWidget * tb_bcc;
	GtkWidget * bcc;
	GtkWidget * subject;
	GtkWidget * view;
	GtkWidget * statusbar;
	gint statusbar_id;
	GtkWidget * ab_window;
};


/* constants */
#define SENDMAIL "/usr/sbin/sendmail"

#ifndef EMBEDDED
static DesktopMenu _menu_file[] =
{
	{ N_("_New message"), G_CALLBACK(on_compose_file_new),
		"stock_mail-compose", GDK_CONTROL_MASK, GDK_N },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Save"), NULL, GTK_STOCK_SAVE, GDK_CONTROL_MASK, GDK_S },
	{ N_("Save _as..."), NULL, GTK_STOCK_SAVE_AS, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Print"), NULL, GTK_STOCK_PRINT, GDK_CONTROL_MASK, GDK_P },
	{ N_("Print pre_view"), NULL, GTK_STOCK_PRINT_PREVIEW, GDK_CONTROL_MASK,
		0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("S_end"), G_CALLBACK(on_compose_file_send), "stock_mail-send",
		GDK_CONTROL_MASK, GDK_Return },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_compose_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_edit[] =
{
	{ N_("_Undo"), NULL, GTK_STOCK_UNDO, GDK_CONTROL_MASK, GDK_Z },
	{ N_("_Redo"), NULL, GTK_STOCK_REDO, GDK_CONTROL_MASK, GDK_Y },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Select all"), NULL,
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		"edit-select-all",
#endif
		GDK_CONTROL_MASK, GDK_A },
	{ N_("_Unselect all"), NULL, NULL, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Cut"), NULL, GTK_STOCK_CUT, GDK_CONTROL_MASK, GDK_X },
	{ N_("_Copy"), NULL, GTK_STOCK_COPY, GDK_CONTROL_MASK, GDK_C },
	{ N_("_Paste"), NULL, GTK_STOCK_PASTE, GDK_CONTROL_MASK, GDK_P },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_view[] =
{
	{ N_("_CC field"), G_CALLBACK(on_compose_view_cc), NULL, 0, 0 },
	{ N_("_BCC field"), G_CALLBACK(on_compose_view_bcc), NULL, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_compose_help_about), GTK_STOCK_ABOUT, 0,
		0 },
#else
	{ N_("_About"), G_CALLBACK(on_compose_help_about), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _compose_menubar[] =
{
	{ N_("_File"), _menu_file },
	{ N_("_Edit"), _menu_edit },
	{ N_("_View"), _menu_view },
	{ N_("_Help"), _menu_help },
	{ NULL, NULL }
};
#endif

static DesktopToolbar _compose_toolbar[] =
{
	{ N_("Send"), G_CALLBACK(on_compose_send), "stock_mail-send", 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Save"), G_CALLBACK(on_compose_save), GTK_STOCK_SAVE, 0, 0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Attach"), NULL, "stock_attach", 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* public */
/* compose_new */
static GtkWidget * _new_text_view(Mailer * mailer);

Compose * compose_new(Mailer * mailer)
{
	Compose * compose;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkSizeGroup * sizegroup;
	GtkWidget * widget;

	if((compose = malloc(sizeof(*compose))) == NULL)
	{
		mailer_error(mailer, strerror(errno), 0);
		return NULL;
	}
	compose->mailer = mailer;
	group = gtk_accel_group_new();
	compose->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(compose->window), group);
	gtk_window_set_title(GTK_WINDOW(compose->window),
			_(PACKAGE " - Compose"));
	gtk_window_set_default_size(GTK_WINDOW(compose->window), 512, 384);
	g_signal_connect_swapped(G_OBJECT(compose->window), "delete-event",
			G_CALLBACK(on_compose_closex), compose);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	widget = desktop_menubar_create(_compose_menubar, compose, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
#endif
	/* toolbar */
	toolbar = desktop_toolbar_create(_compose_toolbar, compose, group);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* FIXME make these fields a GtkTreeView */
	/* from */
	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(_(" From: "));
	gtk_size_group_add_widget(sizegroup, widget);
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
	widget = gtk_label_new(_(" To: "));
	gtk_size_group_add_widget(sizegroup, widget);
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
	widget = gtk_label_new(_(" CC: "));
	gtk_size_group_add_widget(sizegroup, widget);
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
	widget = gtk_label_new(_(" BCC: "));
	gtk_size_group_add_widget(sizegroup, widget);
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
	widget = gtk_label_new(_(" Subject: "));
	gtk_size_group_add_widget(sizegroup, widget);
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
	compose->ab_window = NULL;
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
/* compose_get_mailer */
Mailer * compose_get_mailer(Compose * compose)
{
	return compose->mailer;
}


/* compose_set_subject */
void compose_set_subject(Compose * compose, char const * subject)
{
	gtk_entry_set_text(GTK_ENTRY(compose->subject), subject);
}


/* compose_set_to */
void compose_set_to(Compose * compose, char const * to)
{
	gtk_entry_set_text(GTK_ENTRY(compose->to), to);
}


/* useful */
/* compose_save */
int compose_save(Compose * compose)
{
	/* FIXME implement */
	return 0;
}


/* compose_send */
static char * _send_headers(Compose * compose);
static char * _send_body(GtkWidget * view);
static int _send_mail(Compose * compose, char * msg, size_t msg_len);
static int _mail_child(int fd[2]);
static gboolean _on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data);

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
			_("Sending mail..."));
	gtk_window_set_resizable(GTK_WINDOW(compose->snd_window), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(compose->snd_window),
			GTK_WINDOW(compose->window));
	g_signal_connect_swapped(G_OBJECT(compose->snd_window), "delete-event",
			G_CALLBACK(on_send_closex), compose);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Progression: ")),
			FALSE, FALSE, 0);
	compose->snd_progress = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(compose->snd_progress),
			0.0);
	gtk_box_pack_start(GTK_BOX(hbox), compose->snd_progress, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
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
	g_io_add_watch(compose->channel, G_IO_OUT, _on_send_write, compose);
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
		{ "To: ", NULL },
		{ "Cc: ", NULL },
		{ "Bcc: ", NULL },
		{ "Subject: ", NULL },
		{ NULL, NULL }
	};
	int i;
	char * msg = NULL;
	size_t msg_len = 0;
	char const * p;
	size_t len;
	size_t hdr_len;
	char * q;

	widgets[0].wgt = compose->to;
	widgets[1].wgt = compose->cc;
	widgets[2].wgt = compose->bcc;
	widgets[3].wgt = compose->subject;
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

static gboolean _on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Compose * c = data;
	gsize i;

	if(condition != G_IO_OUT)
		return FALSE;
	if((i = (c->buf_len - c->buf_pos) % 512) == 0)
		i = 512;
	if(g_io_channel_write_chars(source, &c->buf[c->buf_pos], i, &i, NULL)
			!= G_IO_STATUS_NORMAL)
	{
		mailer_error(c->mailer, strerror(errno), FALSE);
		on_send_cancel(c);
		return FALSE;
	}
	c->buf_pos+=i;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(c->snd_progress),
			c->buf_pos / c->buf_len);
	if(c->buf_pos >= c->buf_len)
	{
		on_send_cancel(c);
		compose_delete(c);
		return FALSE;
	}
	return TRUE;
}


/* compose_send_cancel */
void compose_send_cancel(Compose * compose)
{
	/* FIXME verify that a send operation is in progress */
	g_io_channel_shutdown(compose->channel, TRUE, NULL);
	gtk_widget_destroy(compose->snd_window);
	free(compose->buf);
}


/* compose_show_about */
void compose_show_about(Compose * compose, gboolean show)
{
	GtkWidget * dialog;

	if(compose->ab_window != NULL)
	{
		if(show)
			gtk_widget_show(compose->ab_window);
		else
			gtk_widget_hide(compose->ab_window);
		return;
	}
	dialog = desktop_about_dialog_new();
	compose->ab_window = dialog;
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
				compose->window));
	desktop_about_dialog_set_name(dialog, PACKAGE);
	desktop_about_dialog_set_version(dialog, VERSION);
	desktop_about_dialog_set_authors(dialog, _authors);
	desktop_about_dialog_set_copyright(dialog, _copyright);
	desktop_about_dialog_set_logo_icon_name(dialog, "stock_mail");
	desktop_about_dialog_set_license(dialog, _license);
	gtk_widget_show(dialog);
}


/* compose_toggle_show_bcc */
void compose_toggle_show_bcc(Compose * compose)
{
	/* FIXME implement correctly */
	gtk_widget_show(compose->tb_bcc);
}


/* compose_toggle_show_cc */
void compose_toggle_show_cc(Compose * compose)
{
	/* FIXME implement correctly */
	gtk_widget_show(compose->tb_cc);
}
