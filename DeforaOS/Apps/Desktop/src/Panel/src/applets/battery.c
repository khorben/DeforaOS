/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
#ifdef __NetBSD__
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/envsys.h>
# include <fcntl.h>
# include <unistd.h>
# include <paths.h>
#endif
#include <System.h>
#include "Panel.h"


/* Battery */
/* private */
/* types */
typedef struct _Battery
{
	PanelAppletHelper * helper;
	GtkWidget * image;
	GtkWidget * scale;
	guint timeout;
#ifdef __NetBSD__
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
	GtkWidget * ret;
	Battery * battery;
	gdouble value;

	if((battery = malloc(sizeof(*battery))) == NULL)
		return NULL;
	applet->priv = battery;
	battery->helper = applet->helper;
	battery->timeout = 0;
#ifdef __NetBSD__
	battery->fd = -1;
#endif
	value = _battery_get(battery);
	if(value >= 0.0 && value <= 100.0)
		;
	else /* an error occurred */
	{
		_battery_destroy(applet);
		return NULL;
	}
	ret = gtk_hbox_new(FALSE, 0);
	battery->image = gtk_image_new_from_icon_name("battery",
			applet->helper->icon_size);
	gtk_box_pack_start(GTK_BOX(ret), battery->image, FALSE, TRUE, 0);
	battery->scale = gtk_vscale_new_with_range(0, 100, 1);
	gtk_widget_set_sensitive(battery->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(battery->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(battery->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(ret), battery->scale, FALSE, TRUE, 0);
	battery->timeout = g_timeout_add(1000, _on_timeout, battery);
	_battery_set(battery, value);
	return ret;
}


/* battery_destroy */
static void _battery_destroy(PanelApplet * applet)
{
	Battery * battery = applet->priv;

	if(battery->timeout > 0)
		g_source_remove(battery->timeout);
#ifdef __NetBSD__
	if(battery->fd != -1)
		close(battery->fd);
#endif
	free(battery);
}


/* accessors */
/* battery_get */
#ifdef __NetBSD__
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
#else
static gdouble _battery_get(Battery * battery)
{
	/* FIXME not supported */
	error_set("%s", strerror(ENOSYS));
	return -1.0;
}
#endif


/* battery_set */
static void _battery_set(Battery * battery, gdouble value)
{
	/* XXX only set it when necessary? */
	if(value < 0.0)
		gtk_image_set_from_icon_name(GTK_IMAGE(battery->image),
				"error", battery->helper->icon_size);
	else if(value <= 10.0)
		gtk_image_set_from_icon_name(GTK_IMAGE(battery->image),
				"battery-caution", battery->helper->icon_size);
	else if(value <= 20.0)
		gtk_image_set_from_icon_name(GTK_IMAGE(battery->image),
				"battery-low", battery->helper->icon_size);
	else if(value <= 100.0)
		gtk_image_set_from_icon_name(GTK_IMAGE(battery->image),
				"battery", battery->helper->icon_size);
	else
	{
		gtk_image_set_from_icon_name(GTK_IMAGE(battery->image),
				"error", battery->helper->icon_size);
		value = 0.0;
	}
	gtk_range_set_value(GTK_RANGE(battery->scale), value);
}


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Battery * battery = data;
	gdouble value;

	value = _battery_get(battery);
	if(value >= 0.0 && value <= 100.0)
		_battery_set(battery, value);
	else
		return FALSE;
	return TRUE;
}
