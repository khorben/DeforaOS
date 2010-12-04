/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <gtk/gtk.h>
#include <System.h>


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
		return NULL; /* XXX report error */
	if((now = time(NULL)) == -1)
	{
		_calendar_delete(calendar);
		return NULL; /* XXX report error */
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
	gtk_calendar_set_detail_height_rows(GTK_CALENDAR(calendar->widget), 4);
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

static void _calendar_on_today(gpointer data)
{
	Calendar * calendar = data;

	gtk_calendar_select_month(GTK_CALENDAR(calendar->widget),
			calendar->today.tm_year, calendar->today.tm_mon);
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u/%u/%u\n", __func__, day, month, year);
#endif
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
	entry = gtk_entry_new();
	gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month,
			&day);
	if((p = _calendar_get_detail(calendar, year, ++month, day)) != NULL)
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
	ret = config_set(calendar->config, NULL, buf, detail);
	if((filename = _config_get_filename()) != NULL)
		ret |= config_save(calendar->config, filename);
	free(filename);
	return ret;
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
