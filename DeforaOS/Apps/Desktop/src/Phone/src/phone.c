/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
/* FIXME:
 * - keep track of missed calls */



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"
#include "modem.h"
#include "callbacks.h"
#include "phone.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#define min(a, b) ((a) < (b) ? (a) : (b))

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* Phone */
/* private */
/* types */
typedef enum _PhoneAttachmentColumn
{
	PHONE_ATTACHMENT_COLUMN_FILENAME = 0,
	PHONE_ATTACHMENT_COLUMN_BASENAME,
	PHONE_ATTACHMENT_COLUMN_ICON
} PhoneAttachmentColumn;
#define PHONE_ATTACHMENT_COLUMN_LAST PHONE_ATTACHMENT_COLUMN_ICON
#define PHONE_ATTACHMENT_COLUMN_COUNT (PHONE_ATTACHMENT_COLUMN_LAST + 1)

typedef enum _PhoneCallType
{
	PHONE_CALL_TYPE_OUTGOING = 0,
	PHONE_CALL_TYPE_INCOMING,
	PHONE_CALL_TYPE_MISSED
} PhoneCallType;

typedef enum _PhoneContactColumn
{
	PHONE_CONTACT_COLUMN_ID = 0,
	PHONE_CONTACT_COLUMN_NAME,
	PHONE_CONTACT_COLUMN_NUMBER
} PhoneContactColumn;
#define PHONE_CONTACT_COLUMN_LAST	PHONE_CONTACT_COLUMN_NUMBER
#define PHONE_CONTACT_COLUMN_COUNT	(PHONE_CONTACT_COLUMN_LAST + 1)

typedef enum _PhoneLogsColumn
{
	PHONE_LOGS_COLUMN_CALL_TYPE = 0,
	PHONE_LOGS_COLUMN_CALL_TYPE_DISPLAY,
	PHONE_LOGS_COLUMN_NUMBER,
	PHONE_LOGS_COLUMN_DATE,
	PHONE_LOGS_COLUMN_DATE_DISPLAY
} PhoneLogsColumn;
#define PHONE_LOGS_COLUMN_LAST		PHONE_LOGS_COLUMN_DATE_DISPLAY
#define PHONE_LOGS_COLUMN_COUNT		(PHONE_LOGS_COLUMN_LAST + 1)

typedef enum _PhoneMessageColumn
{
	PHONE_MESSAGE_COLUMN_ID = 0,
	PHONE_MESSAGE_COLUMN_NUMBER,
	PHONE_MESSAGE_COLUMN_NUMBER_DISPLAY,
	PHONE_MESSAGE_COLUMN_DATE,
	PHONE_MESSAGE_COLUMN_DATE_DISPLAY,
	PHONE_MESSAGE_COLUMN_FOLDER,
	PHONE_MESSAGE_COLUMN_STATUS,
	PHONE_MESSAGE_COLUMN_WEIGHT,
	PHONE_MESSAGE_COLUMN_CONTENT
} PhoneMessageColumn;
#define PHONE_MESSAGE_COLUMN_LAST	PHONE_MESSAGE_COLUMN_CONTENT
#define PHONE_MESSAGE_COLUMN_COUNT	(PHONE_MESSAGE_COLUMN_LAST + 1)

typedef struct _PhonePluginEntry
{
	Plugin * p;
	PhonePlugin * pp;
} PhonePluginEntry;

typedef enum _PhonePluginsColumn
{
	PHONE_PLUGINS_COLUMN_PLUGIN,
	PHONE_PLUGINS_COLUMN_ENABLED,
	PHONE_PLUGINS_COLUMN_FILENAME,
	PHONE_PLUGINS_COLUMN_ICON,
	PHONE_PLUGINS_COLUMN_NAME
} PhonePluginsColumn;
# define PHONE_PLUGINS_COLUMN_LAST	PHONE_PLUGINS_COLUMN_NAME
# define PHONE_PLUGINS_COLUMN_COUNT	(PHONE_PLUGINS_COLUMN_LAST + 1)

typedef void (*PhoneSettingsCallback)(Phone * phone, gboolean show, ...);

typedef enum _PhoneSettingsColumn
{
	PHONE_SETTINGS_COLUMN_CALLBACK,
	PHONE_SETTINGS_COLUMN_PLUGIN,
	PHONE_SETTINGS_COLUMN_ICON,
	PHONE_SETTINGS_COLUMN_NAME
} PhoneSettingsColumn;
# define PHONE_SETTINGS_COLUMN_LAST	PHONE_SETTINGS_COLUMN_NAME
# define PHONE_SETTINGS_COLUMN_COUNT	(PHONE_SETTINGS_COLUMN_LAST + 1)

typedef enum _PhoneTrack
{
	PHONE_TRACK_CODE_ENTERED = 0,
	PHONE_TRACK_MESSAGE_LIST,
	PHONE_TRACK_MESSAGE_SENT
} PhoneTrack;
#define PHONE_TRACK_LAST	PHONE_TRACK_MESSAGE_SENT
#define PHONE_TRACK_COUNT	(PHONE_TRACK_LAST + 1)

struct _Phone
{
	Modem * modem;
	guint source;
	Config * config;

	/* tracking */
	guint tr_source;
	gboolean tracks[PHONE_TRACK_COUNT];

	/* plugins */
	PhonePluginHelper helper;
	PhonePluginEntry * plugins;
	size_t plugins_cnt;

	/* widgets */
	PangoFontDescription * bold;

	/* call */
	GtkWidget * ca_window;
	GtkWidget * ca_name;
	GtkWidget * ca_number;
	GtkWidget * ca_answer;
	GtkWidget * ca_hangup;
	GtkWidget * ca_image;
	GtkWidget * ca_reject;
	GtkWidget * ca_close;
	GtkWidget * ca_volume;
	GtkWidget * ca_speaker;
	GtkWidget * ca_mute;

	/* code */
	ModemAuthenticationMethod en_method;
	char * en_name;
	GtkWidget * en_window;
	GtkWidget * en_title;
	GtkWidget * en_entry;
	GtkWidget * en_progress;

	/* contacts */
	GtkWidget * co_window;
	GtkListStore * co_store;
	GtkWidget * co_view;
	/* dialog */
	int co_index;
	GtkWidget * co_dialog;
	GtkWidget * co_name;
	GtkWidget * co_number;

	/* dialer */
	GtkWidget * di_window;
	GtkWidget * di_entry;

	/* logs */
	GtkWidget * lo_window;
	GtkListStore * lo_store;
	GtkWidget * lo_view;

	/* messages */
	GtkWidget * me_window;
	GtkListStore * me_store;
	GtkWidget * me_view;
	GtkWidget * me_progress;

	/* plugins */
	GtkWidget * pl_window;
	GtkListStore * pl_store;
	GtkWidget * pl_view;

	/* read */
	unsigned int re_index;
	GtkWidget * re_window;
	GtkWidget * re_name;
	GtkWidget * re_number;
	GtkWidget * re_date;
	GtkWidget * re_view;

	/* settings */
	GtkWidget * se_window;
	GtkListStore * se_store;
	GtkWidget * se_view;

	/* status */
	GtkWidget * st_window;
	unsigned int st_missed;
	unsigned int st_messages;
	GtkWidget * st_missed_box;
	GtkWidget * st_lmissed;
	GtkWidget * st_lmissed_count;
	GtkWidget * st_messages_box;
	GtkWidget * st_lmessages;
	GtkWidget * st_lmessages_count;

	/* system preferences */
	GtkWidget * sy_window;

	/* write */
	GtkWidget * wr_window;
	GtkWidget * wr_entry;
	GtkWidget * wr_count;
	GtkWidget * wr_view;
	GtkWidget * wr_awindow;
	GtkListStore * wr_astore;
	GtkWidget * wr_aview;
	GtkWidget * wr_progress;
};


/* constants */
#define PHONE_CONFIG_FILE	".phone"


/* prototypes */
static int _phone_call_number(Phone * phone, char const * number);

static void _phone_config_foreach(Phone * phone, char const * section,
		PhoneConfigForeachCallback callback, void * priv);
static char * _phone_config_filename(void);
static char const * _phone_config_get(Phone * phone, char const * section,
		char const * variable);
static int _phone_config_save(Phone * phone);
static int _phone_config_set(Phone * phone, char const * section,
		char const * variable, char const * value);
static int _phone_config_set_type(Phone * phone, char const * type,
		char const * section, char const * variable,
		char const * value);

static GtkWidget * _phone_create_button(char const * icon, char const * label);
static GtkWidget * _phone_create_dialpad(Phone * phone,
		char const * button1_image, char const * button1_label,
		GCallback button1_callback,
		char const * button2_image, char const * button2_label,
		GCallback button2_callback,
		GCallback button_callback);
static GtkWidget * _phone_create_progress(GtkWidget * parent,
		char const * text);

static int _phone_confirm(Phone * phone, GtkWidget * window,
		char const * message);
static int _phone_error(GtkWidget * window, char const * message, int ret);

static void _phone_info(Phone * phone, GtkWidget * window, char const * message,
		GCallback callback);

static gboolean _phone_messages_filter_all(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data);
static gboolean _phone_messages_filter_drafts(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data);
static gboolean _phone_messages_filter_inbox(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data);
static gboolean _phone_messages_filter_outbox(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data);
static gboolean _phone_messages_filter_trash(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data);

static void _phone_messages_get_iter(Phone * phone, GtkWidget * view,
		GtkTreeIter * iter);
static GtkWidget * _phone_messages_get_view(Phone * phone);

static GtkWidget * _phone_progress_delete(GtkWidget * widget);
static void _phone_progress_pulse(GtkWidget * widget);

static int _phone_request(Phone * phone, ModemRequest * request);

static void _phone_show_contacts_dialog(Phone * phone, gboolean show,
		int index, char const * name, char const * number);

static void _phone_track(Phone * phone, PhoneTrack what, gboolean track);

static int _phone_trigger(Phone * phone, ModemEventType event);

static int _phone_unload(Phone * phone, PhonePlugin * plugin);

/* callbacks */
static void _phone_modem_event(void * priv, ModemEvent * event);
static void _phone_modem_event_authentication(GtkWidget * widget, gint response,
		gpointer data);
static gboolean _phone_timeout_track(gpointer data);


/* more constants */
static const struct
{
	char const * icon;
	char const * name;
	GtkTreeModelFilterVisibleFunc filter;
} _phone_message_filters[5] =
{
	/* FIXME provide the icons ourselves */
	{ "stock_all",		N_("All"),	_phone_messages_filter_all    },
	{ "phone-inbox",	N_("Inbox"),	_phone_messages_filter_inbox  },
	{ "phone-sent",		N_("Outbox"),	_phone_messages_filter_outbox },
	{ "phone-drafts",	N_("Drafts"),	_phone_messages_filter_drafts },
	{ "gnome-stock-trash",	N_("Trash"),	_phone_messages_filter_trash  }
};


/* public */
/* functions */
/* phone_new */
static int _new_config(Phone * phone);
static gboolean _new_idle(gpointer data);
static void _idle_settings(Phone * phone);
static void _idle_load_plugins(Phone * phone, char const * plugins);

Phone * phone_new(char const * plugin, int retry)
{
	Phone * phone;
	char const * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, retry);
#endif
	if((phone = object_new(sizeof(*phone))) == NULL)
		return NULL;
	if(_new_config(phone) != 0)
	{
		object_delete(phone);
		return NULL;
	}
	if(plugin == NULL && (plugin = config_get(phone->config, NULL, "modem"))
			== NULL)
		plugin = "hayes";
	if(retry < 0)
	{
		retry = 1000;
		if((p = config_get(phone->config, NULL, "retry")) != NULL)
			retry = strtoul(p, NULL, 10);
	}
	phone->modem = modem_new(phone->config, plugin, retry);
	phone->source = 0;
	phone->tr_source = 0;
	memset(&phone->tracks, 0, sizeof(phone->tracks));
	phone->helper.config_foreach = _phone_config_foreach;
	phone->helper.config_get = _phone_config_get;
	phone->helper.config_set = _phone_config_set;
	phone->helper.error = phone_error;
	phone->helper.event = phone_event;
	phone->helper.request = _phone_request;
	phone->helper.trigger = _phone_trigger;
	phone->helper.phone = phone;
	phone->plugins = NULL;
	phone->plugins_cnt = 0;
	/* widgets */
	phone->bold = pango_font_description_new();
	pango_font_description_set_weight(phone->bold, PANGO_WEIGHT_BOLD);
	phone->ca_window = NULL;
	phone->en_window = NULL;
	phone->en_method = MODEM_AUTHENTICATION_METHOD_NONE;
	phone->en_name = NULL;
	phone->en_progress = NULL;
	phone->co_window = NULL;
	phone->co_store = gtk_list_store_new(PHONE_CONTACT_COLUMN_COUNT,
			G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
	phone->co_dialog = NULL;
	phone->di_window = NULL;
	phone->lo_window = NULL;
	phone->lo_store = gtk_list_store_new(PHONE_LOGS_COLUMN_COUNT,
			G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT,
			G_TYPE_STRING);
	phone->me_window = NULL;
	phone->me_store = gtk_list_store_new(PHONE_MESSAGE_COLUMN_COUNT,
			G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT,
			G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_UINT,
			G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(phone->me_store),
			PHONE_MESSAGE_COLUMN_DATE, GTK_SORT_DESCENDING);
	phone->pl_window = NULL;
	phone->pl_store = gtk_list_store_new(PHONE_PLUGINS_COLUMN_COUNT,
			G_TYPE_POINTER, G_TYPE_BOOLEAN, G_TYPE_STRING,
			GDK_TYPE_PIXBUF, G_TYPE_STRING);
	phone->re_index = -1;
	phone->re_window = NULL;
	phone->se_window = NULL;
	phone->st_window = NULL;
	phone->st_missed = 0;
	phone->st_messages = 0;
	phone->sy_window = NULL;
	phone->se_store = gtk_list_store_new(PHONE_SETTINGS_COLUMN_COUNT,
			G_TYPE_POINTER, G_TYPE_POINTER, GDK_TYPE_PIXBUF,
			G_TYPE_STRING);
	phone->wr_window = NULL;
	phone->wr_progress = NULL;
	/* check errors */
	if(phone->modem == NULL)
	{
		phone_error(NULL, error_get(), 1);
		phone_delete(phone);
		return NULL;
	}
	phone->source = g_idle_add(_new_idle, phone);
	modem_set_callback(phone->modem, _phone_modem_event, phone);
	return phone;
}

static int _new_config(Phone * phone)
{
	char * filename;

	if((phone->config = config_new()) == NULL)
		return -1;
	if((filename = _phone_config_filename()) == NULL)
		return -1;
	config_load(phone->config, filename); /* we can ignore errors */
	free(filename);
	return 0;
}

static gboolean _new_idle(gpointer data)
{
	Phone * phone = data;
	char const * plugins;
	PhoneEvent event;

	phone->source = 0;
	phone_show_call(phone, FALSE);
	phone_show_contacts(phone, FALSE);
	phone_show_dialer(phone, FALSE);
	phone_show_messages(phone, FALSE);
	phone_show_read(phone, FALSE);
	phone_show_settings(phone, FALSE);
	phone_show_status(phone, FALSE);
	phone_show_system(phone, FALSE);
	phone_show_write(phone, FALSE);
	_idle_settings(phone);
	if((plugins = config_get(phone->config, NULL, "plugins")) != NULL)
		_idle_load_plugins(phone, plugins);
	/* try to go online */
	memset(&event, 0, sizeof(event));
	event.type = PHONE_EVENT_TYPE_STARTING;
	if(phone_event(phone, &event) != 0 || modem_start(phone->modem) != 0)
		error_print(PACKAGE);
	return FALSE;
}

static void _idle_settings(Phone * phone)
{
	GtkTreeIter iter;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

	gtk_list_store_append(phone->se_store, &iter);
	gtk_list_store_set(phone->se_store, &iter,
			PHONE_SETTINGS_COLUMN_CALLBACK, phone_show_system,
			PHONE_SETTINGS_COLUMN_PLUGIN, NULL,
			PHONE_SETTINGS_COLUMN_NAME, _("System preferences"),
			-1);
	theme = gtk_icon_theme_get_default();
	if((pixbuf = gtk_icon_theme_load_icon(theme, "gnome-settings", 48, 0,
					NULL)) != NULL)
		gtk_list_store_set(phone->se_store, &iter,
				PHONE_SETTINGS_COLUMN_ICON, pixbuf, -1);
	gtk_list_store_append(phone->se_store, &iter);
	gtk_list_store_set(phone->se_store, &iter,
			PHONE_SETTINGS_COLUMN_CALLBACK, phone_show_plugins,
			PHONE_SETTINGS_COLUMN_PLUGIN, NULL,
			PHONE_SETTINGS_COLUMN_NAME, _("Plug-ins"),
			-1);
	theme = gtk_icon_theme_get_default();
	if((pixbuf = gtk_icon_theme_load_icon(theme, "gnome-settings", 48, 0,
					NULL)) != NULL)
		gtk_list_store_set(phone->se_store, &iter,
				PHONE_SETTINGS_COLUMN_ICON, pixbuf, -1);
}

static void _idle_load_plugins(Phone * phone, char const * plugins)
{
	char * p;
	char * q;
	size_t i;

	if((p = strdup(plugins)) == NULL)
		return; /* XXX report error */
	for(q = p, i = 0;;)
	{
		if(q[i] == '\0')
		{
			phone_load(phone, q); /* we can ignore errors */
			break;
		}
		if(q[i++] != ',')
			continue;
		q[i - 1] = '\0';
		phone_load(phone, q); /* we can ignore errors */
		q += i;
		i = 0;
	}
	free(p);
}


/* phone_delete */
void phone_delete(Phone * phone)
{
	PhoneEvent event;

	memset(&event, 0, sizeof(event));
	event.type = PHONE_EVENT_TYPE_STOPPING;
	phone_event(phone, &event); /* ignore errors */
	if(phone->modem != NULL)
		modem_stop(phone->modem);
	event.type = PHONE_EVENT_TYPE_OFFLINE;
	phone_event(phone, &event);
	phone_unload_all(phone);
	if(phone->config != NULL)
		config_delete(phone->config);
	if(phone->source != 0)
		g_source_remove(phone->source);
	if(phone->tr_source != 0)
		g_source_remove(phone->tr_source);
	pango_font_description_free(phone->bold);
	if(phone->modem != NULL)
		modem_delete(phone->modem);
	free(phone->en_name);
	object_delete(phone);
}


/* useful */
/* phone_error */
static int _error_text(char const * message, int ret);

int phone_error(Phone * phone, char const * message, int ret)
{
	if(phone == NULL)
		return _error_text(message, ret);
	return _phone_error(NULL, message, ret);
}

static int _error_text(char const * message, int ret)
{
	fprintf(stderr, "phone: %s\n", message);
	return ret;
}


/* calls */
void phone_call_answer(Phone * phone)
{
	modem_request_type(phone->modem, MODEM_REQUEST_CALL_ANSWER);
}


/* phone_call_hangup */
void phone_call_hangup(Phone * phone)
{
	modem_request_type(phone->modem, MODEM_REQUEST_CALL_HANGUP);
}


/* phone_call_mute */
void phone_call_mute(Phone * phone, gboolean mute)
{
	modem_request_type(phone->modem, MODEM_REQUEST_MUTE, mute ? 1 : 0);
}


/* phone_call_reject */
void phone_call_reject(Phone * phone)
{
	/* XXX reject seemed to work better than hangup */
	modem_request_type(phone->modem, MODEM_REQUEST_CALL_HANGUP);
}


/* phone_call_set_volume */
void phone_call_set_volume(Phone * phone, gdouble volume)
{
	PhoneEvent event;

	memset(&event, 0, sizeof(event));
	event.type = PHONE_EVENT_TYPE_SET_VOLUME;
	event.volume_set.level = volume;
	phone_event(phone, &event);
}


/* phone_call_speaker */
void phone_call_speaker(Phone * phone, gboolean speaker)
{
	PhoneEvent event;

	memset(&event, 0, sizeof(event));
	event.type = speaker ? PHONE_EVENT_TYPE_SPEAKER_ON
		: PHONE_EVENT_TYPE_SPEAKER_OFF;
	phone_event(phone, &event);
}


/* phone_code_append */
int phone_code_append(Phone * phone, char character)
{
	char const * text;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%c)\n", __func__, character);
#endif
	if((character < '0' || character > '9') && character != '*'
			&& character != '+' && character != '#')
		return 1; /* ignore the error */
	text = gtk_entry_get_text(GTK_ENTRY(phone->en_entry));
	len = strlen(text);
	if((p = malloc(len + 2)) == NULL)
		return phone_error(phone, strerror(errno), 1);
	snprintf(p, len + 2, "%s%c", text, character);
	gtk_entry_set_text(GTK_ENTRY(phone->en_entry), p);
	free(p);
	return 0;
}


/* phone_code_enter */
void phone_code_enter(Phone * phone)
{
	char const * code;
	char buf[32];

	if(phone->en_window == NULL)
		return;
	snprintf(buf, sizeof(buf), _("Checking %s..."), phone->en_name);
	switch(phone->en_method)
	{
		case MODEM_AUTHENTICATION_METHOD_PIN:
			code = gtk_entry_get_text(GTK_ENTRY(phone->en_entry));
			phone->en_progress = _phone_create_progress(
					phone->en_window, buf);
			_phone_track(phone, PHONE_TRACK_CODE_ENTERED, TRUE);
			modem_request_type(phone->modem,
					MODEM_REQUEST_AUTHENTICATE, "SIM PIN",
					NULL, code);
			break;
		default:
			break;
	}
}


/* code */
/* phone_code_clear */
void phone_code_clear(Phone * phone)
{
	_phone_track(phone, PHONE_TRACK_CODE_ENTERED, FALSE);
	phone->en_progress = _phone_progress_delete(phone->en_progress);
	if(phone->en_window != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->en_entry), "");
}


/* contacts */
/* phone_contacts_call_selected */
void phone_contacts_call_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int id;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter,
			PHONE_CONTACT_COLUMN_ID, &id, -1);
	modem_request_type(phone->modem, MODEM_REQUEST_CALL_CONTACT,
			MODEM_CALL_TYPE_VOICE, id);
}


/* phone_contacts_delete_selected */
void phone_contacts_delete_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int id;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter,
			PHONE_CONTACT_COLUMN_ID, &id, -1);
	if(_phone_confirm(phone, phone->co_window, _("Delete this contact?"))
			!= 0)
		return;
	gtk_list_store_remove(phone->co_store, &iter); /* XXX it may fail */
	modem_request_type(phone->modem, MODEM_REQUEST_CONTACT_DELETE, id);
}


/* phone_contacts_edit_selected */
void phone_contacts_edit_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int index;
	gchar * name;
	gchar * number;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter,
			PHONE_CONTACT_COLUMN_ID, &index,
			PHONE_CONTACT_COLUMN_NAME, &name,
			PHONE_CONTACT_COLUMN_NUMBER, &number, -1);
	_phone_show_contacts_dialog(phone, TRUE, index, name, number);
	g_free(name);
	g_free(number);
}


/* phone_contacts_new */
void phone_contacts_new(Phone * phone)
{
	_phone_show_contacts_dialog(phone, TRUE, -1, "", "");
}


/* phone_contacts_set */
void phone_contacts_set(Phone * phone, unsigned int index, char const * name,
		char const * number)
{
	GtkTreeModel * model = GTK_TREE_MODEL(phone->co_store);
	GtkTreeIter iter;
	gboolean valid;
	unsigned int id;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, \"%s\", \"%s\")\n", __func__, index,
			name, number);
#endif
	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, PHONE_CONTACT_COLUMN_ID, &id,
				-1);
		if(id == index)
			break;
	}
	if(valid != TRUE)
		gtk_list_store_append(phone->co_store, &iter);
	gtk_list_store_set(phone->co_store, &iter,
			PHONE_CONTACT_COLUMN_ID, index,
			PHONE_CONTACT_COLUMN_NAME, name,
			PHONE_CONTACT_COLUMN_NUMBER, number, -1);
}


/* phone_contacts_write_selected */
void phone_contacts_write_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gchar * number = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) == TRUE)
		gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter,
				PHONE_CONTACT_COLUMN_NUMBER, &number, -1);
	phone_messages_write(phone, number, NULL);
	g_free(number);
}


/* dialer */
/* phone_dialer_append */
int phone_dialer_append(Phone * phone, char character)
{
	char const * text;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%c)\n", __func__, character);
#endif
	if(phone->di_window == NULL)
		return -1;
	if((character < '0' || character > '9') && character != '*'
			&& character != '+' && character != '#')
		return -1; /* ignore the error */
	/* FIXME if in a call send DTMF instead */
	text = gtk_entry_get_text(GTK_ENTRY(phone->di_entry));
	len = strlen(text) + 2;
	if((p = malloc(len)) == NULL)
		return -phone_error(phone, strerror(errno), 1);
	snprintf(p, len, "%s%c", text, character);
	gtk_entry_set_text(GTK_ENTRY(phone->di_entry), p);
	free(p);
	return 0;
}


/* phone_dialer_call */
void phone_dialer_call(Phone * phone, char const * number)
{
	/* FIXME check if it's either a name or number */
	if(number == NULL)
		number = gtk_entry_get_text(GTK_ENTRY(phone->di_entry));
	_phone_call_number(phone, number);
}


/* phone_dialer_hangup */
void phone_dialer_hangup(Phone * phone)
{
	phone_call_hangup(phone);
	if(phone->di_window != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->di_entry), "");
}


/* events */
/* phone_event */
int phone_event(Phone * phone, PhoneEvent * event)
{
	int ret = 0;
	size_t i;
	PhonePlugin * plugin;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, event->type);
#endif
	for(i = 0; i < phone->plugins_cnt; i++)
	{
		plugin = phone->plugins[i].pp;
		if(plugin->event == NULL)
			continue;
		ret |= plugin->event(plugin, event);
	}
	return ret;
}


/* plugins */
/* phone_load */
int phone_load(Phone * phone, char const * plugin)
{
	Plugin * p;
	PhonePlugin * pp;
	PhonePluginEntry * q;
	GtkTreeIter iter;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, plugin);
#endif
	if((p = plugin_new(LIBDIR, PACKAGE, "plugins", plugin)) == NULL)
		return -phone_error(NULL, error_get(), 1);
	if((pp = plugin_lookup(p, "plugin")) == NULL)
	{
		plugin_delete(p);
		return -phone_error(NULL, error_get(), 1);
	}
	pp->helper = &phone->helper;
	if(pp->init != NULL && pp->init(pp) != 0)
	{
		plugin_delete(p);
		return -phone_error(NULL, error_get(), 1);
	}
	if((q = realloc(phone->plugins, sizeof(*q) * (phone->plugins_cnt + 1)))
			== NULL)
	{
		plugin_delete(p);
		return -phone_error(NULL, strerror(errno), 1);
	}
	phone->plugins = q;
	phone->plugins[phone->plugins_cnt].p = p;
	phone->plugins[phone->plugins_cnt++].pp = pp;
	if(pp->name != NULL && pp->settings != NULL)
	{
		gtk_list_store_append(GTK_LIST_STORE(phone->se_store), &iter);
		gtk_list_store_set(phone->se_store, &iter,
				PHONE_SETTINGS_COLUMN_CALLBACK, NULL,
				PHONE_SETTINGS_COLUMN_PLUGIN, pp,
				PHONE_SETTINGS_COLUMN_NAME, pp->name, -1);
		theme = gtk_icon_theme_get_default();
		if(pp->icon == NULL)
			pp->icon = "gnome-settings";
		if((pixbuf = gtk_icon_theme_load_icon(theme, pp->icon, 48, 0,
						NULL)) != NULL)
			gtk_list_store_set(phone->se_store, &iter,
					PHONE_SETTINGS_COLUMN_ICON, pixbuf, -1);
	}
	return 0;
}


/* logs */
/* phone_logs_call_selected */
void phone_logs_call_selected(Phone * phone)
	/* XXX code duplication */
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gchar * number = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->lo_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->lo_store), &iter,
			PHONE_LOGS_COLUMN_NUMBER, &number, -1);
	if(number == NULL)
		return;
	_phone_call_number(phone, number);
	g_free(number);
}


/* phone_logs_clear */
void phone_logs_clear(Phone * phone)
{
	gtk_list_store_clear(phone->lo_store);
}


/* phone_logs_write_selected */
void phone_logs_write_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gchar * number = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->lo_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->lo_store), &iter,
			PHONE_LOGS_COLUMN_NUMBER, &number, -1);
	if(number == NULL)
		return;
	phone_show_write(phone, TRUE, number, "");
	g_free(number);
}


/* messages */
/* phone_messages_call_selected */
void phone_messages_call_selected(Phone * phone)
{
	GtkWidget * view;
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gchar * number = NULL;

	view = _phone_messages_get_view(phone);
	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	_phone_messages_get_iter(phone, view, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(phone->me_store), &iter,
			PHONE_MESSAGE_COLUMN_NUMBER, &number, -1);
	if(number == NULL)
		return;
	_phone_call_number(phone, number);
	g_free(number);
}


/* phone_messages_delete_selected */
void phone_messages_delete_selected(Phone * phone)
{
	GtkWidget * view;
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int index;

	view = _phone_messages_get_view(phone);
	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	_phone_messages_get_iter(phone, view, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(phone->me_store), &iter,
			PHONE_MESSAGE_COLUMN_ID, &index, -1);
	if(_phone_confirm(phone, phone->me_window, _("Delete this message?"))
			!= 0)
		return;
	if(phone->re_index == index)
		phone_show_read(phone, FALSE);
	phone->me_progress = _phone_create_progress(phone->me_window,
			_("Deleting message..."));
	modem_request_type(phone->modem, MODEM_REQUEST_MESSAGE_DELETE, index);
}


/* phone_messages_read_selected */
void phone_messages_read_selected(Phone * phone)
{
	GtkWidget * view;
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int index;
	char * number;
	unsigned int date;
	char * content;

	view = _phone_messages_get_view(phone);
	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	_phone_messages_get_iter(phone, view, &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(phone->me_store), &iter,
			PHONE_MESSAGE_COLUMN_ID, &index,
			PHONE_MESSAGE_COLUMN_NUMBER, &number,
			PHONE_MESSAGE_COLUMN_DATE, &date,
			PHONE_MESSAGE_COLUMN_CONTENT, &content, -1);
	/* FIXME also tell the modem that this message is read */
	gtk_list_store_set(phone->me_store, &iter, PHONE_MESSAGE_COLUMN_STATUS,
			MODEM_MESSAGE_STATUS_READ, PHONE_MESSAGE_COLUMN_WEIGHT,
			PANGO_WEIGHT_NORMAL, -1);
	phone_show_read(phone, TRUE, index, NULL, number, date, content);
	g_free(number);
	g_free(content);
}


/* phone_messages_set */
static char * _messages_set_summary(size_t length, char const * content);

void phone_messages_set(Phone * phone, unsigned int index, char const * number,
		time_t date, ModemMessageFolder folder,
		ModemMessageStatus status, size_t length, char const * content)
{
	GtkTreeModel * model = GTK_TREE_MODEL(phone->me_store);
	GtkTreeIter iter;
	gboolean valid;
	unsigned int id;
	char const * summary;
	char * p;
	char nd[32];
	char dd[32];
	struct tm t;
	PangoWeight weight = PANGO_WEIGHT_NORMAL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, \"%s\", \"%s\")\n", __func__, index,
			number, content);
#endif
	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, PHONE_MESSAGE_COLUMN_ID, &id,
				-1);
		if(id == index)
			break;
	}
	if(valid != TRUE)
		gtk_list_store_append(phone->me_store, &iter);
	if(number == NULL)
		number = "";
	if(content == NULL)
		content = "";
	if((p = _messages_set_summary(length, content)) != NULL)
		summary = p;
	else
		summary = content;
	/* FIXME:
	 * - lookup the name from the number
	 * - this may cut in the middle of a UTF-8 character */
	snprintf(nd, sizeof(nd), "%s\n%s", number, summary);
	localtime_r(&date, &t); /* XXX gmtime_r() or localtime_r()? */
	strftime(dd, sizeof(dd), "%d/%m/%Y %H:%M:%S", &t);
	if(status != MODEM_MESSAGE_STATUS_READ)
		weight = PANGO_WEIGHT_BOLD;
	gtk_list_store_set(phone->me_store, &iter,
			PHONE_MESSAGE_COLUMN_ID, index,
			PHONE_MESSAGE_COLUMN_NUMBER, number,
			PHONE_MESSAGE_COLUMN_NUMBER_DISPLAY, nd,
			PHONE_MESSAGE_COLUMN_DATE, date,
			PHONE_MESSAGE_COLUMN_DATE_DISPLAY, dd,
			PHONE_MESSAGE_COLUMN_FOLDER, folder,
			PHONE_MESSAGE_COLUMN_STATUS, status,
			PHONE_MESSAGE_COLUMN_WEIGHT, weight,
			PHONE_MESSAGE_COLUMN_CONTENT, content, -1);
	if(index == phone->re_index)
		phone_show_read(phone, TRUE, index, NULL, number, date,
				content);
	free(p);
}

static char * _messages_set_summary(size_t length, char const * content)
{
	char * ret;
	size_t l;
	char * p;

	if(length <= 12 && (p = strchr(content, '\n')) == NULL)
		/* already short enough, and has no newline characters */
		return NULL;
	/* truncate to 12 characters, with space for ellipse */
	l = min(length, 12);
	if((ret = malloc(l + 4)) == NULL)
		return NULL;
	snprintf(ret, l + 1, "%s", content);
	/* truncate even more if there is a newline character */
	if((p = strchr(ret, '\n')) != NULL)
		*p = '\0';
	p = strchr(ret, '\0');
	/* ellipsize if relevant */
	if(strlen(ret) < length)
		snprintf(p, 4, "%s", "...");
	return ret;
}


/* phone_messages_write */
void phone_messages_write(Phone * phone, char const * number, char const * text)
{
	GtkTextBuffer * tbuf;

	phone_show_write(phone, TRUE, "", "");
	if(number != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->wr_entry), number);
	if(text != NULL)
	{
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
		gtk_text_buffer_set_text(tbuf, text, strlen(text));
	}
}


/* read */
/* phone_read_call */
void phone_read_call(Phone * phone)
{
	char const * number;

	if(phone->re_window == NULL)
		return;
	if((number = gtk_label_get_text(GTK_LABEL(phone->re_number))) == NULL)
		return;
	_phone_call_number(phone, number);
}


/* phone_read_delete */
void phone_read_delete(Phone * phone)
{
	if(_phone_confirm(phone, phone->re_window, _("Delete this message?"))
			!= 0)
		return;
	phone_show_read(phone, FALSE);
	phone->me_progress = _phone_create_progress(phone->me_window,
			_("Deleting message..."));
	modem_request_type(phone->modem, MODEM_REQUEST_MESSAGE_DELETE,
			phone->re_index);
}


/* phone_read_forward */
void phone_read_forward(Phone * phone)
{
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	gchar * text;

	if(phone->re_window == NULL)
		return;
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->re_view));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	if((text = gtk_text_buffer_get_text(tbuf, &start, &end, FALSE)) == NULL)
		return;
	phone_show_write(phone, TRUE, "", text);
	g_free(text);
}


/* phone_read_reply */
void phone_read_reply(Phone * phone)
{
	char const * number;

	if(phone->re_window == NULL)
		return;
	if((number = gtk_label_get_text(GTK_LABEL(phone->re_number))) == NULL)
		return;
	phone_show_write(phone, TRUE, number, "");
}


/* settings */
/* phone_settings_open_selected */
void phone_settings_open_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	PhoneSettingsCallback callback;
	PhonePlugin * plugin = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->se_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->se_store), &iter,
			PHONE_SETTINGS_COLUMN_CALLBACK, &callback,
			PHONE_SETTINGS_COLUMN_PLUGIN, &plugin, -1);
	if(callback != NULL)
		callback(phone, TRUE);
	else if(plugin != NULL && plugin->settings != NULL)
		plugin->settings(plugin);
}


/* show */
/* phone_show_call */
static void _show_call_window(Phone * phone);

void phone_show_call(Phone * phone, gboolean show, ...)
	/* FIXME let the API closer to ModemEvent.call */
{
	va_list ap;
	PhoneCall call;
	char const * name = NULL;
	char const * number = NULL;

	if(show == FALSE)
	{
		if(phone->ca_window != NULL)
			gtk_widget_hide(phone->ca_window);
		return;
	}
	va_start(ap, show);
	call = va_arg(ap, PhoneCall);
	if(call == PHONE_CALL_INCOMING || call == PHONE_CALL_OUTGOING)
	{
		name = va_arg(ap, char const *);
		number = va_arg(ap, char const *);
	}
	va_end(ap);
	if(phone->ca_window == NULL)
		_show_call_window(phone);
	phone_show_dialer(phone, FALSE);
	if(name != NULL)
	{
		if(name[0] == '\0')
			/* XXX look it up if we have the number */
			name = _("Unknown contact");
		gtk_label_set_text(GTK_LABEL(phone->ca_name), name);
	}
	if(number != NULL)
	{
		if(number[0] == '\0')
			/* XXX look it up if we have the name */
			number = _("Unknown number");
		gtk_label_set_text(GTK_LABEL(phone->ca_number), number);
	}
	gtk_widget_show_all(phone->ca_window);
	switch(call)
	{
		case PHONE_CALL_ESTABLISHED:
#if GTK_CHECK_VERSION(2, 6, 0)
			gtk_window_set_icon_name(GTK_WINDOW(phone->ca_window),
					"call-start"); /* XXX better icon */
#endif
			gtk_window_set_title(GTK_WINDOW(phone->ca_window),
					_("In conversation"));
			gtk_widget_hide(phone->ca_answer);
			gtk_widget_hide(phone->ca_reject);
			gtk_widget_hide(phone->ca_close);
			break;
		case PHONE_CALL_INCOMING:
#if GTK_CHECK_VERSION(2, 6, 0)
			gtk_window_set_icon_name(GTK_WINDOW(phone->ca_window),
					"call-start"); /* XXX better icon */
#endif
			gtk_window_set_title(GTK_WINDOW(phone->ca_window),
					_("Incoming call"));
			gtk_widget_hide(phone->ca_hangup);
			gtk_widget_hide(phone->ca_close);
			break;
		case PHONE_CALL_OUTGOING:
#if GTK_CHECK_VERSION(2, 6, 0)
			gtk_window_set_icon_name(GTK_WINDOW(phone->ca_window),
					"call-start"); /* XXX better icon */
#endif
			gtk_window_set_title(GTK_WINDOW(phone->ca_window),
					_("Outgoing call"));
			gtk_widget_hide(phone->ca_answer);
			gtk_widget_hide(phone->ca_reject);
			gtk_widget_hide(phone->ca_close);
			break;
		case PHONE_CALL_TERMINATED:
#if GTK_CHECK_VERSION(2, 6, 0)
			gtk_window_set_icon_name(GTK_WINDOW(phone->ca_window),
					"call-stop");
#endif
			gtk_window_set_title(GTK_WINDOW(phone->ca_window),
					_("Call finished"));
			gtk_widget_hide(phone->ca_answer);
			gtk_widget_hide(phone->ca_hangup);
			gtk_widget_hide(phone->ca_reject);
			break;
	}
	gtk_window_present(GTK_WINDOW(phone->ca_window));
}

static void _show_call_window(Phone * phone)
{
	GtkWidget * vbox;
	GtkWidget * hbox;

	/* FIXME the "Answer" button may be shown smaller than the others */
	phone->ca_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(phone->ca_window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->ca_window), "call-start");
#endif
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	/* party */
	phone->ca_name = gtk_label_new(NULL);
	gtk_widget_modify_font(phone->ca_name, phone->bold);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_name, FALSE, TRUE, 0);
	phone->ca_number = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_number, FALSE, TRUE, 0);
	/* buttons */
	/* answer */
	phone->ca_answer = _phone_create_button("call-start", _("_Answer"));
	g_signal_connect_swapped(G_OBJECT(phone->ca_answer), "clicked",
			G_CALLBACK(on_phone_call_answer), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_answer, FALSE, TRUE, 0);
	/* hangup */
	phone->ca_hangup = _phone_create_button("call-stop", _("_Hangup"));
	g_signal_connect_swapped(G_OBJECT(phone->ca_hangup), "clicked",
			G_CALLBACK(on_phone_call_hangup), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_hangup, TRUE, TRUE, 0);
	/* reject */
	phone->ca_reject = _phone_create_button("call-stop", _("_Reject"));
	g_signal_connect_swapped(G_OBJECT(phone->ca_reject), "clicked",
			G_CALLBACK(on_phone_call_reject), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_reject, TRUE, TRUE, 0);
	/* close */
	phone->ca_close = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(G_OBJECT(phone->ca_close), "clicked",
			G_CALLBACK(on_phone_call_close), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_close, TRUE, TRUE, 0);
	/* volume bar */
	hbox = gtk_hbox_new(FALSE, 4);
	phone->ca_image = gtk_image_new_from_icon_name(
			"audio-volume-muted", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), phone->ca_image, FALSE, TRUE, 0);
	phone->ca_volume = gtk_hscale_new_with_range(0.0, 1.0, 0.02);
	g_signal_connect(G_OBJECT(phone->ca_volume), "value-changed",
			G_CALLBACK(on_phone_call_volume), phone);
	gtk_box_pack_start(GTK_BOX(hbox), phone->ca_volume, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* speaker mode */
	phone->ca_speaker = gtk_toggle_button_new_with_label(_("Loudspeaker"));
	gtk_button_set_image(GTK_BUTTON(phone->ca_speaker),
			gtk_image_new_from_icon_name("stock_volume-max",
				GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(phone->ca_speaker), "toggled", G_CALLBACK(
				on_phone_call_speaker), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_speaker, TRUE, TRUE, 0);
	/* mute microphone */
	phone->ca_mute = gtk_toggle_button_new_with_label(_("Mute microphone"));
	gtk_button_set_image(GTK_BUTTON(phone->ca_mute),
			gtk_image_new_from_icon_name("audio-input-microphone",
				GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(phone->ca_mute), "toggled", G_CALLBACK(
				on_phone_call_mute), phone);
	gtk_box_pack_start(GTK_BOX(vbox), phone->ca_mute, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(phone->ca_window), vbox);
}


/* phone_show_code */
static void _show_code_window(Phone * phone);

void phone_show_code(Phone * phone, gboolean show, ...)
{
	va_list ap;
	ModemAuthenticationMethod method;
	char const * name;
	char buf[32];

	if(phone->en_window == NULL)
		_show_code_window(phone);
	if(show == FALSE)
	{
		if(phone->en_window != NULL)
			gtk_widget_hide(phone->en_window);
		return;
	}
	va_start(ap, show);
	method = va_arg(ap, ModemAuthenticationMethod);
	name = va_arg(ap, char const *);
	va_end(ap);
	if(phone->en_name == NULL || strcmp(phone->en_name, name) != 0)
		phone_code_clear(phone);
	phone->en_method = method;
	free(phone->en_name);
	if((phone->en_name = strdup(name)) == NULL)
		return; /* XXX report error */
	switch(method)
	{
		case MODEM_AUTHENTICATION_METHOD_PIN:
			snprintf(buf, sizeof(buf), _("Enter %s"), name);
			gtk_window_set_title(GTK_WINDOW(phone->en_window), buf);
			gtk_label_set_text(GTK_LABEL(phone->en_title), buf);
			break;
		default:
			break;
	}
	gtk_window_present(GTK_WINDOW(phone->en_window));
}

static void _show_code_window(Phone * phone)
{
	GtkWidget * vbox;
	GtkWidget * hbox; /* XXX create in phone_create_dialpad? */
	GtkWidget * widget;

	phone->en_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->en_window),
			"stock_lock");
#endif
	gtk_container_set_border_width(GTK_CONTAINER(phone->en_window), 4);
	g_signal_connect(G_OBJECT(phone->en_window), "delete-event", G_CALLBACK(
				on_phone_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	phone->en_title = gtk_label_new("");
	gtk_widget_modify_font(phone->en_title, phone->bold);
	gtk_box_pack_start(GTK_BOX(vbox), phone->en_title, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	phone->en_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(phone->en_entry), FALSE);
	gtk_widget_modify_font(phone->en_entry, phone->bold);
	g_signal_connect_swapped(G_OBJECT(phone->en_entry), "activate",
			G_CALLBACK(on_phone_code_enter), phone);
	gtk_box_pack_start(GTK_BOX(hbox), phone->en_entry, TRUE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"edit-undo", GTK_ICON_SIZE_BUTTON));
	gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(on_phone_code_clear), phone);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	widget = _phone_create_dialpad(phone, GTK_STOCK_OK, _("Enter"),
			G_CALLBACK(on_phone_code_enter),
			GTK_STOCK_CANCEL, _("Skip"),
			G_CALLBACK(on_phone_code_leave),
			G_CALLBACK(on_phone_code_clicked));
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(phone->en_window), vbox);
	gtk_widget_show_all(vbox);
}


/* phone_show_contacts */
void phone_show_contacts(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(phone->co_window == NULL)
	{
		phone->co_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->co_window),
				"stock_addressbook");
#endif
		gtk_window_set_default_size(GTK_WINDOW(phone->co_window), 200,
				300);
		gtk_window_set_title(GTK_WINDOW(phone->co_window),
				_("Contacts"));
		g_signal_connect(G_OBJECT(phone->co_window), "delete-event",
				G_CALLBACK(on_phone_closex), NULL);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Call"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"call-start");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_call), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Write"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-reply-sender");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_write), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_new), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_edit), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_delete), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->co_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->co_store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->co_view),
				FALSE);
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(phone->co_view),
				TRUE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"),
				renderer, "text", PHONE_CONTACT_COLUMN_NAME,
				NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				PHONE_CONTACT_COLUMN_NAME);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->co_view),
				column);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(
					phone->co_store),
				PHONE_CONTACT_COLUMN_NAME, GTK_SORT_ASCENDING);
		gtk_container_add(GTK_CONTAINER(widget), phone->co_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->co_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->co_window));
	else
		gtk_widget_hide(phone->co_window);
}


/* phone_show_dialer */
static void _show_dialer_window(Phone * phone);

void phone_show_dialer(Phone * phone, gboolean show)
{
	if(phone->di_window == NULL)
		_show_dialer_window(phone);
	if(show)
		gtk_window_present(GTK_WINDOW(phone->di_window));
	else
		gtk_widget_hide(phone->di_window);
}

static void _show_dialer_window(Phone * phone)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	phone->di_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->di_window), "phone-dialer");
#endif
	gtk_container_set_border_width(GTK_CONTAINER(phone->di_window), 4);
	gtk_window_set_title(GTK_WINDOW(phone->di_window), _("Dialer"));
	g_signal_connect(G_OBJECT(phone->di_window), "delete-event", G_CALLBACK(
				on_phone_closex), NULL);
#if !GTK_CHECK_VERSION(3, 0, 0)
	gdk_display_add_client_message_filter(gdk_display_get_default(),
			gdk_atom_intern(PHONE_CLIENT_MESSAGE, FALSE),
			on_phone_filter, phone);
#endif
	vbox = gtk_vbox_new(FALSE, 4);
	/* entry */
	hbox = gtk_hbox_new(FALSE, 4);
	phone->di_entry = gtk_entry_new();
	gtk_widget_modify_font(phone->di_entry, phone->bold);
	g_signal_connect_swapped(G_OBJECT(phone->di_entry), "activate",
			G_CALLBACK(on_phone_dialer_call), phone);
	gtk_box_pack_start(GTK_BOX(hbox), phone->di_entry, TRUE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"stock_addressbook", GTK_ICON_SIZE_BUTTON));
	gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_phone_contacts_show), phone);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* dialpad */
	widget = _phone_create_dialpad(phone, "call-start", _("Call"),
			G_CALLBACK(on_phone_dialer_call),
			"call-stop", _("Hang up"),
			G_CALLBACK(on_phone_dialer_hangup),
			G_CALLBACK(on_phone_dialer_clicked));
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(phone->di_window), vbox);
	gtk_widget_show_all(vbox);
}


/* phone_show_logs */
void phone_show_logs(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(phone->lo_window == NULL)
	{
		phone->lo_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->lo_window), 200,
				300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->lo_window),
				"logviewer");
#endif
		gtk_window_set_title(GTK_WINDOW(phone->lo_window),
				_("Phone logs"));
		g_signal_connect(G_OBJECT(phone->lo_window), "delete-event",
				G_CALLBACK(on_phone_closex), NULL);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Call"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"call-start");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_logs_call), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Message"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"stock_mail-compose");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_logs_write), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_CLEAR);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_logs_clear), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* FIXME make it a notebook with different log categories */
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->lo_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->lo_store));
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(phone->lo_view),
				TRUE);
		g_signal_connect_swapped(G_OBJECT(phone->lo_view),
				"row-activated", G_CALLBACK(
					on_phone_logs_activated), phone);
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->lo_view),
				TRUE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				_("Direction"), renderer, "text",
				PHONE_LOGS_COLUMN_CALL_TYPE_DISPLAY, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->lo_view),
				column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("To/From"),
				renderer, "text",
				PHONE_LOGS_COLUMN_NUMBER, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->lo_view),
				column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Date"),
				renderer, "text",
				PHONE_LOGS_COLUMN_DATE_DISPLAY, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->lo_view),
				column);
		gtk_container_add(GTK_CONTAINER(widget), phone->lo_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->lo_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->lo_window));
	else
		gtk_widget_hide(phone->lo_window);
}


/* phone_show_messages */
static void _show_messages_window(Phone * phone);

void phone_show_messages(Phone * phone, gboolean show, ...)
{
	va_list ap;
	ModemMessageFolder folder;

	if(phone->me_window == NULL)
		_show_messages_window(phone);
	if(show)
	{
		va_start(ap, show);
		folder = va_arg(ap, unsigned int);
		va_end(ap);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(phone->me_view),
				folder);
		gtk_window_present(GTK_WINDOW(phone->me_window));
	}
	else
		gtk_widget_hide(phone->me_window);
}

static void _show_messages_window(Phone * phone)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkWidget * view;
	size_t i;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkWidget * hbox;
	char const * icon;
	char const * name;
	GtkTreeModel * filter;
	GtkTreeModel * sort;

	phone->me_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(phone->me_window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->me_window),
			"stock_mail-send-receive");
#endif
	gtk_window_set_title(GTK_WINDOW(phone->me_window), _("Messages"));
	g_signal_connect(G_OBJECT(phone->me_window), "delete-event", G_CALLBACK(
				on_phone_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	/* toolbar */
	widget = gtk_toolbar_new();
	toolitem = gtk_tool_button_new(NULL, _("Call"));
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem), "call-start");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_phone_messages_call), phone);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new(NULL, _("New message"));
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
			"stock_mail-compose");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_phone_messages_write), phone);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new(NULL, _("Reply"));
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
			"mail-reply-sender");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
			G_CALLBACK(on_phone_messages_reply), phone);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
			G_CALLBACK(on_phone_messages_delete), phone);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	phone->me_view = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(phone->me_view), TRUE);
	for(i = 0; i < 5; i++)
	{
		icon = _phone_message_filters[i].icon;
		name = _phone_message_filters[i].name;
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(
					phone->me_store), NULL);
		gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(
					filter),
				_phone_message_filters[i].filter, phone, NULL);
		sort = gtk_tree_model_sort_new_with_model(filter);
		view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sort));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
		g_signal_connect_swapped(G_OBJECT(view), "row-activated",
				G_CALLBACK(on_phone_messages_activated), phone);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("To/From"),
				renderer, "text",
				PHONE_MESSAGE_COLUMN_NUMBER_DISPLAY, "weight",
				PHONE_MESSAGE_COLUMN_WEIGHT, NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				PHONE_MESSAGE_COLUMN_NUMBER_DISPLAY);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Date"),
				renderer, "text",
				PHONE_MESSAGE_COLUMN_DATE_DISPLAY, "weight",
				PHONE_MESSAGE_COLUMN_WEIGHT, NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				PHONE_MESSAGE_COLUMN_DATE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
		gtk_container_add(GTK_CONTAINER(widget), view);
		hbox = gtk_hbox_new(FALSE, 4);
		gtk_notebook_append_page(GTK_NOTEBOOK(phone->me_view), widget,
				hbox);
		gtk_box_pack_start(GTK_BOX(hbox), gtk_image_new_from_icon_name(
					icon, GTK_ICON_SIZE_MENU), FALSE, TRUE,
				0);
		gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_(name)), FALSE,
				TRUE, 0);
		gtk_widget_show_all(hbox);
	}
	gtk_box_pack_start(GTK_BOX(vbox), phone->me_view, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(phone->me_window), vbox);
	gtk_widget_show_all(vbox);
}


/* phone_show_plugins */
static void _on_plugins_cancel(gpointer data);
static void _on_plugins_activated(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static gboolean _on_plugins_closex(gpointer data);
static void _on_plugins_enabled_toggle(GtkCellRendererToggle * renderer,
		char * path, gpointer data);
static void _on_plugins_ok(gpointer data);

void phone_show_plugins(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkWidget * bbox;

	if(phone->pl_window == NULL)
	{
		phone->pl_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		widget = phone->pl_window;
		gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
		gtk_window_set_default_size(GTK_WINDOW(widget), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(widget), "gnome-settings");
#endif
		gtk_window_set_title(GTK_WINDOW(widget), _("Plug-ins"));
		g_signal_connect(G_OBJECT(widget), "delete-event", G_CALLBACK(
					_on_plugins_closex), phone);
		vbox = gtk_vbox_new(FALSE, 4);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->pl_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->pl_store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->pl_view),
				FALSE);
		g_signal_connect(G_OBJECT(phone->pl_view), "row-activated",
				G_CALLBACK(_on_plugins_activated), phone);
		renderer = gtk_cell_renderer_toggle_new();
		g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(
					_on_plugins_enabled_toggle), phone);
		column = gtk_tree_view_column_new_with_attributes(_("Enabled"),
				renderer, "active",
				PHONE_PLUGINS_COLUMN_ENABLED, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->pl_view),
				column);
		renderer = gtk_cell_renderer_pixbuf_new();
		column = gtk_tree_view_column_new_with_attributes(NULL,
				renderer, "pixbuf", PHONE_PLUGINS_COLUMN_ICON,
				NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->pl_view),
				column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"),
				renderer, "text", PHONE_PLUGINS_COLUMN_NAME,
				NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				PHONE_PLUGINS_COLUMN_NAME);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->pl_view),
				column);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(
					phone->pl_store),
				PHONE_PLUGINS_COLUMN_NAME, GTK_SORT_ASCENDING);
		gtk_container_add(GTK_CONTAINER(widget), phone->pl_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		/* dialog */
		bbox = gtk_hbutton_box_new();
		gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),
				GTK_BUTTONBOX_END);
		gtk_box_set_spacing(GTK_BOX(bbox), 4);
		widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(_on_plugins_cancel), phone);
		gtk_container_add(GTK_CONTAINER(bbox), widget);
		widget = gtk_button_new_from_stock(GTK_STOCK_OK);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(_on_plugins_ok), phone);
		gtk_container_add(GTK_CONTAINER(bbox), widget);
		gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->pl_window), vbox);
		_on_plugins_cancel(phone);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->pl_window));
	else
		gtk_widget_hide(phone->pl_window);
}

static void _on_plugins_cancel(gpointer data)
{
	Phone * phone = data;
	DIR * dir;
	struct dirent * de;
	GtkIconTheme * theme;
	char const ext[] = ".so";
	size_t len;
	GtkTreeIter iter;
	Plugin * p;
	PhonePlugin * pp;
	size_t i;
	GdkPixbuf * pixbuf;

	gtk_widget_hide(phone->pl_window);
	gtk_list_store_clear(phone->pl_store);
	if((dir = opendir(LIBDIR "/" PACKAGE "/plugins")) == NULL)
		return;
	theme = gtk_icon_theme_get_default();
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name)) < sizeof(ext))
			continue;
		if(strcmp(&de->d_name[len - sizeof(ext) + 1], ext) != 0)
			continue;
		de->d_name[len - sizeof(ext) + 1] = '\0';
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, de->d_name);
#endif
		if((p = plugin_new(LIBDIR, PACKAGE, "plugins", de->d_name))
				== NULL)
			continue;
		if((pp = plugin_lookup(p, "plugin")) == NULL)
		{
			plugin_delete(p);
			continue;
		}
		gtk_list_store_append(phone->pl_store, &iter);
		gtk_list_store_set(phone->pl_store, &iter,
				PHONE_PLUGINS_COLUMN_FILENAME, de->d_name,
				PHONE_PLUGINS_COLUMN_NAME, pp->name, -1);
		for(i = 0; i < phone->plugins_cnt; i++)
		{
			if(strcmp(phone->plugins[i].pp->name, pp->name) != 0)
				continue;
			gtk_list_store_set(phone->pl_store, &iter,
					PHONE_PLUGINS_COLUMN_ENABLED, TRUE,
					PHONE_PLUGINS_COLUMN_PLUGIN,
					phone->plugins[i].pp, -1);
			break;
		}
		pixbuf = gtk_icon_theme_load_icon(theme, (pp->icon != NULL)
				? pp->icon : "gnome-settings", 48, 0, NULL);
		gtk_list_store_set(phone->pl_store, &iter,
				PHONE_PLUGINS_COLUMN_ICON, pixbuf, -1);
		plugin_delete(p);
	}
	closedir(dir);
}

static void _on_plugins_activated(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	Phone * phone = data;
	GtkTreeIter iter;
	gboolean active;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(phone->pl_store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(phone->pl_store), &iter,
			PHONE_PLUGINS_COLUMN_ENABLED, &active, -1);
	gtk_list_store_set(phone->pl_store, &iter, PHONE_PLUGINS_COLUMN_ENABLED,
			!active, -1);
}

static gboolean _on_plugins_closex(gpointer data)
{
	Phone * phone = data;

	_on_plugins_cancel(phone);
	return TRUE;
}

static void _on_plugins_enabled_toggle(GtkCellRendererToggle * renderer,
		char * path, gpointer data)
{
	Phone * phone = data;
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(phone->pl_store),
			&iter, path);
	gtk_list_store_set(phone->pl_store, &iter, PHONE_PLUGINS_COLUMN_ENABLED,
			!gtk_cell_renderer_toggle_get_active(renderer), -1);
}

static void _on_plugins_ok(gpointer data)
{
	Phone * phone = data;
	GtkTreeModel * model = GTK_TREE_MODEL(phone->pl_store);
	GtkTreeIter iter;
	gboolean valid;
	gboolean enabled;
	PhonePlugin * plugin;
	gchar * name;
	int res = 0;
	String * value = string_new("");
	String * sep = "";

	gtk_widget_hide(phone->pl_window);
	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, PHONE_PLUGINS_COLUMN_ENABLED,
				&enabled, PHONE_PLUGINS_COLUMN_PLUGIN, &plugin,
				PHONE_PLUGINS_COLUMN_FILENAME, &name, -1);
		if(enabled)
		{
			if(plugin == NULL)
				phone_load(phone, name);
			res |= string_append(&value, sep);
			res |= string_append(&value, name);
			sep = ",";
		}
		else if(plugin != NULL)
			_phone_unload(phone, plugin);
		g_free(name);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() value=\"%s\"\n", __func__, value);
#endif
	if(_phone_config_set_type(phone, NULL, NULL, "plugins", value) == 0)
		_phone_config_save(phone);
	string_delete(value);
	_on_plugins_cancel(phone);
}


/* phone_show_read */
void phone_show_read(Phone * phone, gboolean show, ...)
{
	va_list ap;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkTextBuffer * tbuf;
	char const * name;
	char const * number;
	time_t date;
	char const * content;
	struct tm t;
	char buf[32];

	if(show == FALSE)
	{
		phone->re_index = -1; /* XXX unsigned value */
		if(phone->re_window != NULL)
			gtk_widget_hide(phone->re_window);
		return;
	}
	va_start(ap, show);
	phone->re_index = va_arg(ap, unsigned int);
	name = va_arg(ap, char const *);
	number = va_arg(ap, char const *);
	date = va_arg(ap, time_t);
	content = va_arg(ap, char const *);
	va_end(ap);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u, %s, %s, %u, %s\n", __func__,
			phone->re_index, name, number, date, content);
#endif
	if(phone->re_window == NULL)
	{
		phone->re_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->re_window), 200,
				300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->re_window),
				"stock_mail-open");
#endif
		gtk_window_set_title(GTK_WINDOW(phone->re_window),
				_("Read message"));
		g_signal_connect(G_OBJECT(phone->re_window), "delete-event",
				G_CALLBACK(on_phone_closex), phone->re_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Call"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"call-start");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_read_call), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Reply"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-reply-sender");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_read_reply), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Forward"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-forward");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_read_forward), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_read_delete), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* name */
		phone->re_name = gtk_label_new(NULL);
		gtk_widget_modify_font(phone->re_name, phone->bold);
		gtk_box_pack_start(GTK_BOX(vbox), phone->re_name, FALSE, TRUE,
				0);
		/* number */
		phone->re_number = gtk_label_new(NULL);
		gtk_box_pack_start(GTK_BOX(vbox), phone->re_number, FALSE, TRUE,
				0);
		/* date */
		phone->re_date = gtk_label_new(NULL);
		gtk_box_pack_start(GTK_BOX(vbox), phone->re_date, FALSE, TRUE,
				0);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->re_view = gtk_text_view_new();
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(phone->re_view),
				FALSE);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(phone->re_view),
				FALSE);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(phone->re_view),
				GTK_WRAP_WORD_CHAR);
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->re_view));
		gtk_container_add(GTK_CONTAINER(widget), phone->re_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 2);
		gtk_container_add(GTK_CONTAINER(phone->re_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(name != NULL)
		gtk_label_set_text(GTK_LABEL(phone->re_name), name);
	if(number != NULL)
		gtk_label_set_text(GTK_LABEL(phone->re_number), number);
	localtime_r(&date, &t); /* XXX gmtime_r() or localtime_r() ? */
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &t);
	gtk_label_set_text(GTK_LABEL(phone->re_date), buf);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->re_view));
	if(content != NULL)
		gtk_text_buffer_set_text(tbuf, content, -1);
	gtk_window_present(GTK_WINDOW(phone->re_window));
}


/* phone_show_settings */
void phone_show_settings(Phone * phone, gboolean show)
{
	GtkWidget * widget;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(phone->se_window == NULL)
	{
		phone->se_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->se_window), 200,
				300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->se_window),
				"stock_cell-phone");
#endif
		gtk_window_set_title(GTK_WINDOW(phone->se_window),
				_("Phone settings"));
		g_signal_connect(G_OBJECT(phone->se_window), "delete-event",
				G_CALLBACK(on_phone_closex), phone->se_window);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->se_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->se_store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->se_view),
				FALSE);
		g_signal_connect_swapped(G_OBJECT(phone->se_view),
				"row-activated", G_CALLBACK(
					phone_settings_open_selected), phone);
		/* icon */
		renderer = gtk_cell_renderer_pixbuf_new();
		column = gtk_tree_view_column_new_with_attributes(NULL,
				renderer, "pixbuf", PHONE_SETTINGS_COLUMN_ICON,
				NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->se_view),
				column);
		/* name */
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"),
				renderer, "text", PHONE_SETTINGS_COLUMN_NAME,
				NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				PHONE_SETTINGS_COLUMN_NAME);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->se_view),
				column);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(
					phone->se_store),
				PHONE_SETTINGS_COLUMN_NAME, GTK_SORT_ASCENDING);
		gtk_container_add(GTK_CONTAINER(widget), phone->se_view);
		gtk_container_add(GTK_CONTAINER(phone->se_window), widget);
		gtk_widget_show_all(widget);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->se_window));
	else
		gtk_widget_hide(phone->se_window);
}


/* phone_show_status */
static void _show_status_window(Phone * phone);
static gboolean _status_on_closex(gpointer data);
static void _status_on_logs_view(gpointer data);
static void _status_on_messages_read(gpointer data);
static void _status_on_response(gpointer data);

void phone_show_status(Phone * phone, gboolean show, ...)
{
	va_list ap;
	char buf[10];

	if(phone->st_window == NULL)
		_show_status_window(phone);
	if(show == FALSE)
	{
		phone->st_missed = 0;
		phone->st_messages = 0;
		gtk_widget_hide(phone->st_window);
		return;
	}
	va_start(ap, show);
	phone->st_missed += va_arg(ap, unsigned int);
	phone->st_messages += va_arg(ap, unsigned int);
	va_end(ap);
	if(phone->st_missed == 0 && phone->st_messages == 0)
		return;
	/* missed calls */
	snprintf(buf, sizeof(buf), "%u", phone->st_missed);
	gtk_label_set_text(GTK_LABEL(phone->st_lmissed_count), buf);
	gtk_label_set_text(GTK_LABEL(phone->st_lmissed), (phone->st_missed > 1)
			? _("missed calls") : _("missed call"));
	if(phone->st_missed == 0)
		gtk_widget_hide(phone->st_missed_box);
	else
		gtk_widget_show(phone->st_missed_box);
	/* new messages */
	snprintf(buf, sizeof(buf), "%u", phone->st_messages);
	gtk_label_set_text(GTK_LABEL(phone->st_lmessages_count), buf);
	gtk_label_set_text(GTK_LABEL(phone->st_lmessages),
			(phone->st_messages > 1) ? _("new messages")
			: _("new message"));
	if(phone->st_messages == 0)
		gtk_widget_hide(phone->st_messages_box);
	else
		gtk_widget_show(phone->st_messages_box);
	gtk_window_present(GTK_WINDOW(phone->st_window));
}

static void _show_status_window(Phone * phone)
{
	GtkSizeGroup * group;
	GtkSizeGroup * group2;
	GtkWidget * vbox;
	GtkWidget * widget;

	phone->st_window = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Information"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				phone->st_window),
#endif
			"%s", "");
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->st_window),
			"appointment-missed");
#endif
	gtk_window_set_keep_above(GTK_WINDOW(phone->st_window), TRUE);
	gtk_window_set_title(GTK_WINDOW(phone->st_window), _("Status"));
	g_signal_connect_swapped(G_OBJECT(phone->st_window), "delete-event",
			G_CALLBACK(_status_on_closex), phone);
	g_signal_connect_swapped(G_OBJECT(phone->st_window), "response",
			G_CALLBACK(_status_on_response), phone);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(phone->st_window));
#else
	vbox = GTK_DIALOG(phone->st_window)->vbox;
#endif
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	group2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* missed calls */
	phone->st_missed_box = gtk_hbox_new(FALSE, 4);
	phone->st_lmissed_count = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(phone->st_lmissed_count), 1.0, 0.5);
	gtk_widget_modify_font(phone->st_lmissed_count, phone->bold);
	gtk_size_group_add_widget(group, phone->st_lmissed_count);
	gtk_box_pack_start(GTK_BOX(phone->st_missed_box),
			phone->st_lmissed_count, FALSE, TRUE, 0);
	phone->st_lmissed = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(phone->st_lmissed), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(phone->st_missed_box), phone->st_lmissed,
			TRUE, TRUE, 0);
	widget = _phone_create_button("logviewer", _("_View"));
	gtk_size_group_add_widget(group2, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_status_on_logs_view), phone);
	gtk_box_pack_start(GTK_BOX(phone->st_missed_box), widget, FALSE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox), phone->st_missed_box, FALSE, TRUE, 0);
	/* new messages */
	phone->st_messages_box = gtk_hbox_new(FALSE, 4);
	phone->st_lmessages_count = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(phone->st_lmessages_count), 1.0, 0.5);
	gtk_widget_modify_font(phone->st_lmessages_count, phone->bold);
	gtk_size_group_add_widget(group, phone->st_lmessages_count);
	gtk_box_pack_start(GTK_BOX(phone->st_messages_box),
			phone->st_lmessages_count, FALSE, TRUE, 0);
	phone->st_lmessages = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(phone->st_lmessages), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(phone->st_messages_box), phone->st_lmessages,
			TRUE, TRUE, 0);
	widget = _phone_create_button("stock_inbox", _("_Read"));
	gtk_size_group_add_widget(group2, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_status_on_messages_read), phone);
	gtk_box_pack_start(GTK_BOX(phone->st_messages_box), widget, FALSE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox), phone->st_messages_box, FALSE, TRUE,
			0);
	gtk_widget_show_all(vbox);
}

static gboolean _status_on_closex(gpointer data)
{
	_status_on_response(data);
	return TRUE;
}

static void _status_on_logs_view(gpointer data)
{
	Phone * phone = data;

	phone_show_status(phone, FALSE);
	phone_show_logs(phone, TRUE);
}

static void _status_on_messages_read(gpointer data)
{
	Phone * phone = data;

	phone_show_status(phone, FALSE);
	phone_show_messages(phone, TRUE, MODEM_MESSAGE_FOLDER_INBOX);
}

static void _status_on_response(gpointer data)
{
	Phone * phone = data;

	phone_show_status(phone, FALSE);
}


/* phone_show_system */
static void _on_system_cancel(gpointer data);
static gboolean _on_system_closex(gpointer data);
static void _on_system_ok(gpointer data);

void phone_show_system(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * bbox;
	ModemConfig * config;
	size_t i;
	String * label;

	/* XXX creation of this window is not cached for performance reasons */
	if(show == FALSE)
	{
		if(phone->sy_window != NULL)
			gtk_widget_hide(phone->sy_window);
		return;
	}
	if(phone->sy_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(phone->sy_window));
		return;
	}
	phone->sy_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(phone->sy_window), 4);
	gtk_window_set_default_size(GTK_WINDOW(phone->sy_window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(phone->sy_window),
			"gnome-settings");
#endif
	gtk_window_set_title(GTK_WINDOW(phone->sy_window),
			_("System preferences"));
	g_signal_connect_swapped(G_OBJECT(phone->sy_window), "delete-event",
			G_CALLBACK(_on_system_closex), phone);
	vbox = gtk_vbox_new(FALSE, 4);
	config = modem_get_config(phone->modem);
	for(i = 0; config[i].name != NULL; i++)
	{
		widget = NULL;
		switch(config[i].type)
		{
			case MCT_NONE: /* XXX should not happen */
				break;
			case MCT_BOOLEAN:
				widget = gtk_check_button_new_with_label(
						config[i].title);
				break;
			case MCT_FILENAME:
				label = string_new_append(config[i].title, ":",
						NULL);
				widget = gtk_label_new(label);
				string_delete(label);
				gtk_misc_set_alignment(GTK_MISC(widget), 0.0,
						0.5);
				gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE,
						TRUE, 0);
				widget = gtk_file_chooser_button_new(
						_("Open file..."),
						GTK_FILE_CHOOSER_ACTION_OPEN);
				break;
			case MCT_STRING:
			case MCT_UINT32:
				label = string_new_append(config[i].title, ":",
						NULL);
				widget = gtk_label_new(label);
				string_delete(label);
				gtk_misc_set_alignment(GTK_MISC(widget), 0.0,
						0.5);
				gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE,
						TRUE, 0);
				widget = gtk_entry_new();
				break;
		}
		if(widget == NULL)
			continue;
		g_object_set_data(G_OBJECT(phone->sy_window), config[i].name,
				widget);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_system_cancel), phone);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_system_ok), phone);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(phone->sy_window), vbox);
	gtk_widget_show_all(vbox);
	_on_system_cancel(phone);
	gtk_window_present(GTK_WINDOW(phone->sy_window));
}

static void _on_system_cancel(gpointer data)
{
	Phone * phone = data;
	ModemConfig * config;
	size_t i;
	GtkWidget * widget;
	char const * p;
	gboolean active;
	unsigned int u;
	char buf[16];

	gtk_widget_hide(phone->sy_window);
	config = modem_get_config(phone->modem);
	for(i = 0; config[i].name != NULL; i++)
	{
		if((widget = g_object_get_data(G_OBJECT(phone->sy_window),
					config[i].name)) == NULL)
			continue;
		switch(config[i].type)
		{
			case MCT_NONE: /* XXX should not happen */
				break;
			case MCT_BOOLEAN:
				p = config[i].value;
				active = (p != NULL) ? TRUE : FALSE;
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
							widget), active);
				break;
			case MCT_FILENAME:
				p = (config[i].value != NULL)
					? config[i].value : "";
				gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
							widget), p);
				break;
			case MCT_STRING:
				p = (config[i].value != NULL)
					? config[i].value : "";
				gtk_entry_set_text(GTK_ENTRY(widget), p);
				break;
			case MCT_UINT32: /* FIXME really implement */
				u = (unsigned long)config[i].value;
				snprintf(buf, sizeof(buf), "%u", u);
				gtk_entry_set_text(GTK_ENTRY(widget), buf);
				break;
		}
	}
}

static gboolean _on_system_closex(gpointer data)
{
	Phone * phone = data;

	_on_system_cancel(phone);
	return TRUE;
}

static void _on_system_ok(gpointer data)
{
	Phone * phone = data;
	ModemConfig * config;
	size_t i;
	GtkWidget * widget;
	char const * p;

	gtk_widget_hide(phone->sy_window);
	config = modem_get_config(phone->modem);
	for(i = 0; config[i].name != NULL; i++)
	{
		if((widget = g_object_get_data(G_OBJECT(phone->sy_window),
						config[i].name)) == NULL)
			continue;
		switch(config[i].type)
		{
			case MCT_NONE: /* XXX should not happen */
				break;
			case MCT_BOOLEAN:
				config[i].value = gtk_toggle_button_get_active(
						GTK_TOGGLE_BUTTON(widget))
					? (void*)1 : NULL;
				_phone_config_set_type(phone, "modem", "hayes",
						config[i].name, config[i].value
						? "1" : "0");
				break;
			case MCT_FILENAME:
				p = gtk_file_chooser_get_filename(
						GTK_FILE_CHOOSER(widget));
				/* FIXME memory leak */
				config[i].value = strdup(p);
				_phone_config_set_type(phone, "modem", "hayes",
						config[i].name, p);
				break;
			case MCT_STRING:
				p = gtk_entry_get_text(GTK_ENTRY(widget));
				/* FIXME memory leak */
				config[i].value = strdup(p);
				_phone_config_set_type(phone, "modem", "hayes",
						config[i].name, p);
				break;
			case MCT_UINT32:
				p = gtk_entry_get_text(GTK_ENTRY(widget));
				config[i].value = (void*)strtoul(p, NULL, 10);
				_phone_config_set_type(phone, "modem", "hayes",
						config[i].name, p);
				break;
		}
	}
	_phone_config_save(phone);
	if(modem_stop(phone->modem) != 0)
		error_print(PACKAGE);
	else if(modem_start(phone->modem) != 0)
		error_print(PACKAGE);
}


/* phone_show_write */
void phone_show_write(Phone * phone, gboolean show, ...)
{
	va_list ap;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkTextBuffer * tbuf;
	char const * number;
	char const * content;

	if(show == FALSE)
	{
		if(phone->lo_window != NULL)
			gtk_widget_hide(phone->lo_window);
		return;
	}
	va_start(ap, show);
	number = va_arg(ap, char const *);
	content = va_arg(ap, char const *);
	va_end(ap);
	if(phone->wr_window == NULL)
	{
		phone->wr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->wr_window), 200,
				300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->wr_window),
				"stock_mail-compose");
#endif
		gtk_window_set_title(GTK_WINDOW(phone->wr_window),
				_("Write message"));
		g_signal_connect(G_OBJECT(phone->wr_window), "delete-event",
				G_CALLBACK(on_phone_closex), phone->wr_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Send"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-send");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_write_send), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Attach"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"stock_attach");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_write_attach), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_CUT);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_write_cut), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_COPY);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_write_copy), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PASTE);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_write_paste), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* entry */
		hbox = gtk_hbox_new(FALSE, 0);
		phone->wr_entry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), phone->wr_entry, TRUE, TRUE,
				2);
		widget = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name(
					"stock_addressbook",
					GTK_ICON_SIZE_BUTTON));
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_contacts_show), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
		/* character count */
		hbox = gtk_hbox_new(FALSE, 0);
		phone->wr_count = gtk_label_new(NULL);
		gtk_box_pack_start(GTK_BOX(hbox), phone->wr_count, TRUE, TRUE,
				2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
				GTK_SHADOW_ETCHED_IN);
		phone->wr_view = gtk_text_view_new();
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(phone->wr_view),
				GTK_WRAP_WORD_CHAR);
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
		g_signal_connect_swapped(G_OBJECT(tbuf), "changed", G_CALLBACK(
					on_phone_write_changed), phone);
		gtk_container_add(GTK_CONTAINER(widget), phone->wr_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 2);
		/* attachments */
		phone->wr_awindow = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
		phone->wr_astore = gtk_list_store_new(
				PHONE_ATTACHMENT_COLUMN_COUNT, G_TYPE_STRING,
				G_TYPE_STRING, GDK_TYPE_PIXBUF);
		phone->wr_aview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(
					phone->wr_astore));
		gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(phone->wr_aview),
				PHONE_ATTACHMENT_COLUMN_ICON);
		gtk_icon_view_set_text_column(GTK_ICON_VIEW(phone->wr_aview),
				PHONE_ATTACHMENT_COLUMN_BASENAME);
		gtk_widget_show_all(phone->wr_aview);
		gtk_widget_set_no_show_all(phone->wr_awindow, TRUE);
		gtk_container_add(GTK_CONTAINER(phone->wr_awindow),
				phone->wr_aview);
		gtk_box_pack_start(GTK_BOX(vbox), phone->wr_awindow, TRUE, TRUE,
				0);
		gtk_container_add(GTK_CONTAINER(phone->wr_window), vbox);
		gtk_widget_show_all(vbox);
		phone_write_count_buffer(phone);
	}
	if(number != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->wr_entry), number);
	if(content != NULL)
	{
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
		gtk_text_buffer_set_text(tbuf, content, -1);
	}
	gtk_widget_grab_focus((number == NULL || number[0] == '\0')
			? phone->wr_entry : phone->wr_view);
	gtk_window_present(GTK_WINDOW(phone->wr_window));
}


/* phone_unload */
int phone_unload(Phone * phone, char const * name)
{
	/* FIXME implement */
	return -1;
}


/* phone_unload_all */
void phone_unload_all(Phone * phone)
{
	while(phone->plugins_cnt >= 1)
		_phone_unload(phone, phone->plugins[0].pp);
}


/* write */
/* phone_write_attach_dialog */
void phone_write_attach_dialog(Phone * phone)
{
	GtkWidget * dialog;
	char * filename = NULL;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;
	GtkTreeIter iter;

	dialog = gtk_file_chooser_dialog_new(_("Attach file..."),
			GTK_WINDOW(phone->wr_window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	theme = gtk_icon_theme_get_default();
	pixbuf = gtk_icon_theme_load_icon(theme, GTK_STOCK_FILE, 48, 0, NULL);
	gtk_list_store_append(phone->wr_astore, &iter);
	gtk_list_store_set(phone->wr_astore, &iter,
			PHONE_ATTACHMENT_COLUMN_FILENAME, filename,
			PHONE_ATTACHMENT_COLUMN_BASENAME, basename(filename),
			PHONE_ATTACHMENT_COLUMN_ICON, pixbuf, -1);
	free(filename);
	gtk_widget_show(phone->wr_awindow);
}


/* phone_write_copy */
void phone_write_copy(Phone * phone)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	if(gtk_window_get_focus(GTK_WINDOW(phone->wr_window))
			== phone->wr_entry)
	{
		gtk_editable_copy_clipboard(GTK_EDITABLE(phone->wr_entry));
		return;
	}
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	clipboard = gtk_widget_get_clipboard(phone->wr_view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_copy_clipboard(buffer, clipboard);
}


/* phone_write_count_buffer */
void phone_write_count_buffer(Phone * phone)
{
	GtkTextBuffer * tbuf;
	gint cnt;
	int max = 160;
	gint msg_cnt;
	gint cur_cnt;
	char buf[32];

	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	if((cnt = gtk_text_buffer_get_char_count(tbuf)) < 0)
		return;
	if(cnt <= max)
	{
		cur_cnt = cnt;
		msg_cnt = 1;
	}
	else
	{
		max = 153; /* for the User Data Header */
		msg_cnt = (cnt / max) + 1;
		if((cur_cnt = cnt % max) == 0)
		{
			msg_cnt--;
			if(cnt > 0)
				cur_cnt = max;
		}
	}
	snprintf(buf, sizeof(buf), _("%d message%s, %d/%d characters"),
			msg_cnt, (msg_cnt > 1) ? _("s") : "", cur_cnt, max);
	gtk_label_set_text(GTK_LABEL(phone->wr_count), buf);
}


/* phone_write_cut */
void phone_write_cut(Phone * phone)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	if(gtk_window_get_focus(GTK_WINDOW(phone->wr_window))
			== phone->wr_entry)
	{
		gtk_editable_cut_clipboard(GTK_EDITABLE(phone->wr_entry));
		return;
	}
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	clipboard = gtk_widget_get_clipboard(phone->wr_view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}


/* phone_write_paste */
void phone_write_paste(Phone * phone)
{
	GtkTextBuffer * buffer;
	GtkClipboard * clipboard;

	if(gtk_window_get_focus(GTK_WINDOW(phone->wr_window))
			== phone->wr_entry)
	{
		gtk_editable_paste_clipboard(GTK_EDITABLE(phone->wr_entry));
		return;
	}
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	clipboard = gtk_widget_get_clipboard(phone->wr_view,
			GDK_SELECTION_CLIPBOARD);
	gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}


/* phone_write_send */
void phone_write_send(Phone * phone)
{
	gchar const * number;
	gchar * text;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	size_t length;
	ModemMessageEncoding encoding = MODEM_MESSAGE_ENCODING_UTF8;

	phone_show_write(phone, TRUE, NULL, NULL);
	number = gtk_entry_get_text(GTK_ENTRY(phone->wr_entry));
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	text = gtk_text_buffer_get_text(tbuf, &start, &end, FALSE);
	if(number == NULL || number[0] == '\0' || text == NULL)
		return;
	length = strlen(text);
	phone->wr_progress = _phone_create_progress(phone->wr_window,
			_("Sending message..."));
	_phone_track(phone, PHONE_TRACK_MESSAGE_SENT, TRUE);
	modem_request_type(phone->modem, MODEM_REQUEST_MESSAGE_SEND,
			number, encoding, length, text);
	g_free(text);
}


/* private */
/* phone_call_number */
static int _phone_call_number(Phone * phone, char const * number)
{
	PhoneEvent event;
	GtkTreeIter iter;
	time_t date;
	struct tm t;
	char dd[32];

	if(number == NULL)
		return -1;
	memset(&event, 0, sizeof(event));
	event.type = PHONE_EVENT_TYPE_CALLING;
	event.calling.number = number;
	if(phone_event(phone, &event) != 0)
		return -1;
	modem_request_type(phone->modem, MODEM_REQUEST_CALL,
			MODEM_CALL_TYPE_VOICE, number, 0);
	phone_show_call(phone, TRUE, PHONE_CALL_OUTGOING, "", number);
	/* add a log entry */
	gtk_list_store_append(phone->lo_store, &iter);
	date = time(NULL);
	localtime_r(&date, &t);
	strftime(dd, sizeof(dd), "%d/%m/%Y %H:%M:%S", &t);
	gtk_list_store_set(phone->lo_store, &iter,
			PHONE_LOGS_COLUMN_CALL_TYPE, PHONE_CALL_TYPE_OUTGOING,
			PHONE_LOGS_COLUMN_CALL_TYPE_DISPLAY, _("Outgoing"),
			PHONE_LOGS_COLUMN_NUMBER, number,
			PHONE_LOGS_COLUMN_DATE_DISPLAY, dd, -1);
	return 0;
}


/* phone_config_filename */
static char * _phone_config_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(PHONE_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, PHONE_CONFIG_FILE);
	return filename;
}


/* phone_config_foreach */
static void _phone_config_foreach(Phone * phone, char const * section,
		PhoneConfigForeachCallback callback, void * priv)
{
	config_foreach_section(phone->config, section, callback, priv);
}


/* phone_config_get */
static char const * _phone_config_get(Phone * phone, char const * section,
		char const * variable)
{
	char const * ret;
	String * s;

	if((s = string_new_append("plugin_", section, NULL)) == NULL)
		return NULL;
	ret = config_get(phone->config, s, variable);
	string_delete(s);
	return ret;
}


/* phone_config_save */
static int _phone_config_save(Phone * phone)
{
	int ret = 0;
	char * filename;

	if((filename = _phone_config_filename()) == NULL)
		return -1; /* XXX warn the user */
	if(config_save(phone->config, filename) != 0)
		ret = -phone_error(phone, error_get(), 1);
	free(filename);
	return ret;
}


/* phone_config_set */
static int _phone_config_set(Phone * phone, char const * section,
		char const * variable, char const * value)
{
	int ret;

	ret = _phone_config_set_type(phone, "plugin", section, variable, value);
	if(ret == 0)
		ret = _phone_config_save(phone);
	return ret;
}


/* phone_config_set_type */
static int _phone_config_set_type(Phone * phone, char const * type,
		char const * section, char const * variable, char const * value)
{
	int ret;
	String * s = NULL;

	if(type != NULL && (s = string_new_append(type, (section != NULL)
					? "_" : NULL, section, NULL)) == NULL)
		return -1;
	ret = config_set(phone->config, s, variable, value);
	string_delete(s);
	return ret;
}


/* phone_create_button */
static GtkWidget * _phone_create_button(char const * icon, char const * label)
{
	GtkWidget * ret;

	ret = gtk_button_new_with_mnemonic(label);
	gtk_button_set_image(GTK_BUTTON(ret), gtk_image_new_from_icon_name(icon,
				GTK_ICON_SIZE_BUTTON));
	return ret;
}


/* phone_create_dialpad */
static GtkWidget * _phone_create_dialpad(Phone * phone,
		char const * button1_image, char const * button1_label,
		GCallback button1_callback,
		char const * button2_image, char const * button2_label,
		GCallback button2_callback,
		GCallback button_callback)
{
	static struct
	{
		char character;
		char const * label;
	} numbers[12] = {
		{ '1', "<b>_1</b>\n" },
		{ '2', "<b>_2</b>\n<small>ABC</small>" },
		{ '3', "<b>_3</b>\n<small>DEF</small>" },
		{ '4', "<b>_4</b>\n<small>GHI</small>" },
		{ '5', "<b>_5</b>\n<small>JKL</small>" },
		{ '6', "<b>_6</b>\n<small>MNO</small>" },
		{ '7', "<b>_7</b>\n<small>PQRS</small>" },
		{ '8', "<b>_8</b>\n<small>TUV</small>" },
		{ '9', "<b>_9</b>\n<small>WXYZ</small>" },
		{ '*', "<b>_*</b>\n<small>+</small>" },
		{ '0', "<b>_0</b>\n" },
		{ '#', "<b>_#</b>\n" }
	};
	GtkWidget * table;
	GtkWidget * button;
	GtkWidget * image;
	GtkWidget * label;
	int i;

	table = gtk_table_new(5, 6, TRUE);
	/* call */
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name(button1_image,
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), button1_label);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", button1_callback,
			phone);
	gtk_table_attach(GTK_TABLE(table), button, 0, 3, 0, 1,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name(button2_image,
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), button2_label);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", button2_callback,
			phone);
	gtk_table_attach(GTK_TABLE(table), button, 3, 6, 0, 1,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);
	/* numbers */
	for(i = 0; i < 12; i++)
	{
		button = gtk_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),
				numbers[i].label);
		gtk_container_add(GTK_CONTAINER(button), label);
		g_object_set_data(G_OBJECT(button), "character",
				&numbers[i].character);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					button_callback), phone);
		gtk_table_attach(GTK_TABLE(table), button, (i % 3) * 2,
				((i % 3) + 1) * 2, (i / 3) + 1, (i / 3) + 2,
				GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
				2, 2);
	}
	return table;
}


/* phone_create_progress */
static GtkWidget * _phone_create_progress(GtkWidget * parent, char const * text)
{
	GtkWidget * dialog;
	int flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget * vbox;
	GtkWidget * widget;

	/* dialog */
	dialog = gtk_dialog_new_with_buttons(_("Operation in progress..."),
			GTK_WINDOW(parent), flags, GTK_STOCK_CLOSE,
			GTK_RESPONSE_CLOSE, NULL);
	g_signal_connect(G_OBJECT(dialog), "delete-event", G_CALLBACK(
				on_phone_closex), NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	widget = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* progress bar */
	widget = gtk_progress_bar_new();
	g_object_set_data(G_OBJECT(dialog), "progress", widget);
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(widget));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), " ");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_hide), dialog);
	gtk_widget_show_all(vbox);
	gtk_widget_show(dialog);
	return dialog;
}


/* phone_confirm */
static int _phone_confirm(Phone * phone, GtkWidget * window,
		char const * message)
{
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_YES)
		return 0;
	return 1;
}


/* phone_error */
static int _phone_error(GtkWidget * window, char const * message, int ret)
{
	GtkWidget * dialog;
	GtkWindow * w = (window != NULL) ? GTK_WINDOW(window) : NULL;
	GtkDialogFlags flags = (window != NULL)
		? GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT : 0;

	dialog = gtk_message_dialog_new(w, flags, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* phone_info */
static void _phone_info(Phone * phone, GtkWidget * window, char const * message,
		GCallback callback)
{
	GtkWidget * dialog;
	int flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	if(callback == NULL)
		callback = G_CALLBACK(gtk_widget_destroy);
	dialog = gtk_message_dialog_new((window != NULL) ? GTK_WINDOW(window)
			: NULL, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Information"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Information"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(callback),
			phone);
	gtk_widget_show(dialog);
}


/* phone_messages_filter_all */
static gboolean _phone_messages_filter_all(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data)
{
	ModemMessageFolder folder;

	gtk_tree_model_get(model, iter, PHONE_MESSAGE_COLUMN_FOLDER, &folder,
			-1);
	return TRUE;
}


/* phone_messages_filter_drafts */
static gboolean _phone_messages_filter_drafts(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data)
{
	ModemMessageFolder folder;

	gtk_tree_model_get(model, iter, PHONE_MESSAGE_COLUMN_FOLDER, &folder,
			-1);
	return (folder == MODEM_MESSAGE_FOLDER_DRAFTS) ? TRUE : FALSE;
}


/* phone_messages_filter_inbox */
static gboolean _phone_messages_filter_inbox(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data)
{
	ModemMessageFolder folder;

	gtk_tree_model_get(model, iter, PHONE_MESSAGE_COLUMN_FOLDER, &folder,
			-1);
	return (folder == MODEM_MESSAGE_FOLDER_INBOX) ? TRUE : FALSE;
}


/* phone_messages_filter_outbox */
static gboolean _phone_messages_filter_outbox(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data)
{
	ModemMessageFolder folder;

	gtk_tree_model_get(model, iter, PHONE_MESSAGE_COLUMN_FOLDER, &folder,
			-1);
	return (folder == MODEM_MESSAGE_FOLDER_OUTBOX) ? TRUE : FALSE;
}


/* phone_messages_filter_trash */
static gboolean _phone_messages_filter_trash(GtkTreeModel * model,
		GtkTreeIter * iter, gpointer data)
{
	ModemMessageFolder folder;

	gtk_tree_model_get(model, iter, PHONE_MESSAGE_COLUMN_FOLDER, &folder,
			-1);
	return (folder == MODEM_MESSAGE_FOLDER_TRASH) ? TRUE : FALSE;
}


/* phone_messages_get_iter */
static void _phone_messages_get_iter(Phone * phone, GtkWidget * view,
		GtkTreeIter * iter)
{
	GtkTreeModel * model;
	GtkTreeIter p;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(
				model), &p, iter);
	model = gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(model));
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(
				model), iter, &p);
}


/* phone_messages_get_view */
static GtkWidget * _phone_messages_get_view(Phone * phone)
{
	GtkWidget * ret;
	int n;

	n = gtk_notebook_get_current_page(GTK_NOTEBOOK(phone->me_view));
	if(n < 0)
		return NULL;
	ret = gtk_notebook_get_nth_page(GTK_NOTEBOOK(phone->me_view), n);
	if(ret == NULL)
		return NULL;
	return gtk_bin_get_child(GTK_BIN(ret));
}


/* phone_progress_delete */
static GtkWidget * _phone_progress_delete(GtkWidget * widget)
{
	if(widget != NULL)
		gtk_widget_destroy(widget);
	return NULL;
}


/* phone_progress_pulse */
static void _phone_progress_pulse(GtkWidget * widget)
{
	GtkWidget * progress;

	if((progress = g_object_get_data(G_OBJECT(widget), "progress")) == NULL)
		return;
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
}


/* phone_request */
static int _phone_request(Phone * phone, ModemRequest * request)
{
	return modem_request(phone->modem, request);
}


/* phone_show_contacts_dialog */
static void _on_contacts_dialog_response(GtkWidget * widget, gint response,
		gpointer data);

static void _phone_show_contacts_dialog(Phone * phone, gboolean show,
		int index, char const * name, char const * number)
{
	char buf[256];
	GtkDialogFlags f = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if(phone->co_dialog == NULL)
	{
		group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
		phone->co_dialog = gtk_dialog_new_with_buttons("",
				GTK_WINDOW(phone->co_window), f,
				GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
		g_signal_connect(G_OBJECT(phone->co_dialog), "delete-event",
				G_CALLBACK(on_phone_closex), NULL);
		g_signal_connect(G_OBJECT(phone->co_dialog), "response",
				G_CALLBACK(_on_contacts_dialog_response),
				phone);
#if GTK_CHECK_VERSION(2, 14, 0)
		vbox = gtk_dialog_get_content_area(GTK_DIALOG(
					phone->co_dialog));
#else
		vbox = GTK_DIALOG(phone->co_dialog)->vbox;
#endif
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
		hbox = gtk_hbox_new(FALSE, 4);
		widget = gtk_label_new(_("Name: "));
		gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
		gtk_size_group_add_widget(group, widget);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		phone->co_name = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), phone->co_name, TRUE, TRUE,
				0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
		hbox = gtk_hbox_new(FALSE, 4);
		widget = gtk_label_new(_("Number: "));
		gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
		gtk_size_group_add_widget(group, widget);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		phone->co_number = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), phone->co_number, TRUE, TRUE,
				0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
		gtk_widget_show_all(vbox);
	}
	if(show == FALSE)
	{
		gtk_widget_hide(phone->co_dialog);
		return;
	}
	if(index < 0)
		snprintf(buf, sizeof(buf), "%s", _("New contact"));
	else
		snprintf(buf, sizeof(buf), "%s%s", _("Edit contact: "),
				(name != NULL) ? name : "");
	gtk_window_set_title(GTK_WINDOW(phone->co_dialog), buf);
	phone->co_index = index;
	if(name != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->co_name), name);
	if(number != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->co_number), number);
	gtk_widget_show(phone->co_dialog);
}

static void _on_contacts_dialog_response(GtkWidget * widget, gint response,
		gpointer data)
{
	Phone * phone = data;
	char const * name;
	char const * number;

	if(response != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_hide(widget);
		return;
	}
	name = gtk_entry_get_text(GTK_ENTRY(phone->co_name));
	number = gtk_entry_get_text(GTK_ENTRY(phone->co_number));
	if(strlen(name) == 0)
	{
		_phone_error(widget, _("The name cannot be empty"), 0);
		return;
	}
	else if(strlen(number) == 0)
	{
		_phone_error(widget, _("The number cannot be empty"), 0);
		return;
	}
	gtk_widget_hide(widget);
	if(phone->co_index < 0)
		modem_request_type(phone->modem, MODEM_REQUEST_CONTACT_NEW,
				name, number);
	else
		modem_request_type(phone->modem, MODEM_REQUEST_CONTACT_EDIT,
				phone->co_index, name, number);
}


/* phone_track */
static void _phone_track(Phone * phone, PhoneTrack what, gboolean track)
{
	size_t i;

	phone->tracks[what] = track;
	if(track)
	{
		if(phone->tr_source == 0)
			phone->tr_source = g_timeout_add(2000,
					_phone_timeout_track, phone);
	}
	else if(phone->tr_source != 0)
	{
		for(i = 0; i < PHONE_TRACK_COUNT; i++)
			if(phone->tracks[i] != FALSE)
				return;
		g_source_remove(phone->tr_source);
		phone->tr_source = 0;
	}
}


/* phone_trigger */
static int _phone_trigger(Phone * phone, ModemEventType event)
{
	return modem_trigger(phone->modem, event);
}


/* phone_unload */
static int _phone_unload(Phone * phone, PhonePlugin * plugin)
{
	gboolean valid;
	GtkTreeModel * model = GTK_TREE_MODEL(phone->se_store);
	GtkTreeIter iter;
	size_t i;
	PhonePlugin * pp;

	if(plugin == NULL)
		return phone_error(NULL, strerror(EINVAL), -1);
	/* view */
	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;)
	{
		gtk_tree_model_get(model, &iter, PHONE_SETTINGS_COLUMN_PLUGIN,
				&pp, -1);
		if(pp == plugin)
		{
			gtk_list_store_remove(phone->se_store, &iter);
			break;
		}
		else
			valid = gtk_tree_model_iter_next(model, &iter);
	}
	/* plugins */
	for(i = 0; i < phone->plugins_cnt; i++)
	{
		if(phone->plugins[i].pp != plugin)
			continue;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() plugin %lu\n", __func__,
				(unsigned long)i);
#endif
		if(plugin->destroy != NULL && plugin->destroy(plugin) != 0)
			phone_error(phone, error_get(), 0);
		plugin_delete(phone->plugins[i].p);
		memmove(&phone->plugins[i], &phone->plugins[i + 1],
				sizeof(*phone->plugins)
				* (--phone->plugins_cnt - i));
		/* FIXME could call realloc() to gain some memory */
		return 0;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() no such plugin\n", __func__);
#endif
	return phone_error(NULL, strerror(ENOENT), -1); /* XXX not explicit */
}


/* callbacks */
/* phone_modem_event */
static void _modem_event_authentication(Phone * phone, ModemEvent * event);
static void _modem_event_call(Phone * phone, ModemEvent * event);
static void _modem_event_error(Phone * phone, ModemEvent * event);
static void _modem_event_message(Phone * phone, ModemEvent * event);

static void _phone_modem_event(void * priv, ModemEvent * event)
{
	Phone * phone = priv;
	PhoneEvent pevent;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, event->type);
#endif
	memset(&pevent, 0, sizeof(pevent));
	switch(event->type)
	{
		case MODEM_EVENT_TYPE_ERROR:
			_modem_event_error(phone, event);
			break;
		case MODEM_EVENT_TYPE_AUTHENTICATION:
			_modem_event_authentication(phone, event);
			break;
		case MODEM_EVENT_TYPE_CALL:
			_modem_event_call(phone, event);
			break;
		case MODEM_EVENT_TYPE_CONTACT:
			phone_contacts_set(phone, event->contact.id,
					event->contact.name,
					event->contact.number);
			break;
		case MODEM_EVENT_TYPE_MESSAGE:
			_modem_event_message(phone, event);
			break;
		case MODEM_EVENT_TYPE_MESSAGE_SENT:
			_phone_track(phone, PHONE_TRACK_MESSAGE_SENT, FALSE);
			phone->wr_progress = _phone_progress_delete(
					phone->wr_progress);
			_phone_info(phone, phone->wr_window, _("Message sent"),
					NULL);
			break;
		case MODEM_EVENT_TYPE_MODEL:
			modem_request_type(phone->modem,
					MODEM_REQUEST_CONTACT_LIST);
#ifndef DEBUG
			_phone_track(phone, PHONE_TRACK_MESSAGE_LIST, TRUE);
#endif
			break;
		case MODEM_EVENT_TYPE_STATUS:
			pevent.type = (event->status.online != 0)
				? PHONE_EVENT_TYPE_ONLINE
				: PHONE_EVENT_TYPE_OFFLINE;
			phone_event(phone, &pevent);
			break;
		default:
			break;
	}
	pevent.type = PHONE_EVENT_TYPE_MODEM_EVENT;
	pevent.modem_event.event = event;
	phone_event(phone, &pevent);
}

static void _modem_event_authentication(Phone * phone, ModemEvent * event)
{
	ModemAuthenticationMethod method = event->authentication.method;
	char const * name = event->authentication.name;
	char buf[32];
	GCallback callback = G_CALLBACK(_phone_modem_event_authentication);

	switch(event->authentication.status)
	{
		case MODEM_AUTHENTICATION_STATUS_ERROR:
			phone_code_clear(phone);
			snprintf(buf, sizeof(buf), _("Wrong %s"), name);
			_phone_error(phone->en_window, buf, 0);
			break;
		case MODEM_AUTHENTICATION_STATUS_OK:
			phone_code_clear(phone);
			if(name == NULL)
				break;
			snprintf(buf, sizeof(buf), _("%s is valid"), name);
			_phone_info(phone, phone->en_window, buf, callback);
			break;
		case MODEM_AUTHENTICATION_STATUS_REQUIRED:
			if(event->authentication.method
					== MODEM_AUTHENTICATION_METHOD_PIN)
				phone_show_code(phone, TRUE, method, name);
			break;
	}
}

static void _modem_event_call(Phone * phone, ModemEvent * event)
{
	PhoneCall call;
	ModemCallStatus call_status;
	ModemCallDirection call_direction;

	if(event->call.number == NULL)
		return; /* XXX ignore for now */
	call_status = event->call.status;
	call_direction = event->call.direction;
	if(call_status == MODEM_CALL_STATUS_ACTIVE)
		call = PHONE_CALL_ESTABLISHED;
	else if(call_status == MODEM_CALL_STATUS_RINGING)
		call = (call_direction == MODEM_CALL_DIRECTION_INCOMING)
			? PHONE_CALL_INCOMING : PHONE_CALL_OUTGOING;
	else
		call = PHONE_CALL_TERMINATED;
	phone_show_call(phone, TRUE, call, "", event->call.number);
}

static void _modem_event_error(Phone * phone, ModemEvent * event)
{
	phone_error(phone, event->error.message, 0);
}

static void _modem_event_message(Phone * phone, ModemEvent * event)
{
	ModemMessageEncoding encoding;
	size_t length;
	char * content;
	char const * p;

	encoding = event->message.encoding;
	length = event->message.length;
	switch(encoding)
	{
		case MODEM_MESSAGE_ENCODING_ASCII:
		case MODEM_MESSAGE_ENCODING_UTF8:
			if((content = malloc(length + 1)) == NULL)
				return; /* XXX report error */
			if(length > 0)
				memcpy(content, event->message.content, length);
			content[length] = '\0'; /* just in case */
			phone_messages_set(phone, event->message.id,
					event->message.number,
					event->message.date,
					event->message.folder,
					event->message.status, length, content);
			free(content);
			break;
		case MODEM_MESSAGE_ENCODING_DATA:
		case MODEM_MESSAGE_ENCODING_NONE:
			p = _("Raw data (not shown)");
			length = strlen(p);
			phone_messages_set(phone, event->message.id,
					event->message.number,
					event->message.date,
					event->message.folder,
					event->message.status, length, p);
			break;
	}
	if(event->message.status == MODEM_MESSAGE_STATUS_NEW)
		phone_show_status(phone, TRUE, 0, 1);
}


/* phone_modem_event_authentication */
static void _phone_modem_event_authentication(GtkWidget * widget, gint response,
		gpointer data)
{
	Phone * phone = data;

	phone_show_code(phone, FALSE);
	gtk_widget_destroy(widget);
}


/* phone_timeout_track */
static gboolean _phone_timeout_track(gpointer data)
{
	Phone * phone = data;

	if(phone->tracks[PHONE_TRACK_CODE_ENTERED])
		_phone_progress_pulse(phone->en_progress);
	if(phone->tracks[PHONE_TRACK_MESSAGE_LIST])
	{
		_phone_track(phone, PHONE_TRACK_MESSAGE_LIST, FALSE);
		modem_request_type(phone->modem, MODEM_REQUEST_MESSAGE_LIST);
	}
	if(phone->tracks[PHONE_TRACK_MESSAGE_SENT])
		_phone_progress_pulse(phone->wr_progress);
	return TRUE;
}
