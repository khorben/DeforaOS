/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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
/* TODO:
 * - more error checking
 * - determine if there is an asynchronous mode */



#include <System.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Panel.h"


/* wpa_supplicant */
/* private */
/* types */
typedef enum _WpaCommand { WC_LIST_NETWORKS, WC_STATUS } WpaCommand;

typedef struct _WpaEntry
{
	WpaCommand command;
	char * buf;
	size_t buf_cnt;
} WpaEntry;

typedef struct _Wpa
{
	guint source;
	int fd;
	GIOChannel * channel;
	guint rd_source;
	guint wr_source;

	WpaEntry * queue;
	size_t queue_cnt;

	/* widgets */
	GtkWidget * image;
	GtkWidget * label;
} Wpa;


/* prototypes */
static GtkWidget * _wpa_init(PanelApplet * applet);
static void _wpa_destroy(PanelApplet * applet);

static int _wpa_queue(PanelApplet * applet, WpaCommand command, ...);

/* callbacks */
static gboolean _on_timeout(gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Wifi",
	NULL,
	_wpa_init,
	_wpa_destroy,
	NULL,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* wpa_init */
static gboolean _init_timeout(gpointer data);

static GtkWidget * _wpa_init(PanelApplet * applet)
{
	Wpa * wpa;
	PangoFontDescription * bold;
	GtkWidget * hbox;

	if((wpa = object_new(sizeof(*wpa))) == NULL)
		return NULL;
	applet->priv = wpa;
	wpa->source = 0;
	wpa->fd = -1;
	wpa->channel = NULL;
	wpa->rd_source = 0;
	wpa->wr_source = 0;
	wpa->queue = NULL;
	wpa->queue_cnt = 0;
	/* widgets */
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	hbox = gtk_hbox_new(FALSE, 4);
	/* FIXME use this icon in the battery applet for charging */
	wpa->image = gtk_image_new_from_stock(GTK_STOCK_DISCONNECT,
			applet->helper->icon_size);
	gtk_box_pack_start(GTK_BOX(hbox), wpa->image, FALSE, TRUE, 0);
	wpa->label = gtk_label_new(" ");
	gtk_widget_modify_font(wpa->label, bold);
	gtk_box_pack_start(GTK_BOX(hbox), wpa->label, FALSE, TRUE, 0);
	if(_init_timeout(applet) != FALSE)
		wpa->source = g_timeout_add(5000, _init_timeout, applet);
	gtk_widget_show_all(hbox);
	pango_font_description_free(bold);
	return hbox;
}

static gboolean _init_timeout(gpointer data)
{
	int ret = TRUE;
	PanelApplet * applet = data;
	Wpa * wpa = applet->priv;
	char const path[] = "/var/run/wpa_supplicant";
	char const local[] = "/tmp/panel_wpa_supplicant"; /* XXX random */
	DIR * dir;
	struct dirent * de;
	struct stat st;
	struct sockaddr_un lu;
	struct sockaddr_un ru;

	if((dir = opendir(path)) == NULL)
		return applet->helper->error(NULL, path, TRUE);
	if((wpa->fd = socket(PF_LOCAL, SOCK_DGRAM, 0)) == -1)
		return applet->helper->error(applet->helper->panel, "socket",
				TRUE);
	lu.sun_family = AF_UNIX;
	lu.sun_len = snprintf(lu.sun_path, sizeof(lu.sun_path), "%s", local);
	if(bind(wpa->fd, (struct sockaddr *)&lu, sizeof(lu)) != 0)
	{
		close(wpa->fd);
		unlink(local);
		return applet->helper->error(NULL, local, TRUE);
	}
	ru.sun_family = AF_UNIX;
	while((de = readdir(dir)) != NULL)
	{
		if(snprintf(ru.sun_path, sizeof(ru.sun_path), "%s/%s", path,
					de->d_name) >= (int)sizeof(ru.sun_path)
				|| lstat(ru.sun_path, &st) != 0
				|| (st.st_mode & S_IFSOCK) != S_IFSOCK)
			continue;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, de->d_name);
#endif
		if(connect(wpa->fd, (struct sockaddr *)&ru, sizeof(ru)) != 0)
			continue;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() connected\n", __func__);
#endif
		wpa->channel = g_io_channel_unix_new(wpa->fd);
		g_io_channel_set_encoding(wpa->channel, NULL, NULL);
		g_io_channel_set_buffered(wpa->channel, FALSE);
		_on_timeout(applet);
		wpa->source = g_timeout_add(5000, _on_timeout, applet);
		ret = FALSE;
		break;
	}
	if(ret == TRUE)
		close(wpa->fd);
	closedir(dir);
	return ret;
}


/* wpa_destroy */
static void _wpa_destroy(PanelApplet * applet)
{
	Wpa * wpa = applet->priv;
	size_t i;

	/* FIXME remove timeouts and sources */
	for(i = 0; i < wpa->queue_cnt; i++)
		free(wpa->queue[i].buf);
	free(wpa->queue);
	object_delete(wpa);
}


/* wpa_queue */
static int _wpa_queue(PanelApplet * applet, WpaCommand command, ...)
{
	Wpa * wpa = applet->priv;
	char const * cmd = NULL;
	WpaEntry * p;

	switch(command)
	{
		case WC_LIST_NETWORKS:
			cmd = "LIST_NETWORKS";
			break;
		case WC_STATUS:
			cmd = "STATUS-VERBOSE";
			break;
	}
	if(cmd == NULL)
		return -1;
	if((p = realloc(wpa->queue, sizeof(*p) * (wpa->queue_cnt + 1))) == NULL)
		return -1;
	wpa->queue = p;
	p = &wpa->queue[wpa->queue_cnt];
	p->command = command;
	p->buf = strdup(cmd);
	p->buf_cnt = strlen(cmd);
	if(p->buf == NULL)
		return -1;
	if(wpa->queue_cnt++ == 0)
		wpa->wr_source = g_io_add_watch(wpa->channel, G_IO_OUT,
				_on_watch_can_write, applet);
	return 0;
}


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	PanelApplet * applet = data;

	_wpa_queue(applet, WC_STATUS);
	return TRUE;
}


/* on_watch_can_read */
static gboolean _read_status(PanelApplet * applet, Wpa * wpa, char const * buf,
		size_t cnt);
static gboolean _read_list_networks(PanelApplet * applet, Wpa * wpa,
		char const * buf, size_t cnt);

static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	int ret = FALSE;
	PanelApplet * applet = data;
	Wpa * wpa = applet->priv;
	WpaEntry * entry = &wpa->queue[0];
	char buf[256]; /* XXX in wpa */
	gsize cnt;
	GError * error = NULL;
	GIOStatus status;

	if(condition != G_IO_IN || source != wpa->channel
			|| wpa->queue_cnt == 0 || entry->buf_cnt != 0)
		return FALSE; /* should not happen */
	status = g_io_channel_read_chars(source, buf, sizeof(buf), &cnt,
			&error);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"", __func__);
	fwrite(buf, sizeof(*buf), cnt, stderr);
	fprintf(stderr, "\"\n");
#endif
	switch(entry->command)
	{
		case WC_LIST_NETWORKS:
			ret = _read_list_networks(applet, wpa, buf, cnt);
			break;
		case WC_STATUS:
			ret = _read_status(applet, wpa, buf, cnt);
			break;
	}
	if(ret == TRUE)
		return TRUE;
	wpa->rd_source = 0;
	memmove(entry, &wpa->queue[1], sizeof(*entry) * (--wpa->queue_cnt));
	if(wpa->queue_cnt == 0)
		return FALSE;
	wpa->wr_source = g_io_add_watch(wpa->channel, G_IO_OUT,
			_on_watch_can_write, applet);
	return ret;
}

static gboolean _read_list_networks(PanelApplet * applet, Wpa * wpa,
		char const * buf, size_t cnt)
{
	size_t i;
	size_t j;
	char * p = NULL;
	char * q;
	unsigned int u;
	char ssid[80];
	char bssid[80];
	char flags[80];
	int res;

	for(i = 0; i < cnt;)
	{
		for(j = i; j < cnt; j++)
			if(buf[j] == '\n')
				break;
		if((q = realloc(p, ++j - i)) == NULL)
			continue;
		p = q;
		snprintf(p, j - i, "%s", &buf[i]);
		p[j - i - 1] = '\0';
#ifdef DEBUG
		fprintf(stderr, "DEBUG: line \"%s\"\n", p);
#endif
		if((res = sscanf(p, "%u\t%79[^\t]\t%79[^\t]\t%79s", &u, ssid,
						bssid, flags)) == 4)
			if(strcmp(flags, "[CURRENT]") == 0)
			{
				gtk_image_set_from_stock(GTK_IMAGE(wpa->image),
						GTK_STOCK_CONNECT,
						applet->helper->icon_size);
				gtk_label_set_text(GTK_LABEL(wpa->label), ssid);
				break;
			}
		i = j;
	}
	free(p);
	return FALSE;
}

static gboolean _read_status(PanelApplet * applet, Wpa * wpa, char const * buf,
		size_t cnt)
{
	size_t i;
	size_t j;
	char * p = NULL;
	char * q;
	char variable[80];
	char value[80];

	for(i = 0; i < cnt;)
	{
		for(j = i; j < cnt; j++)
			if(buf[j] == '\n')
				break;
		if((q = realloc(p, ++j - i)) == NULL)
			continue;
		p = q;
		snprintf(p, j - i, "%s", &buf[i]);
		p[j - i - 1] = '\0';
#ifdef DEBUG
		fprintf(stderr, "DEBUG: line \"%s\"\n", p);
#endif
		if(sscanf(p, "%79[^=]=%79[^\n]", variable, value) != 2)
			continue;
		if(strcmp(variable, "wpa_state") == 0)
			gtk_image_set_from_stock(GTK_IMAGE(wpa->image),
					(strcmp(value, "COMPLETED") == 0)
					? GTK_STOCK_CONNECT
					: GTK_STOCK_DISCONNECT,
					applet->helper->icon_size);
		if(strcmp(variable, "ssid") == 0)
			gtk_label_set_text(GTK_LABEL(wpa->label), value);
		i = j;
	}
	free(p);
	return FALSE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	PanelApplet * applet = data;
	Wpa * wpa = applet->priv;
	WpaEntry * entry = &wpa->queue[0];
	gsize cnt;
	GError * error = NULL;
	GIOStatus status;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_OUT || source != wpa->channel
			|| wpa->queue_cnt == 0 || entry->buf_cnt == 0)
	{
		wpa->wr_source = 0;
		return FALSE; /* should not happen */
	}
	status = g_io_channel_write_chars(source, entry->buf, entry->buf_cnt,
			&cnt, &error);
	memmove(entry->buf, &entry->buf[cnt], entry->buf_cnt - cnt);
	entry->buf_cnt -= cnt;
	if(entry->buf_cnt != 0)
		return TRUE;
	wpa->rd_source = g_io_add_watch(wpa->channel, G_IO_IN,
			_on_watch_can_read, applet);
	wpa->wr_source = 0;
	return FALSE;
}
