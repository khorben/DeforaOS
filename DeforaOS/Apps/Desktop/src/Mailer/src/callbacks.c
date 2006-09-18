/* callbacks.c */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "compose.h"
#include "mailer.h"
#include "callbacks.h"
#include "../config.h"


/* constants */
static char const * _authors[] =
{
	"Pierre 'khorben' Pronchery",
	NULL
};

/* FIXME */
static char const _license[] = "GPLv2";


/* callbacks */
/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_widget_hide(widget);
	/* FIXME may be composing */
	gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_new_mail(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;

	compose_new(mailer);
}

void on_file_quit(GtkWidget * widget, gpointer data)
{
	/* FIXME may be composing */
	gtk_main_quit();
}


/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data)
{
	/* FIXME */
}


/* help menu */
void on_help_about(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
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
		mailer_error(mailer, "malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				mailer->window));
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
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	/* FIXME */
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */


/* compose window */
gboolean on_compose_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Compose * c = data;

	compose_delete(c);
	return TRUE;
}


void on_compose_save(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_save(c);
}


void on_compose_send(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_send(c);
}


/* compose file menu */
void on_compose_file_close(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_delete(c);
}


/* compose view menu */
void on_compose_view_cc(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	gtk_widget_show(c->tb_cc);
}


void on_compose_view_bcc(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	gtk_widget_show(c->tb_bcc);
}


void on_compose_help_about(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	on_help_about(widget, c->mailer);
}


/* send mail */
gboolean on_send_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Compose * c = data;

	on_send_cancel(widget, c);
	return FALSE;
}


void on_send_cancel(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	g_io_channel_shutdown(c->channel, TRUE, NULL);
	gtk_widget_destroy(c->snd_window);
	free(c->buf);
}


gboolean on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Compose * c = data;
	gsize i;

	if((i = (c->buf_len - c->buf_pos) % 512) == 0)
		i = 512;
	if(g_io_channel_write_chars(source, &c->buf[c->buf_pos], i, &i, NULL)
			!= G_IO_STATUS_NORMAL)
	{
		mailer_error(c->mailer, strerror(errno), FALSE);
		on_send_cancel(c->snd_window, c);
		return FALSE;
	}
	c->buf_pos+=i;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(c->snd_progress),
			c->buf_pos / c->buf_len);
	if(c->buf_pos >= c->buf_len)
	{
		on_send_cancel(c->snd_window, c);
		compose_delete(c);
		return FALSE;
	}
	return TRUE;
}
