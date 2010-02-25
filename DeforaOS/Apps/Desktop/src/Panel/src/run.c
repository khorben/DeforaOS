/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
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
	Config * config;
	gboolean terminal;

	/* widgets */
	GtkWidget * window;
	GtkWidget * entry;

	/* internal */
	pid_t pid;		/* current child */
} Run;


/* constants */
#define PACKAGE		"run"
#define RUN_CONFIG_FILE	".runrc"


/* functions */
/* useful */
static int _run_error(char const * message, int ret);
static char * _run_get_config_filename(void);

/* callbacks */
static gboolean _on_run_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_run_cancel(GtkWidget * widget, gpointer data);
static void _on_run_choose_activate(GtkWidget * widget, gint arg1,
		gpointer data);
static void _on_run_execute(GtkWidget * widget, gpointer data);
static void _on_run_path_activate(GtkWidget * widget, gpointer data);
static void _on_run_terminal_toggle(GtkWidget * widget, gpointer data);

/* run_new */
static GtkWidget * _new_entry(Config * config);

static Run * _run_new(void)
{
	Run * run;
	GtkWindow * window;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;

	if((run = object_new(sizeof(*run))) == NULL)
		return NULL;
	run->config = config_new();
	run->terminal = FALSE;
	run->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = GTK_WINDOW(run->window);
	gtk_window_set_keep_above(window, TRUE);
	gtk_window_set_icon_name(window, GTK_STOCK_EXECUTE);
	gtk_window_set_position(window, GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_resizable(window, FALSE);
	gtk_window_set_skip_pager_hint(window, TRUE);
	gtk_window_set_title(window, "Run program...");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_run_closex), run);
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Command:");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	run->entry = _new_entry(run->config);
	g_signal_connect(G_OBJECT(run->entry), "activate", G_CALLBACK(
				_on_run_path_activate), run);
	gtk_box_pack_start(GTK_BOX(hbox), run->entry, TRUE, TRUE, 4);
	widget = gtk_file_chooser_dialog_new("Run program...", window,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(widget), "response", G_CALLBACK(
				_on_run_choose_activate), run);
	widget = gtk_file_chooser_button_new_with_dialog(widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 4);
	widget = gtk_check_button_new_with_label("Run in a terminal");
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
				_on_run_terminal_toggle), run);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_execute), run);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_cancel), run);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(run->window), vbox);
	gtk_widget_show_all(run->window);
	return run;
}

static GtkWidget * _new_entry(Config * config)
{
	GtkWidget * entry;
	char * p;
	char const * q;
	GtkEntryCompletion * completion;
	GtkListStore * store;
	int i;
	char buf[10];
	GtkTreeIter iter;

	entry = gtk_entry_new();
	if(config == NULL)
		return entry;
	if((p = _run_get_config_filename()) == NULL)
		return entry;
	if(config_load(config, p) != 0)
		error_print(PACKAGE);
	free(p);
	completion = gtk_entry_completion_new();
	gtk_entry_set_completion(GTK_ENTRY(entry), completion);
	g_object_unref(completion);
	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
	g_object_unref(store);
	gtk_entry_completion_set_text_column(completion, 0);
	for(i = 0; i < 100; i++)
	{
		snprintf(buf, sizeof(buf), "%s%d", "command", i);
		if((q = config_get(config, "", buf)) == NULL)
			continue;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, q, -1);
	}
	return entry;
}


/* run_delete */
static void _run_delete(Run * run)
{
	if(run->config != NULL)
		config_delete(run->config);
	object_delete(run);
}


/* useful */
/* run_error */
static int _run_error(char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_widget_show(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* run_get_config_filename */
static char * _run_get_config_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(RUN_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, RUN_CONFIG_FILE);
	return filename;
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
static gboolean _execute_idle(gpointer data);
static void _idle_save_config(Run * run);

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
			return FALSE;
		}
		_idle_save_config(run);
		gtk_main_quit();
	}
	return FALSE;
}

static void _idle_save_config(Run * run)
{
	char const * p;
	int i;
	char buf[10];
	char const * q;
	char * filename;

	if((filename = _run_get_config_filename()) == NULL)
		return;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	config_reset(run->config);
	if(config_load(run->config, filename) != 0)
		/* XXX this risks losing the configuration */
		error_print(PACKAGE);
	p = gtk_entry_get_text(GTK_ENTRY(run->entry));
	for(i = 0; i < 100; i++)
	{
		snprintf(buf, sizeof(buf), "%s%d", "command", i);
		q = config_get(run->config, "", buf);
		if(q == NULL || strcmp(p, q) != 0)
			continue;
		free(filename); /* the command is already known */
		return;
	}
	for(i = 0; i < 100; i++)
	{
		snprintf(buf, sizeof(buf), "%s%d", "command", i);
		q = config_get(run->config, "", buf);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() config_set(config, "", %s, %s)\n",
				__func__, buf, p);
#endif
		config_set(run->config, "", buf, p);
		if((p = q) == NULL)
			break;
	}
	if(config_save(run->config, filename) != 0)
		error_print(PACKAGE);
	free(filename);
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
	Run * run;

	gtk_init(&argc, &argv);
	if((run = _run_new()) == NULL)
		return _run_error(error_get(), 2);
	gtk_main();
	_run_delete(run);
	return 0;
}
