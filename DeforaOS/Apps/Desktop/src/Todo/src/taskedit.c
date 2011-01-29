/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
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



#include <stdlib.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "taskedit.h"
#define _(string) gettext(string)


/* TaskEdit */
/* private */
/* types */
struct _TaskEdit
{
	Todo * todo;
	Task * task;

	/* widgets */
	GtkWidget * window;
	GtkWidget * title;
	GtkWidget * priority;
	GtkWidget * description;
};


/* public */
/* functions */
/* task_new */
static void _on_taskedit_cancel(gpointer data);
static void _on_taskedit_ok(gpointer data);

TaskEdit * taskedit_new(Todo * todo, Task * task)
{
	TaskEdit * taskedit;
	char buf[80];
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * scrolled;
	GtkWidget * widget;
	GtkWidget * entry;
	GtkWidget * bbox;
	char const * description;

	if((taskedit = malloc(sizeof(*taskedit))) == NULL)
		return NULL;
	taskedit->todo = todo;
	taskedit->task = task;
	taskedit->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", _("Edit task: "), task_get_title(
				task));
	gtk_window_set_default_size(GTK_WINDOW(taskedit->window), 300, 400);
	gtk_window_set_title(GTK_WINDOW(taskedit->window), buf);
	g_signal_connect_swapped(G_OBJECT(taskedit->window), "delete-event",
			G_CALLBACK(_on_taskedit_cancel), taskedit);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	/* title */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Title:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	taskedit->title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(taskedit->title), task_get_title(task));
	gtk_box_pack_start(GTK_BOX(hbox), taskedit->title, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* priority */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Priority:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	taskedit->priority = gtk_combo_box_entry_new_text();
	entry = gtk_bin_get_child(GTK_BIN(taskedit->priority));
	gtk_entry_set_text(GTK_ENTRY(entry), task_get_priority(task));
	gtk_box_pack_start(GTK_BOX(hbox), taskedit->priority, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* description */
	widget = gtk_label_new(_("Description:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	taskedit->description = gtk_text_view_new();
	if((description = task_get_description(task)) != NULL)
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						taskedit->description)),
				description, -1);
	gtk_container_add(GTK_CONTAINER(scrolled), taskedit->description);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_taskedit_cancel), taskedit);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_taskedit_ok), taskedit);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(taskedit->window), 4);
	gtk_container_add(GTK_CONTAINER(taskedit->window), vbox);
	gtk_widget_show_all(taskedit->window);
	return taskedit;
}

static void _on_taskedit_cancel(gpointer data)
{
	TaskEdit * taskedit = data;

	taskedit_delete(taskedit);
}

static void _on_taskedit_ok(gpointer data)
{
	TaskEdit * taskedit = data;
	GtkWidget * entry;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	gchar * description;
	
	task_set_title(taskedit->task, gtk_entry_get_text(GTK_ENTRY(
					taskedit->title)));
	entry = gtk_bin_get_child(GTK_BIN(taskedit->priority));
	task_set_priority(taskedit->task, gtk_entry_get_text(GTK_ENTRY(entry)));
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(taskedit->description));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	description = gtk_text_buffer_get_text(tbuf, &start, &end, FALSE);
	task_set_description(taskedit->task, description);
	g_free(description);
	task_save(taskedit->task);
	todo_task_reload_all(taskedit->todo); /* XXX violent solution */
	_on_taskedit_cancel(taskedit);
}


/* taskedit_delete */
void taskedit_delete(TaskEdit * taskedit)
{
	gtk_widget_destroy(taskedit->window);
	free(taskedit);
}
