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



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#if defined(__NetBSD__)
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/envsys.h>
# include <fcntl.h>
# include <unistd.h>
# include <paths.h>
#elif defined(__linux__)
# include <fcntl.h>
# include <unistd.h>
# include <stdio.h>
#endif
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Battery */
/* private */
/* types */
typedef enum _BatteryLevel
{
	BATTERY_LEVEL_UNKNOWN = 0,
	BATTERY_LEVEL_ERROR,
	BATTERY_LEVEL_CAUTION,
	BATTERY_LEVEL_LOW,
	BATTERY_LEVEL_NORMAL,
	BATTERY_LEVEL_CHARGING
} BatteryLevel;
#define BATTERY_LEVEL_LAST	BATTERY_LEVEL_CHARGING
#define BATTERY_LEVEL_COUNT	(BATTERY_LEVEL_LAST + 1)

typedef struct _Battery
{
	PanelAppletHelper * helper;
	BatteryLevel level;

	/* widgets */
	GtkWidget * hbox;
	GtkWidget * image;
#ifndef EMBEDDED
	GtkWidget * scale;
#endif
	guint timeout;

	/* platform-specific */
#if defined(__NetBSD__) || defined(__linux__)
	int fd;
#endif
} Battery;


/* prototypes */
static GtkWidget * _battery_init(PanelApplet * applet);
static void _battery_destroy(PanelApplet * applet);

static gdouble _battery_get(Battery * battery);
static void _battery_set(Battery * battery, gdouble value);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_battery_init,
	_battery_destroy,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* battery_init */
static GtkWidget * _battery_init(PanelApplet * applet)
{
	GtkWidget * hbox;
	Battery * battery;

	if((battery = malloc(sizeof(*battery))) == NULL)
		return NULL;
	applet->priv = battery;
	battery->helper = applet->helper;
	battery->level = -1;
	battery->timeout = 0;
#if defined(__NetBSD__) || defined(__linux__)
	battery->fd = -1;
#endif
	hbox = gtk_hbox_new(FALSE, 0);
	battery->hbox = hbox;
	battery->image = gtk_image_new_from_icon_name("battery",
			applet->helper->icon_size);
	gtk_box_pack_start(GTK_BOX(hbox), battery->image, FALSE, TRUE, 0);
#ifndef EMBEDDED
	battery->scale = gtk_vscale_new_with_range(0, 100, 1);
	gtk_widget_set_sensitive(battery->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(battery->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(battery->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(hbox), battery->scale, FALSE, TRUE, 0);
#endif
	battery->timeout = g_timeout_add(5000, _on_timeout, battery);
	_on_timeout(battery);
	gtk_widget_show(battery->image);
#ifndef EMBEDDED
	gtk_widget_show(battery->scale);
#endif
	return hbox;
}


/* battery_destroy */
static void _battery_destroy(PanelApplet * applet)
{
	Battery * battery = applet->priv;

	if(battery->timeout > 0)
		g_source_remove(battery->timeout);
#if defined(__NetBSD__) || defined(__linux__)
	if(battery->fd != -1)
		close(battery->fd);
#endif
	free(battery);
}


/* battery_set */
static void _set_image(Battery * battery, BatteryLevel level);

static void _battery_set(Battery * battery, gdouble value)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%.1lf%%", value);
	/* XXX only show when necessary? */
	if(value >= 0.0 && value <= 100.0)
		gtk_widget_show(battery->hbox);
	if(value < 0.0)
	{
		_set_image(battery, BATTERY_LEVEL_UNKNOWN);
		value = 0.0;
		snprintf(buf, sizeof(buf), "%s", _("Unknown"));
	}
	else if(value <= 10.0)
		_set_image(battery, BATTERY_LEVEL_CAUTION);
	else if(value <= 20.0)
		_set_image(battery, BATTERY_LEVEL_LOW);
	else if(value <= 100.0)
		_set_image(battery, BATTERY_LEVEL_NORMAL);
	else
	{
		_set_image(battery, BATTERY_LEVEL_ERROR);
		value = 0.0;
		snprintf(buf, sizeof(buf), "%s", _("Error"));
	}
#ifndef EMBEDDED
	gtk_range_set_value(GTK_RANGE(battery->scale), value);
#endif
#if GTK_CHECK_VERSION(2, 12, 0)
	/* FIXME use the tooltip to display the exact level */
	gtk_widget_set_tooltip_text(battery->image, buf);
#endif
}

static void _set_image(Battery * battery, BatteryLevel level)
{
	char const * icons[BATTERY_LEVEL_COUNT] =
	{
		"stock_dialog-question",
		"stock_dialog-error",
		"battery-caution",
		"battery-low",
		"battery",
		"battery" /* XXX find a better icon */
	};

	if(battery->level == level)
		return;
	battery->level = level;
	gtk_image_set_from_icon_name(GTK_IMAGE(battery->image), icons[level],
			battery->helper->icon_size);
}


/* callbacks */
/* on_timeout */
#if defined(__NetBSD__)
static int _get_tre(int fd, int sensor, envsys_tre_data_t * tre);

static gdouble _battery_get(Battery * battery)
{
	int i;
	envsys_basic_info_t info;
	envsys_tre_data_t tre;
	unsigned int rate = 0;
	unsigned int charge = 0;
	unsigned int maxcharge = 0;

	if(battery->fd == -1
			&& (battery->fd = open(_PATH_SYSMON, O_RDONLY) < 0))
	{
		error_set("%s: %s", _PATH_SYSMON, strerror(errno));
		return -1.0;
	}
	for(i = 0; i >= 0; i++)
	{
		memset(&info, 0, sizeof(info));
		info.sensor = i;
		if(ioctl(battery->fd, ENVSYS_GTREINFO, &info) == -1)
		{
			close(battery->fd);
			battery->fd = -1;
			/* FIXME why does it always break once? */
			error_set("%s: %s", "ENVSYS_GTREINFO", strerror(errno));
			return -1.0;
		}
		if(!(info.validflags & ENVSYS_FVALID))
			break;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %d \"%s\"\n", __func__, i,
				info.desc);
#endif
		if(strcmp("acpiacad0 connected", info.desc) == 0
				&& _get_tre(battery->fd, i, &tre) == 0
				&& tre.validflags & ENVSYS_FCURVALID)
			/* FIXME implement */
			continue;
		if(strncmp("acpibat ", info.desc, 7) != 0
				|| info.desc[7] == '\0'
				|| info.desc[8] != ' ')
			continue;
		if(strcmp("charge", &info.desc[9]) == 0
				&& _get_tre(battery->fd, i, &tre) == 0
				&& tre.validflags & ENVSYS_FCURVALID
				&& tre.validflags & ENVSYS_FMAXVALID)
		{
			charge += tre.cur.data_us;
			maxcharge += tre.max.data_us;
		}
		else if(strcmp("charge rate", &info.desc[9]) == 0
				&& _get_tre(battery->fd, i, &tre) == 0
				&& tre.validflags & ENVSYS_FCURVALID)
			rate += tre.cur.data_us;
		else if(strcmp("charging", &info.desc[9]) == 0
				&& _get_tre(battery->fd, i, &tre) == 0
				&& tre.validflags & ENVSYS_FCURVALID
				&& tre.cur.data_us > 0)
			/* FIXME implement */
			continue;
		else if(strcmp("discharge rate", &info.desc[9]) == 0
				&& _get_tre(battery->fd, i, &tre) == 0
				&& tre.validflags & ENVSYS_FCURVALID)
			rate += tre.cur.data_us;
	}
	return (charge * 100.0) / maxcharge;
}

static int _get_tre(int fd, int sensor, envsys_tre_data_t * tre)
{
	memset(tre, 0, sizeof(*tre));
	tre->sensor = sensor;
	if(ioctl(fd, ENVSYS_GTREDATA, tre) == -1)
		return 1;
	return !(tre->validflags & ENVSYS_FVALID);
}
#elif defined(__linux__)
static gdouble _battery_get(Battery * battery)
{
	const char apm[] = "/proc/apm";
	char buf[80];
	ssize_t buf_cnt;
	double d;
	unsigned int u;
	int i;
	int b;

	if(battery->fd == -1 && (battery->fd = open(apm, O_RDONLY)) == -1)
	{
		error_set("%s: %s", apm, strerror(errno));
		return 0.0 / 0.0;
	}
	errno = ENODATA;
	if((buf_cnt = read(battery->fd, buf, sizeof(buf))) <= 0)
	{
		error_set("%s: %s", apm, strerror(errno));
		close(battery->fd);
		battery->fd = -1;
		return 0.0 / 0.0;
	}
	buf[--buf_cnt] = '\0';
	if(sscanf(buf, "%lf %lf %x %x %x %x %d%% %d min", &d, &d, &u, &u, &u,
				&u, &b, &i) != 8)
	{
		error_set("%s: %s", apm, strerror(errno));
		d = 0.0 / 0.0;
	}
	else
		d = b;
	close(battery->fd);
	battery->fd = -1;
	return d;
}
#else
static gdouble _battery_get(Battery * battery)
{
	/* FIXME not supported */
	error_set("%s", strerror(ENOSYS));
	return 0.0 / 0.0;
}
#endif


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Battery * battery = data;

	_battery_set(battery, _battery_get(battery));
	return TRUE;
}
