/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Accessories */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <System.h>
#include "../config.h"


/* Calendar */
/* private */
/* types */
typedef struct _Calendar
{
	struct tm today;
	Config * config;

	/* widgets */
	GtkWidget * window;
	GtkWidget * widget;
} Calendar;

typedef struct _CalendarEvent
{
	char * name;
	char * location;
	time_t start;
	time_t end;
	char * description;
} CalendarEvent;


/* constants */
#define CALENDAR_CONFIG_FILE ".calendar"


/* prototypes */
static Calendar * _calendar_new(void);
static void _calendar_delete(Calendar * calendar);
/* accessors */
static char const * _calendar_get_detail(Calendar * calendar, unsigned int year,
		unsigned int month, unsigned int day);
static int _calendar_set_detail(Calendar * calendar, unsigned int year,
		unsigned int month, unsigned int day, char const * detail);
static int _calendar_set_event(Calendar * calendar, CalendarEvent * event);
/* useful */
static int _calendar_error(Calendar * calendar, char const * message, int ret);
static int _calendar_open(Calendar * calendar, char const * filename);

/* events */
static CalendarEvent * _calendar_event_new(void);
static void _calendar_event_delete(CalendarEvent * event);
/* accessors */
static int _calendar_event_set_description(CalendarEvent * event,
		char const * description);
static int _calendar_event_set_end(CalendarEvent * event, time_t end);
static int _calendar_event_set_location(CalendarEvent * event,
		char const * location);
static int _calendar_event_set_name(CalendarEvent * event, char const * name);
static int _calendar_event_set_start(CalendarEvent * event, time_t start);

static char * _config_get_filename(void);


/* functions */
/* calendar_new */
static void _new_config(Calendar * calendar);
static gboolean _calendar_on_closex(gpointer data);
static void _calendar_on_today(gpointer data);
#if GTK_CHECK_VERSION(2, 14, 0)
static void _calendar_on_details(GtkWidget * widget, gpointer data);
static gchar * _calendar_on_detail(GtkWidget * widget, guint year, guint month,
		guint day, gpointer data);
static void _calendar_on_open(gpointer data);
#endif
static void _calendar_on_edit(gpointer data);

static Calendar * _calendar_new(void)
{
	Calendar * calendar;
	time_t now;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;

	if((calendar = malloc(sizeof(*calendar))) == NULL)
	{
		_calendar_error(NULL, NULL, 1);
		return NULL;
	}
	calendar->config = NULL;
	if((now = time(NULL)) == -1)
	{
		_calendar_delete(calendar);
		_calendar_error(NULL, NULL, 1);
		return NULL;
	}
	localtime_r(&now, &calendar->today);
	_new_config(calendar);
	/* window */
	calendar->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(calendar->window), "Calendar");
	g_signal_connect_swapped(G_OBJECT(calendar->window), "delete-event",
			G_CALLBACK(_calendar_on_closex), calendar);
	vbox = gtk_vbox_new(FALSE, 4);
	/* toolbar */
	widget = gtk_toolbar_new();
	toolitem = gtk_tool_button_new(NULL, "Today");
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem), "go-jump");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_calendar_on_today), calendar);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
#if GTK_CHECK_VERSION(2, 14, 0)
	toolitem = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_calendar_on_open), calendar);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_toggle_tool_button_new();
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toolitem), "Details");
	g_signal_connect(G_OBJECT(toolitem), "toggled", G_CALLBACK(
				_calendar_on_details), calendar);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_calendar_on_edit), calendar);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* calendar */
	calendar->widget = gtk_calendar_new();
	gtk_calendar_set_display_options(GTK_CALENDAR(calendar->widget),
			GTK_CALENDAR_SHOW_HEADING
			| GTK_CALENDAR_SHOW_DAY_NAMES
			| GTK_CALENDAR_SHOW_WEEK_NUMBERS);
#if GTK_CHECK_VERSION(2, 14, 0)
	gtk_calendar_set_detail_height_rows(GTK_CALENDAR(calendar->widget), 1);
	gtk_calendar_set_detail_func(GTK_CALENDAR(calendar->widget),
			(GtkCalendarDetailFunc)_calendar_on_detail, calendar,
			NULL);
#endif
	g_signal_connect_swapped(G_OBJECT(calendar->widget),
			"day-selected-double-click", G_CALLBACK(
				_calendar_on_edit), calendar);
	gtk_box_pack_start(GTK_BOX(vbox), calendar->widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(calendar->window), vbox);
	gtk_widget_show_all(calendar->window);
	return calendar;
}

static void _new_config(Calendar * calendar)
{
	char * filename;

	if((calendar->config = config_new()) == NULL)
		return;
	if((filename = _config_get_filename()) != NULL)
		config_load(calendar->config, filename);
	free(filename);
}

static gboolean _calendar_on_closex(gpointer data)
{
	Calendar * calendar = data;

	gtk_widget_hide(calendar->window);
	gtk_main_quit();
	return FALSE;
}

static void _calendar_on_open(gpointer data)
{
	Calendar * calendar = data;
	GtkWidget * dialog;
	gchar * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open file...",
			GTK_WINDOW(calendar->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	_calendar_open(calendar, filename);
	g_free(filename);
}

static void _calendar_on_today(gpointer data)
{
	Calendar * calendar = data;

	gtk_calendar_select_month(GTK_CALENDAR(calendar->widget),
			calendar->today.tm_mon, calendar->today.tm_year + 1900);
	gtk_calendar_select_day(GTK_CALENDAR(calendar->widget),
			calendar->today.tm_mday);
}

#if GTK_CHECK_VERSION(2, 14, 0)
static void _calendar_on_details(GtkWidget * widget, gpointer data)
{
	Calendar * calendar = data;
	gboolean active;
	GtkCalendarDisplayOptions options;

	active = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(
				widget));
	options = gtk_calendar_get_display_options(GTK_CALENDAR(
				calendar->widget));
	if(active)
		options |= GTK_CALENDAR_SHOW_DETAILS;
	else
		options &= ~GTK_CALENDAR_SHOW_DETAILS;
	gtk_calendar_set_display_options(GTK_CALENDAR(calendar->widget),
			options);
}
#endif

#if GTK_CHECK_VERSION(2, 14, 0)
static gchar * _calendar_on_detail(GtkWidget * widget, guint year, guint month,
		guint day, gpointer data)
{
	Calendar * calendar = data;
	char * ret;
	char const * p;
	size_t i;
	size_t cnt;

	if((p = _calendar_get_detail(calendar, year, month + 1, day))
			== NULL)
		return NULL;
	/* XXX we have to escape pango markup */
	for(i = 0, cnt = 0; p[i] != '\0'; i++)
		if(p[i] == '<')
			cnt++;
	if((ret = malloc(i + 1 + cnt * 3)) == NULL)
		return NULL;
	for(i = 0, cnt = 0; p[i] != '\0'; i++)
		if(p[i] != '<')
			ret[cnt++] = p[i];
		else
		{
			ret[cnt++] = '&';
			ret[cnt++] = 'l';
			ret[cnt++] = 't';
			ret[cnt++] = ';';
		}
	ret[cnt] = '\0';
	return ret;
}
#endif

static void _calendar_on_edit(gpointer data)
{
	Calendar * calendar = data;
	guint year;
	guint month;
	guint day;
	char buf[32];
	char const * p;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * entry;
	int res;

	dialog = gtk_dialog_new_with_buttons("Edit detail",
			GTK_WINDOW(calendar->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month,
			&day);
	snprintf(buf, sizeof(buf), "%s%02u/%02u/%u:", "Edit detail for ", day,
			++month, year);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(buf), FALSE, TRUE, 0);
	entry = gtk_entry_new();
	if((p = _calendar_get_detail(calendar, year, month, day)) != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), p);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_OK)
	{
		p = gtk_entry_get_text(GTK_ENTRY(entry));
		_calendar_set_detail(calendar, year, month, day, p);
	}
	gtk_widget_destroy(dialog);
}


/* calendar_delete */
static void _calendar_delete(Calendar * calendar)
{
	if(calendar->config != NULL)
		config_delete(calendar->config);
	free(calendar);
}


/* accessors */
/* calendar_get_detail */
static char const * _calendar_get_detail(Calendar * calendar, unsigned int year,
		unsigned int month, unsigned int day)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%u%02u%02u", year, month, day);
	return config_get(calendar->config, NULL, buf);
}


/* calendar_set_detail */
static int _calendar_set_detail(Calendar * calendar, unsigned int year,
		unsigned int month, unsigned int day, char const * detail)
{
	int ret;
	char buf[16];
	char * filename;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, detail);
#endif
	snprintf(buf, sizeof(buf), "%u%02u%02u", year, month, day);
	if(detail != NULL && detail[0] == '\0')
		detail = NULL; /* unset if empty */
	ret = config_set(calendar->config, NULL, buf, detail);
	if((filename = _config_get_filename()) != NULL)
		ret |= config_save(calendar->config, filename);
	free(filename);
	return ret;
}


/* calendar_set_event */
static int _calendar_set_event(Calendar * calendar, CalendarEvent * event)
{
	time_t start;
	time_t end;
	struct tm t;
	const int oneday = 60 * 60 * 24;

	if((start = event->start) < 0 || (end = event->end) < 0)
		return -1; /* XXX report error */
	memset(&t, 0, sizeof(start));
	/* FIXME check with the timezone */
	if(localtime_r(&start, &t) == NULL)
		return -1; /* XXX report error */
	_calendar_set_detail(calendar, t.tm_year + 1900, t.tm_mon + 1,
			t.tm_mday, event->name);
	for(start += oneday; start < end; start += oneday)
	{
		if(localtime_r(&start, &t) == NULL)
			return -1; /* XXX report error */
		_calendar_set_detail(calendar, t.tm_year + 1900, t.tm_mon + 1,
				t.tm_mday, event->name);
	}
	return 0;
}


/* useful */
/* calendar_error */
static int _calendar_error(Calendar * calendar, char const * message, int ret)
{
	GtkWidget * dialog;
	char const * error;

	if(calendar == NULL)
	{
		fputs(PACKAGE, stderr);
		perror(message);
		return ret;
	}
	error = strerror(errno);
	dialog = gtk_message_dialog_new(GTK_WINDOW(calendar->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s%s%s", message ? message : "", message ? ": " : "",
			error);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* calendar_open */
static int _open_parse(Calendar * calendar, char const * filename, FILE * fp);
static int _open_parse_headers(Calendar * calendar, char const * filename,
		FILE * fp, char * buf, size_t cnt);
static int _open_parse_event(Calendar * calendar, char const * filename,
		FILE * fp, char * buf, size_t cnt);
static int _open_parse_event_callback(CalendarEvent * event, char const * buf,
		size_t cnt);
static int _open_parse_event_description(CalendarEvent * event,
		char const * value);
static int _open_parse_event_dtend(CalendarEvent * event, char const * value);
static int _open_parse_event_dtstart(CalendarEvent * event, char const * value);
static int _open_parse_event_location(CalendarEvent * event,
		char const * value);
static int _open_parse_event_summary(CalendarEvent * event, char const * value);

static int _calendar_open(Calendar * calendar, char const * filename)
{
	int ret;
	FILE * fp;

	if((fp = fopen(filename, "r")) == NULL)
		return -_calendar_error(calendar, filename, 1);
	ret = _open_parse(calendar, filename, fp);
	if(fclose(fp) != 0 && ret == 0)
		ret = -_calendar_error(calendar, filename, 1);
	return ret;
}

static int _open_parse(Calendar * calendar, char const * filename, FILE * fp)
{
	int ret;
	char buf[80];
	char const begin[] = "BEGIN:VCALENDAR\r\n";
	char const event[] = "BEGIN:VEVENT\r\n";
	char const end[] = "END:VCALENDAR\r\n";

	if(fgets(buf, sizeof(buf), fp) == NULL || strcmp(buf, begin) != 0)
	{
		if(ferror(fp))
			return -_calendar_error(calendar, filename, 1);
		/* FIXME report error */
		fprintf(stderr, "%s: %s: %s\n", PACKAGE, filename,
				"Not a valid calendar");
		return -1;
	}
	ret = _open_parse_headers(calendar, filename, fp, buf, sizeof(buf));
	while(ret == 0 && strcmp(buf, event) == 0
			&& (ret = _open_parse_event(calendar, filename, fp, buf,
					sizeof(buf))) == 0)
	{
		if(fgets(buf, sizeof(buf), fp) == NULL)
			break; /* XXX report error */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %d \"%s\"\n", __func__, ret, buf);
#endif
	}
	if(ret == 0 && strcmp(buf, end) == 0)
		return 0;
	return ret;
}

static int _open_parse_headers(Calendar * calendar, char const * filename,
		FILE * fp, char * buf, size_t cnt)
{
	char const begin[] = "BEGIN";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(fgets(buf, cnt, fp) != NULL)
	{
		if(strncmp(buf, "BEGIN:", sizeof(begin) - 1) == 0)
			return 0;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	}
	if(ferror(fp))
		return -_calendar_error(calendar, filename, 1);
	return -1; /* XXX reached end of file */
}

static int _open_parse_event(Calendar * calendar, char const * filename,
		FILE * fp, char * buf, size_t cnt)
{
	int ret = 0;
	CalendarEvent * event;
	char const end[] = "END:VEVENT\r\n";
	size_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((event = _calendar_event_new()) == NULL)
		return -1;
	for(;;)
	{
		if(fgets(buf, cnt, fp) == NULL)
		{
			ret = 1;
			break;
		}
		if(strcmp(buf, end) == 0)
			break;
		len = strlen(buf);
		if(len >= 2 && buf[len - 1] == '\n')
			buf[--len] = '\0';
		if(len >= 1 && buf[len - 1] == '\r')
			buf[--len] = '\0';
		ret |= _open_parse_event_callback(event, buf, cnt);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	}
	if(ret == 0)
		_calendar_set_event(calendar, event);
	_calendar_event_delete(event);
	if(ferror(fp))
		return -_calendar_error(calendar, filename, 1);
	return ret;
}

static int _open_parse_event_callback(CalendarEvent * event, char const * buf,
		size_t cnt)
{
	struct
	{
		char const * property;
		int (*callback)(CalendarEvent * event, char const * value);
	} pc[] =
	{
		{ "DESCRIPTION",_open_parse_event_description	},
		{ "DTEND",	_open_parse_event_dtend		},
		{ "DTSTART",	_open_parse_event_dtstart	},
		{ "LOCATION",	_open_parse_event_location	},
		{ "SUMMARY",	_open_parse_event_summary	},
		{ NULL,		NULL				}
	};
	size_t i;
	size_t len;
	char const date[] = "VALUE=DATE:";

	for(i = 0; pc[i].property != NULL; i++)
	{
		len = strlen(pc[i].property);
		if(strncmp(pc[i].property, buf, len) != 0)
			continue;
		if(buf[len] == ':')
			return pc[i].callback(event, &buf[len + 1]);
		if(buf[len] != ';')
			continue;
		if(strncmp(date, &buf[len + 1], sizeof(date) - 1) == 0)
			return pc[i].callback(event, &buf[len + sizeof(date)]);
	}
	return 0;
}

static int _open_parse_event_description(CalendarEvent * event,
		char const * value)
{
	int ret;
	size_t len;
	char * p;
	size_t i;
	size_t j;

	len = strlen(value);
	if((p = malloc(len + 1)) == NULL)
		return -1; /* XXX report error */
	for(i = 0, j = 0; i < len; i++, j++)
		if((p[j] = value[i]) != '\\')
			continue;
		else if(value[++i] == 'n' || value[i] == 'N')
			p[j] = '\n';
		else
			p[j] = value[i];
	p[j] = '\0';
	ret = _calendar_event_set_description(event, p);
	free(p);
	return ret;
}

static int _open_parse_event_dtend(CalendarEvent * event, char const * value)
{
	struct tm tm;
	time_t t;

	memset(&tm, 0, sizeof(tm));
	if(sscanf(value, "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday)
			!= 3 || tm.tm_year < 1900 || tm.tm_mon == 0)
		return -1;
	tm.tm_year -= 1900;
	tm.tm_mon--;
	t = mktime(&tm);
	return _calendar_event_set_end(event, t);
}

static int _open_parse_event_dtstart(CalendarEvent * event, char const * value)
{
	struct tm tm;
	time_t t;

	memset(&tm, 0, sizeof(tm));
	if(sscanf(value, "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday)
			!= 3 || tm.tm_year < 1900 || tm.tm_mon == 0)
		return -1;
	tm.tm_year -= 1900;
	tm.tm_mon--;
	t = mktime(&tm);
	return _calendar_event_set_start(event, t);
}

static int _open_parse_event_location(CalendarEvent * event,
		char const * value)
{
	return _calendar_event_set_location(event, value);
}

static int _open_parse_event_summary(CalendarEvent * event, char const * value)
{
	return _calendar_event_set_name(event, value);
}


/* calendar_event_new */
static CalendarEvent * _calendar_event_new(void)
{
	CalendarEvent * event;

	if((event = object_new(sizeof(*event))) == NULL)
		return NULL;
	memset(event, 0, sizeof(*event));
	event->start = -1;
	event->end = -1;
	return event;
}


/* calendar_event_delete */
static void _calendar_event_delete(CalendarEvent * event)
{
	free(event->name);
	free(event->location);
	free(event->description);
	object_delete(event);
}


/* accessors */
/* calendar_event_set_description */
static int _calendar_event_set_description(CalendarEvent * event,
		char const * description)
{
	char * p;

	if((p = strdup(description)) == NULL)
		return -1;
	free(event->description);
	event->description = p;
	return 0;
}


/* calendar_event_set_end */
static int _calendar_event_set_end(CalendarEvent * event, time_t end)
{
	event->end = end;
	if(event->start > end)
		event->start = end;
	return 0;
}


/* calendar_event_set_location */
static int _calendar_event_set_location(CalendarEvent * event,
		char const * location)
{
	char * p;

	if((p = strdup(location)) == NULL)
		return -1;
	free(event->location);
	event->location = p;
	return 0;
}


/* calendar_event_set_name */
static int _calendar_event_set_name(CalendarEvent * event, char const * name)
{
	char * p;

	if((p = strdup(name)) == NULL)
		return -1;
	free(event->name);
	event->name = p;
	return 0;
}


/* calendar_event_set_start */
static int _calendar_event_set_start(CalendarEvent * event, time_t start)
{
	event->start = start;
	if(event->end < start)
		event->end = start;
	return 0;
}


/* config_get_filename */
static char * _config_get_filename(void)
{
	char * filename;
	char const * homedir;
	size_t len;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(CALENDAR_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, CALENDAR_CONFIG_FILE);
	return filename;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: calendar\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Calendar * calendar;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	calendar = _calendar_new();
	gtk_main();
	_calendar_delete(calendar);
	return 0;
}
