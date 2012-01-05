/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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



#include <string.h>
#include <gtk/gtk.h>
#include <System.h>
#include <Desktop.h>
#include <Desktop/Panel.h>
#include "Locker.h"


/* Panel */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
} PanelPlugin;


/* prototypes */
/* plug-in */
static PanelPlugin * _panel_init(LockerPluginHelper * helper);
static void _panel_destroy(PanelPlugin * panel);
static void _panel_event(PanelPlugin * panel, LockerEvent event);


/* public */
/* variables */
LockerPluginDefinition plugin =
{
	"Panel",
	"gnome-monitor",
	NULL,
	_panel_init,
	_panel_destroy,
	_panel_event
};


/* private */
/* functions */
/* panel_init */
static PanelPlugin * _panel_init(LockerPluginHelper * helper)
{
	PanelPlugin * panel;

	if((panel = object_new(sizeof(*panel))) == NULL)
		return NULL;
	panel->helper = helper;
	return panel;
}


/* panel_destroy */
static void _panel_destroy(PanelPlugin * panel)
{
	object_delete(panel);
}


/* panel_event */
static void _event_show(gboolean show);

static void _panel_event(PanelPlugin * panel, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_ACTIVATING:
		case LOCKER_EVENT_LOCKING:
			_event_show(FALSE);
			break;
		case LOCKER_EVENT_UNLOCKING:
			_event_show(TRUE);
			break;
	}
}

static void _event_show(gboolean show)
{
	desktop_message_send(PANEL_CLIENT_MESSAGE, PANEL_MESSAGE_SHOW,
			PANEL_MESSAGE_SHOW_PANEL_BOTTOM
			| PANEL_MESSAGE_SHOW_PANEL_TOP, show);
}
