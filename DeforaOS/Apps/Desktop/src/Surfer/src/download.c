/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <gtk/gtk.h>
#ifdef WITH_WEBKIT
# include <webkit/webkit.h>
#else
# define GNET_EXPERIMENTAL
# include <gnet.h>
#endif


/* Download */
/* types */
typedef struct _Prefs
{
	char const * output;
	char const * user_agent;
} Prefs;

typedef struct _Download
{
	Prefs * prefs;
	char * url;

	struct timeval tv;

#ifdef WITH_WEBKIT
	WebKitDownload * conn;
#else
	FILE * fp;
	GConnHttp * conn;
#endif
	guint64 content_length;
	guint64 data_received;

	/* widgets */
	GtkWidget * window;
	GtkWidget * status;
	GtkWidget * done;
	GtkWidget * speed;
	GtkWidget * progress;
	GtkWidget * check;
	GtkWidget * cancel;

	guint timeout;
	int pulse;
} Download;


/* constants */
#define PROGNAME	"download"


/* variables */
static unsigned int _download_cnt = 0;


/* prototypes */
static Download * _download_new(Prefs * prefs, char const * url);
static void _download_delete(Download * download);
static int _download_cancel(Download * download);
static int _download_error(Download * download, char const * message, int ret);
static void _download_refresh(Download * download);
#ifndef WITH_WEBKIT
static int _download_write(Download * download);
#endif

/* callbacks */
static void _download_on_cancel(gpointer data);
static gboolean _download_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#ifndef WITH_WEBKIT
static void _download_on_http(GConnHttp * conn, GConnHttpEvent * event,
		gpointer data);
#endif
static gboolean _download_on_idle(gpointer data);
static gboolean _download_on_timeout(gpointer data);


/* functions */
static void _download_label(GtkWidget * vbox, PangoFontDescription * bold,
		GtkSizeGroup * left, GtkSizeGroup * right, char const * label,
		GtkWidget ** widget, char const * text);

static Download * _download_new(Prefs * prefs, char const * url)
{
	Download * download;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	PangoFontDescription * bold;

	if((download = malloc(sizeof(*download))) == NULL)
	{
		_download_error(NULL, "malloc", -1);
		return NULL;
	}
	download->prefs = prefs; /* XXX copy the preferences instead */
	if(gettimeofday(&download->tv, NULL) != 0)
	{
		_download_error(NULL, "gettimeofday", 1);
		return NULL;
	}
	download->url = strdup(url);
	if(prefs->output == NULL)
		prefs->output = basename(download->url);
	download->conn = NULL;
	download->data_received = 0;
	download->content_length = 0;
	download->pulse = 0;
	/* window */
	download->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s %s", "Download", url);
	gtk_window_set_title(GTK_WINDOW(download->window), buf);
	g_signal_connect(G_OBJECT(download->window), "delete-event", G_CALLBACK(
				_download_on_closex), download);
	vbox = gtk_vbox_new(FALSE, 0);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	left = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	right = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	_download_label(vbox, bold, left, right, "File: ", &download->status,
			prefs->output);
	_download_label(vbox, bold, left, right, "Status: ", &download->status,
			"Resolving...");
	_download_label(vbox, bold, left, right, "Done: ", &download->done,
			"0.0 kB");
	_download_label(vbox, bold, left, right, "Speed: ", &download->speed,
			"0.0 kB/s");
	/* progress bar */
	download->progress = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(download->progress), " ");
	gtk_box_pack_start(GTK_BOX(vbox), download->progress, TRUE, TRUE, 4);
	/* checkbox */
	download->check = gtk_check_button_new_with_label(
			"Close window when the download is complete");
	gtk_box_pack_start(GTK_BOX(vbox), download->check, TRUE, TRUE, 0);
	/* button */
	hbox = gtk_hbox_new(FALSE, 0);
	download->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(download->cancel), "clicked",
			G_CALLBACK(_download_on_cancel), download);
	gtk_box_pack_end(GTK_BOX(hbox), download->cancel, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(download->window), 4);
	gtk_container_add(GTK_CONTAINER(download->window), vbox);
	g_idle_add(_download_on_idle, download);
	_download_refresh(download);
	gtk_widget_show_all(download->window);
	_download_cnt++;
	return download;
}

static void _download_label(GtkWidget * vbox, PangoFontDescription * bold,
		GtkSizeGroup * left, GtkSizeGroup * right, char const * label,
		GtkWidget ** widget, char const * text)
{
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	*widget = gtk_label_new(label);
	gtk_widget_modify_font(*widget, bold);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0, 0);
	gtk_size_group_add_widget(left, *widget);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, TRUE, TRUE, 0);
	*widget = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0, 0);
	gtk_size_group_add_widget(right, *widget);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
}


/* download_delete */
static void _download_delete(Download * download)
{
#ifdef WITH_WEBKIT
	if(download->conn != NULL)
		webkit_download_cancel(download->conn);
	/* XXX should also unlink the (temporary) output file */
#else
	if(download->conn != NULL)
		gnet_conn_http_delete(download->conn);
	if(download->fp != NULL)
	{
		if(fclose(download->fp) != 0)
			_download_error(download, download->prefs->output, 1);
		if(unlink(download->prefs->output) != 0)
			_download_error(download, download->prefs->output, 1);
	}
#endif
	free(download->url);
	gtk_widget_destroy(download->window);
	free(download);
	if(--_download_cnt == 0)
		gtk_main_quit();
}


/* download_cancel */
static int _download_cancel(Download * download)
{
	_download_delete(download);
	return 0;
}


/* download_error */
static int _download_error(Download * download, char const * message, int ret)
{
	GtkWidget * dialog;

	if(ret < 0)
	{
		fputs(PROGNAME ": ", stderr);
		perror(message);
		return -ret;
	}
	dialog = gtk_message_dialog_new(download != NULL
			? GTK_WINDOW(download->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* download_refresh */
static void _download_refresh(Download * download)
{
	char buf[256]; /* FIXME convert to UTF-8 */
	struct timeval tv;
	double rate;
	double fraction;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u/%u\n", __func__,
			download->data_received, download->content_length);
#endif
	/* XXX should check gettimeofday() return value explicitly */
	if(download->data_received > 0 && gettimeofday(&tv, NULL) == 0)
	{
		if((tv.tv_sec = tv.tv_sec - download->tv.tv_sec) < 0)
			tv.tv_sec = 0;
		if((tv.tv_usec = tv.tv_usec - download->tv.tv_usec) < 0)
		{
			tv.tv_sec--;
			tv.tv_usec += 1000000;
		}
		rate = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
		rate = download->data_received / rate;
		snprintf(buf, sizeof(buf), "%.1f kB/s", rate);
		gtk_label_set_text(GTK_LABEL(download->speed), buf);
	}
	if(download->content_length == 0)
	{
		rate = download->data_received / 1024;
		snprintf(buf, sizeof(buf), "%.1f kB", rate);
		gtk_label_set_text(GTK_LABEL(download->done), buf);
		buf[0] = '\0';
		if(download->pulse != 0)
		{
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(
						download->progress));
			download->pulse = 0;
		}
	}
	else
	{
		rate = download->data_received / 1024;
		fraction = download->content_length / 1024;
		snprintf(buf, sizeof(buf), "%.1f of %.1f kB", rate, fraction);
		gtk_label_set_text(GTK_LABEL(download->done), buf);
		fraction = (double)download->data_received
			/ (double)download->content_length;
		snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(
					download->progress), fraction);
	}
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(download->progress), buf);
}


/* download_write */
#ifndef WITH_WEBKIT
static int _download_write(Download * download)
{
	gchar * buf;
	gsize size;
	gsize s;

	if(gnet_conn_http_steal_buffer(download->conn, &buf, &size) != TRUE)
		return 0;
	/* FIXME use a GIOChannel instead */
	s = fwrite(buf, sizeof(*buf), size, download->fp);
	g_free(buf);
	if(s == size)
	{
		download->pulse = 1;
		return 0;
	}
	_download_error(download, download->prefs->output, 0);
	_download_cancel(download);
	return 1;
}
#endif


/* callbacks */
/* download_on_cancel */
static void _download_on_cancel(gpointer data)
{
	Download * download = data;

	gtk_widget_hide(download->window);
	_download_cancel(download);
}


/* download_on_closex */
static gboolean _download_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	Download * download = data;

	gtk_widget_hide(widget);
	_download_cancel(download);
	return FALSE;
}


/* download_on_http */
#ifndef WITH_WEBKIT
static void _http_connected(Download * download);
static void _http_error(GConnHttpEventError * event, Download * download);
static void _http_data_complete(GConnHttpEventData * event,
		Download * download);
static void _http_data_partial(GConnHttpEventData * event, Download * download);
static void _http_redirect(GConnHttpEventRedirect * event, Download * download);
static void _http_resolved(GConnHttpEventResolved * event, Download * download);
static void _http_response(GConnHttpEventResponse * event, Download * download);
static void _http_timeout(Download * download);

static void _download_on_http(GConnHttp * conn, GConnHttpEvent * event,
		gpointer data)
{
	Download * download = data;

	if(download->conn != conn)
		return; /* FIXME report error */
	switch(event->type)
	{
		case GNET_CONN_HTTP_CONNECTED:
			_http_connected(download);
			break;
		case GNET_CONN_HTTP_ERROR:
			_http_error((GConnHttpEventError*)event, download);
			break;
		case GNET_CONN_HTTP_DATA_COMPLETE:
			_http_data_complete((GConnHttpEventData*)event,
					download);
			break;
		case GNET_CONN_HTTP_DATA_PARTIAL:
			_http_data_partial((GConnHttpEventData*)event,
					download);
			break;
		case GNET_CONN_HTTP_REDIRECT:
			_http_redirect((GConnHttpEventRedirect*)event,
					download);
			break;
		case GNET_CONN_HTTP_RESOLVED:
			_http_resolved((GConnHttpEventResolved*)event,
					download);
			break;
		case GNET_CONN_HTTP_RESPONSE:
			_http_response((GConnHttpEventResponse*)event,
					download);
			break;
		case GNET_CONN_HTTP_TIMEOUT:
			_http_timeout(download);
			break;
	}
}

static void _http_connected(Download * download)
{
	gtk_label_set_text(GTK_LABEL(download->status), "Connected");
	/* FIXME implement */
}

static void _http_error(GConnHttpEventError * event, Download * download)
{
	char buf[10];

	snprintf(buf, sizeof(buf), "%s %u", "Error ", event->code);
	gtk_label_set_text(GTK_LABEL(download->status), buf);
	/* FIXME implement */
}

static void _http_data_complete(GConnHttpEventData * event,
		Download * download)
{
	g_source_remove(download->timeout);
	download->timeout = 0;
	if(_download_write(download) != 0)
		return;
	download->data_received = event->data_received;
	if(event->content_length == 0)
	download->content_length = (event->content_length != 0)
		? event->content_length : event->data_received;
	if(fclose(download->fp) != 0)
		_download_error(download, download->prefs->output, 0);
	download->fp = NULL;
	_download_refresh(download);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(download->check)))
	{
		_download_cancel(download);
		return;
	}
	gtk_label_set_text(GTK_LABEL(download->status), "Complete");
	gtk_button_set_label(GTK_BUTTON(download->cancel), GTK_STOCK_CLOSE);
}

static void _http_data_partial(GConnHttpEventData * event, Download * download)
{
	if(download->content_length == 0 && download->data_received == 0)
		gtk_label_set_text(GTK_LABEL(download->status), "Downloading");
	download->data_received = event->data_received;
	download->content_length = event->content_length;
	_download_write(download);
}

static void _http_redirect(GConnHttpEventRedirect * event, Download * download)
{
	char buf[256];

	if(event->new_location != NULL)
		snprintf(buf, sizeof(buf), "%s %s", "Redirected to",
				event->new_location);
	else
		strcpy(buf, "Redirected");
	gtk_label_set_text(GTK_LABEL(download->status), buf);
	/* FIXME implement */
}

static void _http_resolved(GConnHttpEventResolved * event, Download * download)
{
	gtk_label_set_text(GTK_LABEL(download->status), "Resolved");
	/* FIXME implement */
}

static void _http_response(GConnHttpEventResponse * event, Download * download)
{
	char buf[10];

	snprintf(buf, sizeof(buf), "%s %u", "Code ", event->response_code);
	gtk_label_set_text(GTK_LABEL(download->status), buf);
	/* FIXME implement */
}

static void _http_timeout(Download * download)
{
	gtk_label_set_text(GTK_LABEL(download->status), "Timeout");
	/* FIXME implement */
}
#endif


/* download_on_idle */
static gboolean _download_on_idle(gpointer data)
{
	Download * download = data;
	Prefs * prefs = download->prefs;
	char * p = NULL;
#ifdef WITH_WEBKIT
	WebKitNetworkRequest * request;

	if((p = malloc(strlen(prefs->output) + 6)) == NULL)
	{
		_download_error(download, prefs->output, 0);
		_download_cancel(download);
		return FALSE;
	}
	/* FIXME needs to be an absolute path */
	sprintf(p, "%s%s", "file:", prefs->output);
	request = webkit_network_request_new(download->url);
	download->conn = webkit_download_new(request);
	webkit_download_set_destination_uri(download->conn, p);
	free(p);
	webkit_download_start(download->conn);
#else
	if((download->fp = fopen(prefs->output, "w")) == NULL)
	{
		_download_error(download, prefs->output, 0);
		free(p);
		_download_cancel(download);
		return FALSE;
	}
	download->conn = gnet_conn_http_new();
	if(gnet_conn_http_set_method(download->conn, GNET_CONN_HTTP_METHOD_GET,
			NULL, 0) != TRUE)
		return _download_error(download, "Unknown error", FALSE);
	gnet_conn_http_set_uri(download->conn, download->url);
	if(prefs->user_agent != NULL)
		gnet_conn_http_set_user_agent(download->conn,
				prefs->user_agent);
	gnet_conn_http_run_async(download->conn, _download_on_http, download);
#endif
	download->timeout = g_timeout_add(250, _download_on_timeout, download);
	return FALSE;
}


/* download_on_timeout */
static gboolean _download_on_timeout(gpointer data)
{
	gboolean ret = TRUE;
	Download * d = data;
#ifdef WITH_WEBKIT
	WebKitDownloadStatus status;

	/* FIXME not very efficient */
	status = webkit_download_get_status(d->conn);
	switch(status)
	{
		case WEBKIT_DOWNLOAD_STATUS_ERROR:
			ret = FALSE;
			gtk_label_set_text(GTK_LABEL(d->status), "Error");
			break;
		case WEBKIT_DOWNLOAD_STATUS_FINISHED:
			/* XXX pasted from _http_data_complete */
			ret = FALSE;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
							d->check)))
			{
				_download_cancel(d);
				break;
			}
			gtk_label_set_text(GTK_LABEL(d->status), "Complete");
			gtk_button_set_label(GTK_BUTTON(d->cancel),
					GTK_STOCK_CLOSE);
			d->data_received = webkit_download_get_current_size(
					d->conn);
			d->content_length = webkit_download_get_total_size(
					d->conn);
			break;
		case WEBKIT_DOWNLOAD_STATUS_STARTED:
			gtk_label_set_text(GTK_LABEL(d->status), "Downloading");
			d->data_received = webkit_download_get_current_size(
					d->conn);
			d->content_length = webkit_download_get_total_size(
					d->conn);
			break;
		default: /* XXX anything else to handle here? */
			break;
	}
#endif
	_download_refresh(d);
	if(ret != TRUE)
		d->timeout = 0;
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: download [-O output][-U user-agent] URL...\n"
"  -O	file to write document to\n"
"  -U	user agent string to send\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	Download ** download;
	int cnt;

	memset(&prefs, 0, sizeof(prefs));
	if(g_thread_supported() == FALSE)
		g_thread_init(NULL);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "O:U:")) != -1)
		switch(o)
		{
			case 'O':
				prefs.output = optarg;
				break;
			case 'U':
				prefs.user_agent = optarg;
				break;
			default:
				return _usage();
		}
	if((cnt = argc - optind) == 0)
		return _usage();
	if((download = malloc(sizeof(*download) * cnt)) == NULL)
		return _download_error(NULL, "malloc", -2);
	for(o = 0; o < cnt; o++)
		download[o] = _download_new(&prefs, argv[optind + o]);
	gtk_main();
	return 0;
}
