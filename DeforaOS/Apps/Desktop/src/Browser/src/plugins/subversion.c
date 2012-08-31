/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
 * - parse SVN's meta-data */



#include <System.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Subversion */
/* private */
/* types */
typedef struct _SVNTask SVNTask;

typedef struct _BrowserPlugin
{
	BrowserPluginHelper * helper;

	char * filename;

	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * name;
	GtkWidget * status;
	/* directory */
	GtkWidget * directory;
	/* file */
	GtkWidget * file;
	/* additional actions */
	GtkWidget * add;
	GtkWidget * make;

	/* tasks */
	SVNTask ** tasks;
	size_t tasks_cnt;
} SVN;

struct _SVNTask
{
	SVN * svn;

	GPid pid;
	guint source;

	/* stdout */
	gint o_fd;
	GIOChannel * o_channel;
	guint o_source;

	/* stderr */
	gint e_fd;
	GIOChannel * e_channel;
	guint e_source;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * statusbar;
	guint statusbar_id;
};


/* prototypes */
static SVN * _subversion_init(BrowserPluginHelper * helper);
static void _subversion_destroy(SVN * svn);
static GtkWidget * _subversion_get_widget(SVN * svn);
static void _subversion_refresh(SVN * svn, GList * selection);

static int _subversion_add_task(SVN * svn, char const * title,
		char const * directory, char * argv[]);

/* tasks */
static void _subversion_task_delete(SVNTask * task);
static void _subversion_task_set_status(SVNTask * task, char const * status);
static void _subversion_task_close(SVNTask * task);
static void _subversion_task_close_channel(SVNTask * task,
		GIOChannel * channel);

/* callbacks */
static void _subversion_on_add(gpointer data);
static void _subversion_on_commit(gpointer data);
static void _subversion_on_diff(gpointer data);
static void _subversion_on_log(gpointer data);
static void _subversion_on_make(gpointer data);
static void _subversion_on_update(gpointer data);
/* tasks */
static gboolean _subversion_task_on_closex(gpointer data);
static void _subversion_task_on_child_watch(GPid pid, gint status,
		gpointer data);
static gboolean _subversion_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);


/* public */
/* variables */
/* plug-in */
BrowserPluginDefinition plugin =
{
	N_("Subversion"),
	"applications-development",
	NULL,
	_subversion_init,
	_subversion_destroy,
	_subversion_get_widget,
	_subversion_refresh
};


/* private */
/* functions */
/* subversion_init */
static GtkWidget * _init_button(GtkSizeGroup * group, char const * icon,
		char const * label, GCallback callback, gpointer data);

static SVN * _subversion_init(BrowserPluginHelper * helper)
{
	SVN * svn;
	PangoFontDescription * font;
	GtkSizeGroup * group;
	GtkSizeGroup * bgroup;
	GtkWidget * widget;

	if((svn = object_new(sizeof(*svn))) == NULL)
		return NULL;
	svn->helper = helper;
	svn->filename = NULL;
	svn->source = 0;
	/* widgets */
	svn->widget = gtk_vbox_new(FALSE, 4);
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	bgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* label */
	svn->name = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(svn->name), PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(svn->name), 0.0, 0.0);
	gtk_widget_modify_font(svn->name, font);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->name, FALSE, TRUE, 0);
	svn->status = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(svn->status), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(svn->status), 0.0, 0.0);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->status, FALSE, TRUE, 0);
	/* directory */
	svn->directory = gtk_vbox_new(FALSE, 4);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_subversion_on_diff), svn);
	gtk_box_pack_start(GTK_BOX(svn->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_subversion_on_log), svn);
	gtk_box_pack_start(GTK_BOX(svn->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Update"),
			G_CALLBACK(_subversion_on_update), svn);
	gtk_box_pack_start(GTK_BOX(svn->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_subversion_on_commit), svn);
	gtk_box_pack_start(GTK_BOX(svn->directory), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(svn->directory);
	gtk_widget_set_no_show_all(svn->directory, TRUE);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->directory, FALSE, TRUE,
			0);
	/* file */
	svn->file = gtk_vbox_new(FALSE, 4);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_subversion_on_diff), svn);
	gtk_box_pack_start(GTK_BOX(svn->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_subversion_on_log), svn);
	gtk_box_pack_start(GTK_BOX(svn->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Update"),
			G_CALLBACK(_subversion_on_update), svn);
	gtk_box_pack_start(GTK_BOX(svn->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_subversion_on_commit), svn);
	gtk_box_pack_start(GTK_BOX(svn->file), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(svn->file);
	gtk_widget_set_no_show_all(svn->file, TRUE);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->file, FALSE, TRUE, 0);
	/* additional actions */
	svn->add = _init_button(bgroup, GTK_STOCK_ADD, _("Add to repository"),
			G_CALLBACK(_subversion_on_add), svn);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->add, FALSE, TRUE, 0);
	svn->make = _init_button(bgroup, GTK_STOCK_EXECUTE, _("Run make"),
			G_CALLBACK(_subversion_on_make), svn);
	gtk_box_pack_start(GTK_BOX(svn->widget), svn->make, FALSE, TRUE, 0);
	gtk_widget_show_all(svn->widget);
	pango_font_description_free(font);
	/* tasks */
	svn->tasks = NULL;
	svn->tasks_cnt = 0;
	return svn;
}

static GtkWidget * _init_button(GtkSizeGroup * group, char const * icon,
		char const * label, GCallback callback, gpointer data)
{
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * widget;
	char const stock[] = "gtk-";

	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_with_label(label);
	gtk_size_group_add_widget(group, widget);
	if(icon != NULL)
	{
		if(strncmp(icon, stock, sizeof(stock) - 1) == 0)
			image = gtk_image_new_from_stock(icon,
					GTK_ICON_SIZE_BUTTON);
		else
			image = gtk_image_new_from_icon_name(icon,
					GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(widget), image);
	}
	g_signal_connect_swapped(widget, "clicked", callback, data);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}


/* subversion_destroy */
static void _subversion_destroy(SVN * svn)
{
	size_t i;

	for(i = 0; i < svn->tasks_cnt; i++)
		_subversion_task_delete(svn->tasks[i]);
	free(svn->tasks);
	if(svn->source != 0)
		g_source_remove(svn->source);
	object_delete(svn);
}


/* subversion_get_widget */
static GtkWidget * _subversion_get_widget(SVN * svn)
{
	return svn->widget;
}


/* subversion_refresh */
static void _refresh_dir(SVN * svn);
static void _refresh_make(SVN * svn, struct stat * st);
static void _refresh_status(SVN * svn, char const * status);

static void _subversion_refresh(SVN * svn, GList * selection)
{
	char * path = (selection != NULL) ? selection->data : NULL;
	struct stat st;
	gchar * p;

	if(svn->source != 0)
		g_source_remove(svn->source);
	free(svn->filename);
	svn->filename = NULL;
	if(lstat(path, &st) != 0)
		return;
	if((svn->filename = strdup(path)) == NULL)
		return;
	p = g_filename_display_basename(path);
	gtk_label_set_text(GTK_LABEL(svn->name), p);
	g_free(p);
	_refresh_status(svn, NULL);
	gtk_widget_hide(svn->directory);
	gtk_widget_hide(svn->file);
	gtk_widget_hide(svn->add);
	gtk_widget_hide(svn->make);
	if(S_ISDIR(st.st_mode))
		_refresh_dir(svn);
	_refresh_make(svn, &st);
}

static void _refresh_dir(SVN * svn)
{
	char const dir[] = ".svn";
	size_t len = strlen(svn->filename);
	char * p;
	struct stat st;

	/* consider ".svn" folders like their parent */
	if((len = strlen(svn->filename)) >= 4 && strcmp(&svn->filename[len - 4],
				"/.svn") == 0)
		svn->filename[len - 4] = '\0';
	/* check if it is a SVN repository */
	len = strlen(svn->filename) + sizeof(dir) + 1;
	if((p = malloc(len)) != NULL)
	{
		snprintf(p, len, "%s/%s", svn->filename, dir);
		if(lstat(p, &st) != 0)
		{
			_refresh_status(svn, _("Not a Subversion repository"));
			free(p);
			return;
		}
	}
	free(p);
	gtk_widget_show(svn->directory);
}

static void _refresh_make(SVN * svn, struct stat * st)
{
	gboolean show = FALSE;
	gchar * dirname;
	char const * makefile[] = { "Makefile", "makefile", "GNUmakefile" };
	size_t i;
	gchar * p;

	dirname = S_ISDIR(st->st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	for(i = 0; show == FALSE && i < sizeof(makefile) / sizeof(*makefile);
			i++)
	{
		p = g_strdup_printf("%s/%s", dirname, makefile[i]);
		show = (lstat(p, st) == 0) ? TRUE : FALSE;
		g_free(p);
	}
	g_free(dirname);
	if(show)
		gtk_widget_show(svn->make);
}

static void _refresh_status(SVN * svn, char const * status)
{
	if(status == NULL)
		status = "";
	gtk_label_set_text(GTK_LABEL(svn->status), status);
}


/* svn_add_task */
static int _subversion_add_task(SVN * svn, char const * title,
		char const * directory, char * argv[])
{
	BrowserPluginHelper * helper = svn->helper;
	SVNTask ** p;
	SVNTask * task;
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
	gboolean res;
	GError * error = NULL;
	PangoFontDescription * font;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * widget;

	if((p = realloc(svn->tasks, sizeof(*p) * (svn->tasks_cnt + 1))) == NULL)
		return -helper->error(helper->browser, strerror(errno), 1);
	svn->tasks = p;
	if((task = object_new(sizeof(*task))) == NULL)
		return -helper->error(helper->browser, error_get(), 1);
	task->svn = svn;
	res = g_spawn_async_with_pipes(directory, argv, NULL, flags, NULL, NULL,
			&task->pid, NULL, &task->o_fd, &task->e_fd, &error);
	if(res != TRUE)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
		object_delete(task);
		return -1;
	}
	svn->tasks[svn->tasks_cnt++] = task;
	/* widgets */
	font = pango_font_description_new();
	pango_font_description_set_family(font, "monospace");
	task->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(task->window), 600, 400);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(task->window), plugin.icon);
#endif
	snprintf(buf, sizeof(buf), "%s - %s (%s)", _("Subversion"), title,
			directory);
	gtk_window_set_title(GTK_WINDOW(task->window), buf);
	g_signal_connect_swapped(task->window, "delete-event", G_CALLBACK(
				_subversion_task_on_closex), task);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	task->view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(task->view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(task->view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(task->view),
			GTK_WRAP_WORD_CHAR);
	gtk_widget_modify_font(task->view, font);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),
			task->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	task->statusbar = gtk_statusbar_new();
	task->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), task->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(task->window), vbox);
	gtk_widget_show_all(task->window);
	pango_font_description_free(font);
	/* events */
	task->source = g_child_watch_add(task->pid,
			_subversion_task_on_child_watch, task);
	task->o_channel = g_io_channel_unix_new(task->o_fd);
	if((g_io_channel_set_encoding(task->o_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->o_source = g_io_add_watch(task->o_channel, G_IO_IN,
			_subversion_task_on_io_can_read, task);
	task->e_channel = g_io_channel_unix_new(task->e_fd);
	if((g_io_channel_set_encoding(task->e_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->e_source = g_io_add_watch(task->e_channel, G_IO_IN,
			_subversion_task_on_io_can_read, task);
	_subversion_task_set_status(task, _("Running command..."));
	return 0;
}


/* tasks */
/* svn_task_delete */
static void _subversion_task_delete(SVNTask * task)
{
	_subversion_task_close(task);
	if(task->source != 0)
		g_source_remove(task->source);
	task->source = 0;
	gtk_widget_destroy(task->window);
	object_delete(task);
}


/* svn_task_set_status */
static void _subversion_task_set_status(SVNTask * task, char const * status)
{
	GtkStatusbar * sb = GTK_STATUSBAR(task->statusbar);

	if(task->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				task->statusbar_id);
	task->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* svn_task_close */
static void _subversion_task_close(SVNTask * task)
{
	_subversion_task_close_channel(task, task->o_channel);
	_subversion_task_close_channel(task, task->e_channel);
}


/* svn_task_close */
static void _subversion_task_close_channel(SVNTask * task, GIOChannel * channel)
{
	if(channel != NULL && channel == task->o_channel)
	{
		if(task->o_source != 0)
			g_source_remove(task->o_source);
		task->o_source = 0;
		g_io_channel_shutdown(task->o_channel, FALSE, NULL);
		g_io_channel_unref(task->o_channel);
		task->o_channel = NULL;
	}
	if(channel != NULL && task->e_channel != NULL)
	{
		if(task->e_source != 0)
			g_source_remove(task->e_source);
		task->e_source = 0;
		g_io_channel_shutdown(task->e_channel, FALSE, NULL);
		g_io_channel_unref(task->e_channel);
		task->e_channel = NULL;
	}
}


/* callbacks */
/* svn_on_add */
static gboolean _add_is_binary(char const * type);

static void _subversion_on_add(gpointer data)
{
	SVN * svn = data;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "svn", "add", "--", NULL, NULL, NULL };
	Mime * mime;
	char const * type;

	if(svn->filename == NULL)
		return;
	dirname = g_path_get_dirname(svn->filename);
	basename = g_path_get_basename(svn->filename);
	argv[3] = basename;
	mime = svn->helper->get_mime(svn->helper->browser);
	type = mime_type(mime, svn->filename);
	if(_add_is_binary(type))
	{
		argv[4] = argv[3];
		argv[3] = argv[2];
		argv[2] = "-kb";
	}
	_subversion_add_task(svn, "svn add", dirname, argv);
	g_free(basename);
	g_free(dirname);
}

static gboolean _add_is_binary(char const * type)
{
	char const text[] = "text/";
	char const * types[] = { "application/x-perl",
		"application/x-shellscript",
		"application/xml",
		"application/xslt+xml" };
	size_t i;

	if(type == NULL)
		return TRUE;
	if(strncmp(text, type, sizeof(text) - 1) == 0)
		return FALSE;
	for(i = 0; i < sizeof(types) / sizeof(*types); i++)
		if(strcmp(types[i], type) == 0)
			return FALSE;
	return TRUE;
}


/* svn_on_commit */
static void _subversion_on_commit(gpointer data)
{
	SVN * svn = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "svn", "commit", "--", NULL, NULL };

	if(svn->filename == NULL || lstat(svn->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(svn->filename);
	argv[3] = basename;
	_subversion_add_task(svn, "svn commit", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* svn_on_diff */
static void _subversion_on_diff(gpointer data)
{
	SVN * svn = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "svn", "diff", "--", NULL, NULL };

	if(svn->filename == NULL || lstat(svn->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(svn->filename);
	argv[3] = basename;
	_subversion_add_task(svn, "svn diff", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* svn_on_log */
static void _subversion_on_log(gpointer data)
{
	SVN * svn = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "svn", "log", "--", NULL, NULL };

	if(svn->filename == NULL || lstat(svn->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(svn->filename);
	argv[3] = basename;
	_subversion_add_task(svn, "svn log", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* svn_on_make */
static void _subversion_on_make(gpointer data)
{
	SVN * svn = data;
	struct stat st;
	gchar * dirname;
	char * argv[] = { "make", NULL };

	if(svn->filename == NULL || lstat(svn->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	_subversion_add_task(svn, "make", dirname, argv);
	g_free(dirname);
}


/* svn_on_update */
static void _subversion_on_update(gpointer data)
{
	SVN * svn = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "svn", "update", "--", NULL, NULL };

	if(svn->filename == NULL || lstat(svn->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(svn->filename)
		: g_path_get_dirname(svn->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(svn->filename);
	argv[3] = basename;
	_subversion_add_task(svn, "svn update", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* svn_task_on_closex */
static gboolean _subversion_task_on_closex(gpointer data)
{
	SVNTask * task = data;

	gtk_widget_hide(task->window);
	_subversion_task_close(task);
	/* FIXME really implement */
	return TRUE;
}


/* svn_task_on_child_watch */
static void _subversion_task_on_child_watch(GPid pid, gint status,
		gpointer data)
{
	SVNTask * task = data;
	char buf[256];

	task->source = 0;
	if(WIFEXITED(status))
	{
		snprintf(buf, sizeof(buf),
				_("Command exited with error code %d"),
				WEXITSTATUS(status));
		_subversion_task_set_status(task, buf);
	}
	else if(WIFSIGNALED(status))
	{
		snprintf(buf, sizeof(buf), _("Command exited with signal %d"),
				WTERMSIG(status));
		_subversion_task_set_status(task, buf);
	}
	g_spawn_close_pid(pid);
}


/* svn_task_on_io_can_read */
static gboolean _subversion_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	SVNTask * task = data;
	SVN * svn = task->svn;
	BrowserPluginHelper * helper = svn->helper;
	char buf[256];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;

	if(condition != G_IO_IN)
		return FALSE;
	if(channel != task->o_channel && channel != task->e_channel)
		return FALSE;
	status = g_io_channel_read_chars(channel, buf, sizeof(buf), &cnt,
			&error);
	if(cnt > 0)
	{
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(task->view));
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, buf, cnt);
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->browser, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			_subversion_task_close_channel(task, channel);
			return FALSE;
	}
	return TRUE;
}
