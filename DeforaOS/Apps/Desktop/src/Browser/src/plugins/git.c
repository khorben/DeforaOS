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
 * - parse git's meta-data */



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


/* Git */
/* private */
/* types */
typedef struct _GitTask GitTask;

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
	GitTask ** tasks;
	size_t tasks_cnt;
} Git;

struct _GitTask
{
	Git * git;

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
static Git * _git_init(BrowserPluginHelper * helper);
static void _git_destroy(Git * git);
static GtkWidget * _git_get_widget(Git * git);
static void _git_refresh(Git * git, GList * selection);

/* useful */
static int _git_add_task(Git * git, char const * title,
		char const * directory, char * argv[]);

/* tasks */
static void _git_task_delete(GitTask * task);
static void _git_task_set_status(GitTask * task, char const * status);
static void _git_task_close(GitTask * task);
static void _git_task_close_channel(GitTask * task,
		GIOChannel * channel);

/* callbacks */
static void _git_on_add(gpointer data);
static void _git_on_blame(gpointer data);
static void _git_on_commit(gpointer data);
static void _git_on_diff(gpointer data);
static void _git_on_log(gpointer data);
static void _git_on_make(gpointer data);
static void _git_on_pull(gpointer data);
/* tasks */
static gboolean _git_task_on_closex(gpointer data);
static void _git_task_on_child_watch(GPid pid, gint status,
		gpointer data);
static gboolean _git_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);


/* public */
/* variables */
/* plug-in */
BrowserPluginDefinition plugin =
{
	N_("Git"),
	"applications-development",
	NULL,
	_git_init,
	_git_destroy,
	_git_get_widget,
	_git_refresh
};


/* private */
/* functions */
/* subversion_init */
static GtkWidget * _init_button(GtkSizeGroup * group, char const * icon,
		char const * label, GCallback callback, gpointer data);

static Git * _git_init(BrowserPluginHelper * helper)
{
	Git * git;
	PangoFontDescription * font;
	GtkSizeGroup * group;
	GtkSizeGroup * bgroup;
	GtkWidget * widget;

	if((git = object_new(sizeof(*git))) == NULL)
		return NULL;
	git->helper = helper;
	git->filename = NULL;
	git->source = 0;
	/* widgets */
	git->widget = gtk_vbox_new(FALSE, 4);
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	bgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* label */
	git->name = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(git->name), PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(git->name), 0.0, 0.0);
	gtk_widget_modify_font(git->name, font);
	gtk_box_pack_start(GTK_BOX(git->widget), git->name, FALSE, TRUE, 0);
	git->status = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(git->status), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(git->status), 0.0, 0.0);
	gtk_box_pack_start(GTK_BOX(git->widget), git->status, FALSE, TRUE, 0);
	/* directory */
	git->directory = gtk_vbox_new(FALSE, 4);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_git_on_diff), git);
	gtk_box_pack_start(GTK_BOX(git->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_git_on_log), git);
	gtk_box_pack_start(GTK_BOX(git->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Pull"),
			G_CALLBACK(_git_on_pull), git);
	gtk_box_pack_start(GTK_BOX(git->directory), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_git_on_commit), git);
	gtk_box_pack_start(GTK_BOX(git->directory), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(git->directory);
	gtk_widget_set_no_show_all(git->directory, TRUE);
	gtk_box_pack_start(GTK_BOX(git->widget), git->directory, FALSE, TRUE,
			0);
	/* file */
	git->file = gtk_vbox_new(FALSE, 4);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Request diff"),
			G_CALLBACK(_git_on_diff), git);
	gtk_box_pack_start(GTK_BOX(git->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("Annotate"),
			G_CALLBACK(_git_on_blame), git);
	gtk_box_pack_start(GTK_BOX(git->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_INDEX, _("View log"),
			G_CALLBACK(_git_on_log), git);
	gtk_box_pack_start(GTK_BOX(git->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_REFRESH, _("Pull"),
			G_CALLBACK(_git_on_pull), git);
	gtk_box_pack_start(GTK_BOX(git->file), widget, FALSE, TRUE, 0);
	widget = _init_button(bgroup, GTK_STOCK_JUMP_TO, _("Commit"),
			G_CALLBACK(_git_on_commit), git);
	gtk_box_pack_start(GTK_BOX(git->file), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(git->file);
	gtk_widget_set_no_show_all(git->file, TRUE);
	gtk_box_pack_start(GTK_BOX(git->widget), git->file, FALSE, TRUE, 0);
	/* additional actions */
	git->add = _init_button(bgroup, GTK_STOCK_ADD, _("Add to repository"),
			G_CALLBACK(_git_on_add), git);
	gtk_box_pack_start(GTK_BOX(git->widget), git->add, FALSE, TRUE, 0);
	git->make = _init_button(bgroup, GTK_STOCK_EXECUTE, _("Run make"),
			G_CALLBACK(_git_on_make), git);
	gtk_box_pack_start(GTK_BOX(git->widget), git->make, FALSE, TRUE, 0);
	gtk_widget_show_all(git->widget);
	pango_font_description_free(font);
	/* tasks */
	git->tasks = NULL;
	git->tasks_cnt = 0;
	return git;
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
static void _git_destroy(Git * git)
{
	size_t i;

	for(i = 0; i < git->tasks_cnt; i++)
		_git_task_delete(git->tasks[i]);
	free(git->tasks);
	if(git->source != 0)
		g_source_remove(git->source);
	object_delete(git);
}


/* subversion_get_widget */
static GtkWidget * _git_get_widget(Git * git)
{
	return git->widget;
}


/* subversion_refresh */
static void _refresh_dir(Git * git);
static void _refresh_make(Git * git, struct stat * st);
static void _refresh_status(Git * git, char const * status);

static void _git_refresh(Git * git, GList * selection)
{
	char * path = (selection != NULL) ? selection->data : NULL;
	struct stat st;
	gchar * p;

	if(git->source != 0)
		g_source_remove(git->source);
	free(git->filename);
	git->filename = NULL;
	if(lstat(path, &st) != 0)
		return;
	if((git->filename = strdup(path)) == NULL)
		return;
	p = g_filename_display_basename(path);
	gtk_label_set_text(GTK_LABEL(git->name), p);
	g_free(p);
	_refresh_status(git, NULL);
	gtk_widget_hide(git->directory);
	gtk_widget_hide(git->file);
	gtk_widget_hide(git->add);
	gtk_widget_hide(git->make);
	if(S_ISDIR(st.st_mode))
		_refresh_dir(git);
	_refresh_make(git, &st);
}

static void _refresh_dir(Git * git)
{
	char const dir[] = ".git";
	size_t len = strlen(git->filename);
	char * p;
	struct stat st;

	/* consider ".git" folders like their parent */
	if((len = strlen(git->filename)) >= 4 && strcmp(&git->filename[len - 4],
				"/.git") == 0)
		git->filename[len - 4] = '\0';
	/* check if it is a Git repository */
	len = strlen(git->filename) + sizeof(dir) + 1;
	if((p = malloc(len)) != NULL)
	{
		snprintf(p, len, "%s/%s", git->filename, dir);
		if(lstat(p, &st) != 0)
		{
			_refresh_status(git, _("Not a Git repository"));
			free(p);
			return;
		}
	}
	free(p);
	gtk_widget_show(git->directory);
}

static void _refresh_make(Git * git, struct stat * st)
{
	gboolean show = FALSE;
	gchar * dirname;
	char const * makefile[] = { "Makefile", "makefile", "GNUmakefile" };
	size_t i;
	gchar * p;

	dirname = S_ISDIR(st->st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	for(i = 0; show == FALSE && i < sizeof(makefile) / sizeof(*makefile);
			i++)
	{
		p = g_strdup_printf("%s/%s", dirname, makefile[i]);
		show = (lstat(p, st) == 0) ? TRUE : FALSE;
		g_free(p);
	}
	g_free(dirname);
	if(show)
		gtk_widget_show(git->make);
}

static void _refresh_status(Git * git, char const * status)
{
	if(status == NULL)
		status = "";
	gtk_label_set_text(GTK_LABEL(git->status), status);
}


/* useful */
/* git_add_task */
static int _git_add_task(Git * git, char const * title,
		char const * directory, char * argv[])
{
	BrowserPluginHelper * helper = git->helper;
	GitTask ** p;
	GitTask * task;
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
	gboolean res;
	GError * error = NULL;
	PangoFontDescription * font;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * widget;

	if((p = realloc(git->tasks, sizeof(*p) * (git->tasks_cnt + 1))) == NULL)
		return -helper->error(helper->browser, strerror(errno), 1);
	git->tasks = p;
	if((task = object_new(sizeof(*task))) == NULL)
		return -helper->error(helper->browser, error_get(), 1);
	task->git = git;
#ifdef DEBUG
	argv[0] = "echo";
#endif
	res = g_spawn_async_with_pipes(directory, argv, NULL, flags, NULL, NULL,
			&task->pid, NULL, &task->o_fd, &task->e_fd, &error);
	if(res != TRUE)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
		object_delete(task);
		return -1;
	}
	git->tasks[git->tasks_cnt++] = task;
	/* widgets */
	font = pango_font_description_new();
	pango_font_description_set_family(font, "monospace");
	task->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(task->window), 600, 400);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(task->window), plugin.icon);
#endif
	snprintf(buf, sizeof(buf), "%s - %s (%s)", _("Git"), title,
			directory);
	gtk_window_set_title(GTK_WINDOW(task->window), buf);
	g_signal_connect_swapped(task->window, "delete-event", G_CALLBACK(
				_git_task_on_closex), task);
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
			_git_task_on_child_watch, task);
	task->o_channel = g_io_channel_unix_new(task->o_fd);
	if((g_io_channel_set_encoding(task->o_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->o_source = g_io_add_watch(task->o_channel, G_IO_IN,
			_git_task_on_io_can_read, task);
	task->e_channel = g_io_channel_unix_new(task->e_fd);
	if((g_io_channel_set_encoding(task->e_channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	task->e_source = g_io_add_watch(task->e_channel, G_IO_IN,
			_git_task_on_io_can_read, task);
	_git_task_set_status(task, _("Running command..."));
	return 0;
}


/* tasks */
/* git_task_delete */
static void _git_task_delete(GitTask * task)
{
	_git_task_close(task);
	if(task->source != 0)
		g_source_remove(task->source);
	task->source = 0;
	gtk_widget_destroy(task->window);
	object_delete(task);
}


/* git_task_set_status */
static void _git_task_set_status(GitTask * task, char const * status)
{
	GtkStatusbar * sb = GTK_STATUSBAR(task->statusbar);

	if(task->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				task->statusbar_id);
	task->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* git_task_close */
static void _git_task_close(GitTask * task)
{
	_git_task_close_channel(task, task->o_channel);
	_git_task_close_channel(task, task->e_channel);
}


/* git_task_close */
static void _git_task_close_channel(GitTask * task, GIOChannel * channel)
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
/* git_on_add */
static gboolean _add_is_binary(char const * type);

static void _git_on_add(gpointer data)
{
	Git * git = data;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "add", "--", NULL, NULL, NULL };
	Mime * mime;
	char const * type;

	if(git->filename == NULL)
		return;
	dirname = g_path_get_dirname(git->filename);
	basename = g_path_get_basename(git->filename);
	argv[3] = basename;
	mime = git->helper->get_mime(git->helper->browser);
	type = mime_type(mime, git->filename);
	if(_add_is_binary(type))
	{
		argv[4] = argv[3];
		argv[3] = argv[2];
		argv[2] = "-kb";
	}
	_git_add_task(git, "git add", dirname, argv);
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


/* subversion_on_blame */
static void _git_on_blame(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "blame", "--", NULL, NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(git->filename);
	argv[3] = basename;
	_git_add_task(git, "git blame", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* git_on_commit */
static void _git_on_commit(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "commit", "--", NULL, NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(git->filename);
	argv[3] = basename;
	_git_add_task(git, "git commit", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* git_on_diff */
static void _git_on_diff(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "diff", "--", NULL, NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(git->filename);
	argv[3] = basename;
	_git_add_task(git, "git diff", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* git_on_log */
static void _git_on_log(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "log", "--", NULL, NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(git->filename);
	argv[3] = basename;
	_git_add_task(git, "git log", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* git_on_make */
static void _git_on_make(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	char * argv[] = { "make", NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	_git_add_task(git, "make", dirname, argv);
	g_free(dirname);
}


/* git_on_pull */
static void _git_on_pull(gpointer data)
{
	Git * git = data;
	struct stat st;
	gchar * dirname;
	gchar * basename;
	char * argv[] = { "git", "pull", "--", NULL, NULL };

	if(git->filename == NULL || lstat(git->filename, &st) != 0)
		return;
	dirname = S_ISDIR(st.st_mode) ? g_strdup(git->filename)
		: g_path_get_dirname(git->filename);
	basename = S_ISDIR(st.st_mode) ? NULL
		: g_path_get_basename(git->filename);
	argv[3] = basename;
	_git_add_task(git, "git pull", dirname, argv);
	g_free(basename);
	g_free(dirname);
}


/* git_task_on_closex */
static gboolean _git_task_on_closex(gpointer data)
{
	GitTask * task = data;

	gtk_widget_hide(task->window);
	_git_task_close(task);
	/* FIXME really implement */
	return TRUE;
}


/* git_task_on_child_watch */
static void _git_task_on_child_watch(GPid pid, gint status,
		gpointer data)
{
	GitTask * task = data;
	char buf[256];

	task->source = 0;
	if(WIFEXITED(status))
	{
		snprintf(buf, sizeof(buf),
				_("Command exited with error code %d"),
				WEXITSTATUS(status));
		_git_task_set_status(task, buf);
	}
	else if(WIFSIGNALED(status))
	{
		snprintf(buf, sizeof(buf), _("Command exited with signal %d"),
				WTERMSIG(status));
		_git_task_set_status(task, buf);
	}
	g_spawn_close_pid(pid);
}


/* git_task_on_io_can_read */
static gboolean _git_task_on_io_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	GitTask * task = data;
	Git * git = task->git;
	BrowserPluginHelper * helper = git->helper;
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
			_git_task_close_channel(task, channel);
			return FALSE;
	}
	return TRUE;
}
