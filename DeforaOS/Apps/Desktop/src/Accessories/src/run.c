/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Accessories */
/* Accessories is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * Accessories is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Accessories; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>


/* run */
/* types */
typedef struct _Run
{
	gboolean terminal;
	GtkWidget * window;
	GtkWidget * entry;
	pid_t pid;		/* current child */
} Run;


/* functions */
/* useful */
static int _run_error(char const * message, int ret);

/* callbacks */
static gboolean _on_run_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_run_cancel(GtkWidget * widget, gpointer data);
static void _on_run_choose_activate(GtkWidget * widget, gint arg1,
		gpointer data);
static void _on_run_execute(GtkWidget * widget, gpointer data);
static void _on_run_path_activate(GtkWidget * widget, gpointer data);
static void _on_run_terminal_toggle(GtkWidget * widget, gpointer data);

/* run */
static void _run(void)
{
	static Run run;
	GtkWindow * window;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;

	run.terminal = FALSE;
	run.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = GTK_WINDOW(run.window);
	gtk_window_set_title(window, "Run program...");
	gtk_window_set_resizable(window, FALSE);
	gtk_window_set_keep_above(window, TRUE);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_run_closex), &run);
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Command: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	run.entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(run.entry), "activate", G_CALLBACK(
				_on_run_path_activate), &run);
	gtk_box_pack_start(GTK_BOX(hbox), run.entry, TRUE, TRUE, 4);
	widget = gtk_file_chooser_dialog_new("Run program...", window,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(widget), "response", G_CALLBACK(
				_on_run_choose_activate), &run);
	widget = gtk_file_chooser_button_new_with_dialog(widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 4);
	widget = gtk_check_button_new_with_label("Run in a terminal");
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
				_on_run_terminal_toggle), &run);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_execute), &run);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_cancel), &run);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(run.window), vbox);
	gtk_widget_show_all(run.window);
}


/* useful */
/* run_error */
static int _run_error(char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_widget_show(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* callbacks */
/* on_run_closex */
static gboolean _on_run_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	Run * run = data;

	gtk_widget_hide(run->window);
	gtk_main_quit();
	return FALSE;
}


/* on_run_cancel */
static void _on_run_cancel(GtkWidget * widget, gpointer data)
{
	Run * run = data;

	gtk_widget_hide(run->window);
	gtk_main_quit();
}


/* on_run_choose_activate */
static void _on_run_choose_activate(GtkWidget * widget, gint arg1,
		gpointer data)
{
	Run * run = data;

	if(arg1 != GTK_RESPONSE_ACCEPT)
		return;
	gtk_entry_set_text(GTK_ENTRY(run->entry), gtk_file_chooser_get_filename(
				GTK_FILE_CHOOSER(widget)));
}


/* on_run_execute */
/* static void _execute_parent(GtkWidget * window, pid_t pid); */
static gboolean _execute_idle(gpointer data);

static void _on_run_execute(GtkWidget * widget, gpointer data)
{
	Run * run = data;
	char const * path;

	path = gtk_entry_get_text(GTK_ENTRY(run->entry));
	if((run->pid = fork()) == -1) /* handle error */
	{
		gtk_widget_hide(run->window);
		_run_error(strerror(errno), 0);
		gtk_widget_show(run->window);
	}
	else if(run->pid != 0) /* parent process */
	{
		gtk_widget_hide(run->window);
		g_idle_add(_execute_idle, run);
	}
	else /* child process */
	{
		if(run->terminal == TRUE)
			execlp("xterm", "xterm", "-e", "sh", "-c", path, NULL);
		else
			execlp("/bin/sh", "run", "-c", path, NULL);
		_exit(127); /* an error occured */
	}
}

static gboolean _execute_idle(gpointer data)
{
	Run * run = data;
	pid_t p;
	int status;

	if((p = waitpid(run->pid, &status, 0)) == -1) /* XXX blocks Gtk+ */
	{
		if(errno == ECHILD) /* should not happen */
		{
			_run_error(strerror(errno), 0);
			gtk_main_quit();
			return FALSE;
		}
		if(errno == EINTR)
			return TRUE;
		_run_error(strerror(errno), 0);
		gtk_widget_show(run->window);
		return FALSE;
	}
	if(WIFEXITED(status))
	{
		if(WEXITSTATUS(status) == 127) /* XXX may mean anything... */
		{
			_run_error("Child exited with error code 127", 0);
			gtk_widget_show(run->window);
		}
		else
			gtk_main_quit();
	}
	return FALSE;
}


/* on_run_path_activate */
static void _on_run_path_activate(GtkWidget * widget, gpointer data)
{
	_on_run_execute(widget, data);
}


/* on_run_terminal_toggle */
static void _on_run_terminal_toggle(GtkWidget * widget, gpointer data)
{
	Run * run = data;

	run->terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	_run();
	gtk_main();
	return 0;
}
