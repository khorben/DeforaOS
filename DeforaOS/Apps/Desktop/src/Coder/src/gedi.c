/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Coder */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.";
/* TODO:
 * - rename to "Coder"
 * - add a "backend" type of plug-ins (asm, hexedit, make, project, UWff...)
 * - add a "plug-in" type of plug-ins (time tracker, ...) */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "gedi.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* GEDI */
/* private */
/* types */
struct _GEDI
{
	Config * config;
	Project ** projects;
	size_t projects_cnt;
	Project * cur;

	/* widgets */
	/* toolbar */
	GtkWidget * tb_window;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_editor_command;
	GtkWidget * pr_editor_terminal;

	/* files */
	GtkWidget * fi_window;
	GtkWidget * fi_combo;
	GtkWidget * fi_view;

	/* about */
	GtkWidget * ab_window;
};


/* constants */
#define ICON_NAME	"applications-development"

static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* variables */
/* menubar */
static DesktopMenu _gedi_menu_file[] =
{
	{ N_("_New file..."), G_CALLBACK(on_file_new), GTK_STOCK_NEW,
		GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("_Open file..."), G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ "", NULL , NULL, 0, 0 },
	{ N_("_Preferences..."), G_CALLBACK(on_file_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Exit"), G_CALLBACK(on_file_exit), GTK_STOCK_QUIT,
		GDK_CONTROL_MASK, GDK_KEY_Q },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _gedi_menu_projects[] = /* FIXME will certainly be dynamic */
{
	{ N_("_New project..."), G_CALLBACK(on_project_new), GTK_STOCK_NEW, 0,
		0 },
	{ N_("_Open project..."), G_CALLBACK(on_project_open), GTK_STOCK_OPEN,
		0, 0 },
	{ N_("_Save project"), G_CALLBACK(on_project_save), GTK_STOCK_SAVE,
		GDK_CONTROL_MASK, GDK_KEY_S },
	{ N_("Save project _as..."), G_CALLBACK(on_project_save_as),
		GTK_STOCK_SAVE_AS, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Properties..."), G_CALLBACK(on_project_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _gedi_menu_help[] =
{
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _gedi_menubar[] =
{
	{ N_("_File"),		_gedi_menu_file },
	{ N_("_Projects"),	_gedi_menu_projects },
	{ N_("_Help"),		_gedi_menu_help },
	{ NULL,			NULL }
};

/* toolbar */
static DesktopToolbar _gedi_toolbar[] =
{
	{ N_("Exit"), G_CALLBACK(on_file_exit), GTK_STOCK_QUIT, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* prototypes */
static Project * _gedi_get_current_project(GEDI * gedi);


/* public */
/* functions */
/* gedi_new */
static void _new_config(GEDI * g);

GEDI * gedi_new(void)
{
	GEDI * gedi;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if((gedi = malloc(sizeof(*gedi))) == NULL)
		return NULL;
	gedi->projects = NULL;
	gedi->projects_cnt = 0;
	gedi->cur = NULL;
	_new_config(gedi);
	/* main window */
	group = gtk_accel_group_new();
	gedi->tb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(gedi->tb_window), group);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(gedi->tb_window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(gedi->tb_window), _("Coder"));
	gtk_window_set_resizable(GTK_WINDOW(gedi->tb_window), FALSE);
	g_signal_connect_swapped(G_OBJECT(gedi->tb_window), "delete-event",
			G_CALLBACK(on_closex), gedi);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_gedi_menubar, gedi, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* toolbar */
	widget = desktop_toolbar_create(_gedi_toolbar, gedi, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gedi->tb_window), vbox);
	/* files */
	gedi->fi_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(gedi->fi_window), 150, 200);
	gtk_window_set_title(GTK_WINDOW(gedi->fi_window), _("Files"));
	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 2);
	gedi->fi_combo = gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(vbox), gedi->fi_combo, FALSE, TRUE, 2);
	gedi->fi_view = gtk_tree_view_new();
	gtk_box_pack_start(GTK_BOX(vbox), gedi->fi_view, TRUE, TRUE, 2);
	gtk_container_add(GTK_CONTAINER(gedi->fi_window), hbox);
	/* about */
	gedi->ab_window = NULL;
	gtk_widget_show_all(gedi->tb_window);
	gtk_widget_show_all(gedi->fi_window);
	return gedi;
}

static char * _config_file(void);
static void _new_config(GEDI * gedi)
{
	char * filename;

	if((gedi->config = config_new()) == NULL)
	{
		gedi_error(gedi, strerror(errno), 0);
		return;
	}
	config_load(gedi->config, PREFIX "/etc/" PACKAGE ".conf");
	if((filename = _config_file()) == NULL)
		return;
	config_load(gedi->config, filename);
	free(filename);
}

static char * _config_file(void)
{
	char const conffile[] = ".gedi";
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	len = strlen(homedir) + 1 + strlen(conffile) + 1;
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, conffile);
	return filename;
}


/* gedi_delete */
void gedi_delete(GEDI * gedi)
{
	char * filename;
	size_t i;

	if((filename = _config_file()) != NULL)
	{
		config_save(gedi->config, filename);
		free(filename);
	}
	config_delete(gedi->config);
	for(i = 0; i < gedi->projects_cnt; i++)
		project_delete(gedi->projects[i]);
	free(gedi->projects);
	free(gedi);
}


/* useful */
/* gedi_about */
static gboolean _about_on_closex(gpointer data);

void gedi_about(GEDI * gedi)
{
	if(gedi->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(gedi->ab_window));
		return;
	}
	gedi->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(gedi->ab_window), GTK_WINDOW(
				gedi->tb_window));
	desktop_about_dialog_set_authors(gedi->ab_window, _authors);
	desktop_about_dialog_set_comments(gedi->ab_window,
			_("Integrated Development Environment for the DeforaOS"
			" desktop"));
	desktop_about_dialog_set_copyright(gedi->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(gedi->ab_window, ICON_NAME);
	desktop_about_dialog_set_license(gedi->ab_window, _license);
	desktop_about_dialog_set_name(gedi->ab_window, PACKAGE);
	desktop_about_dialog_set_version(gedi->ab_window, VERSION);
	g_signal_connect_swapped(G_OBJECT(gedi->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), gedi);
	gtk_widget_show(gedi->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	GEDI * gedi = data;

	gtk_widget_hide(gedi->ab_window);
	return TRUE;
}


/* gedi_error */
int gedi_error(GEDI * gedi, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gedi->tb_window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* gedi_file_open */
void gedi_file_open(GEDI * gedi, char const * filename)
{
	/* FIXME really use the MIME sub-system */
	char * argv[] = { NULL, NULL, NULL };
	char const * p;
	GError * error = NULL;

	if((p = config_get(gedi->config, "editor", "command")) == NULL)
		p = "editor"; /* XXX gather defaults in a common place */
	if((argv[0] = strdup(p)) == NULL)
		return; /* XXX report error */
	if(filename != NULL)
		argv[1] = strdup(filename); /* XXX check and report error */
	if(g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
				NULL, &error) != TRUE)
		gedi_error(gedi, error->message, 1);
	free(argv[1]);
	free(argv[0]);
}


/* gedi_project_open */
int gedi_project_open(GEDI * gedi, char const * filename)
{
	Project * project;

	if((project = project_new()) == NULL)
		return -gedi_error(gedi, error_get(), 1);
	if(project_load(project, filename) != 0
			|| gedi_project_open_project(gedi, project) != 0)
	{
		project_delete(project);
		return -gedi_error(gedi, error_get(), 1);
	}
	gedi->cur = project;
	gtk_combo_box_append_text(GTK_COMBO_BOX(gedi->fi_combo),
			project_get_package(project));
	/* FIXME doesn't always select the last project opened */
	gtk_combo_box_set_active(GTK_COMBO_BOX(gedi->fi_combo),
			gtk_combo_box_get_active(GTK_COMBO_BOX(gedi->fi_combo))
			+ 1);
	return 0;
}


/* gedi_project_open_dialog */
void gedi_project_open_dialog(GEDI * gedi)
{
	GtkWidget * dialog;
	GtkFileFilter * filter;
	gchar * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open project..."),
			GTK_WINDOW(gedi->tb_window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Project files"));
	gtk_file_filter_add_pattern(filter, "project.conf");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	gedi_project_open(gedi, filename);
	g_free(filename);
}


/* gedi_project_open_project */
int gedi_project_open_project(GEDI * gedi, Project * project)
{
	Project ** p;

	if(project == NULL)
		return -error_set_code(1, "%s", strerror(EINVAL));;
	if((p = realloc(gedi->projects, sizeof(*p) * (gedi->projects_cnt + 1)))
			== NULL)
		return -error_set_code(1, "%s", strerror(errno));
	gedi->projects = p;
	gedi->projects[gedi->projects_cnt++] = project;
	return 0;
}


/* gedi_project_properties */
void gedi_project_properties(GEDI * gedi)
{
	Project * project;

	if((project = _gedi_get_current_project(gedi)) == NULL)
		return;
	project_properties(project);
}


/* gedi_project_save */
int gedi_project_save(GEDI * gedi)
{
	Project * project;

	if((project = _gedi_get_current_project(gedi)) == NULL)
		return -1;
	if(project_get_pathname(project) == NULL)
		return gedi_project_save_dialog(gedi);
	return project_save(project);
}


/* gedi_project_save_as */
int gedi_project_save_as(GEDI * gedi, char const * filename)
{
	Project * project;

	if((project = _gedi_get_current_project(gedi)) == NULL)
		return -1;
	if(project_set_pathname(project, filename) != 0)
		return -1;
	return project_save(project);
}


/* gedi_project_save_dialog */
int gedi_project_save_dialog(GEDI * gedi)
{
	int ret = -1;
	Project * project;
	GtkWidget * dialog;
	gchar * filename = NULL;

	if((project = _gedi_get_current_project(gedi)) == NULL)
		return -1;
	dialog = gtk_file_chooser_dialog_new(_("Save project as..."), NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	/* FIXME add options? (recursive save) */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename != NULL)
		ret = gedi_project_save_as(gedi, filename);
	g_free(filename);
	return ret;
}


/* gedi_show_preferences */
static void _show_preferences_window(GEDI * gedi);
static void _preferences_set(GEDI * gedi);
static gboolean _on_preferences_closex(gpointer data);
static void _on_preferences_apply(gpointer data);
static void _on_preferences_cancel(gpointer data);
static void _on_preferences_ok(gpointer data);

void gedi_show_preferences(GEDI * gedi, gboolean show)
{
	if(gedi->pr_window == NULL)
		_show_preferences_window(gedi);
	if(show)
		gtk_window_present(GTK_WINDOW(gedi->pr_window));
	else
		gtk_widget_hide(gedi->pr_window);
}

static void _show_preferences_window(GEDI * gedi)
{
	GtkWidget * vbox;
	GtkWidget * nb;
	GtkWidget * nb_vbox;
	GtkWidget * hbox;
	GtkWidget * b_ok;
	GtkWidget * b_apply;
	GtkWidget * b_cancel;

	gedi->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL); /* XXX dialog */
	gtk_container_set_border_width(GTK_CONTAINER(gedi->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(gedi->pr_window), _("Preferences"));
	g_signal_connect_swapped(G_OBJECT(gedi->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), gedi);
	vbox = gtk_vbox_new(FALSE, 4);
	nb = gtk_notebook_new();
	/* notebook page editor */
	nb_vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(nb_vbox), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Editor:")), FALSE,
			TRUE, 0);
	gedi->pr_editor_command = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), gedi->pr_editor_command, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), hbox, FALSE, TRUE, 0);
	gedi->pr_editor_terminal = gtk_check_button_new_with_mnemonic(
			_("Run in a _terminal"));
	gtk_box_pack_start(GTK_BOX(nb_vbox), gedi->pr_editor_terminal, FALSE,
			TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				_("Editor")));
	/* notebook page plug-ins */
	nb_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(nb_vbox), 4);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				_("Plug-ins")));
	gtk_box_pack_start(GTK_BOX(vbox), nb, TRUE, TRUE, 0);
	/* buttons */
	hbox = gtk_hbox_new(TRUE, 4);
	b_ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(b_ok), "clicked", G_CALLBACK(
				_on_preferences_ok), gedi);
	gtk_box_pack_end(GTK_BOX(hbox), b_ok, FALSE, TRUE, 0);
	b_apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect_swapped(G_OBJECT(b_apply), "clicked", G_CALLBACK(
				_on_preferences_apply), gedi);
	gtk_box_pack_end(GTK_BOX(hbox), b_apply, FALSE, TRUE, 0);
	b_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(b_cancel), "clicked", G_CALLBACK(
				_on_preferences_cancel), gedi);
	gtk_box_pack_end(GTK_BOX(hbox), b_cancel, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gedi->pr_window), vbox);
	_preferences_set(gedi);
	gtk_widget_show_all(vbox);
}

static void _preferences_set(GEDI * gedi)
{
	char const * p;

	if((p = config_get(gedi->config, "editor", "command")) == NULL)
		p = "editor";
	gtk_entry_set_text(GTK_ENTRY(gedi->pr_editor_command), p);
	/* FIXME implement the rest */
}

static void _on_preferences_apply(gpointer data)
{
	GEDI * gedi = data;

	config_set(gedi->config, "editor", "command", gtk_entry_get_text(
				GTK_ENTRY(gedi->pr_editor_command)));
	/* FIXME implement the rest */
}


static void _on_preferences_cancel(gpointer data)
{
	GEDI * gedi = data;

	_preferences_set(gedi);
	gtk_widget_hide(gedi->pr_window);
}

static void _on_preferences_ok(gpointer data)
{
	GEDI * gedi = data;

	_on_preferences_apply(gedi);
	gtk_widget_hide(gedi->pr_window);
	/* FIXME actually save preferences */
}

static gboolean _on_preferences_closex(gpointer data)
{
	GEDI * gedi = data;

	_on_preferences_cancel(gedi);
	return TRUE;
}


/* private */
/* gedi_get_current_project */
static Project * _gedi_get_current_project(GEDI * gedi)
{
	if(gedi->cur == NULL)
	{
		/* FIXME should not happen (disable callback action) */
		gedi_error(gedi, _("No project opened"), 1);
		return NULL;
	}
	return gedi->cur;
}
