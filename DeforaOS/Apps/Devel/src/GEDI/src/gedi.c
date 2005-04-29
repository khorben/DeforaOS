/* gedi.c */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gedi.h"


/* GEDI */
char const * _about_authors[] =
{
	"Fabien Dombard <fdombard@fpconcept.net>",
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};
char const _about_website[] =
"http://cvs.defora.org/cgi-bin/cvsweb/DeforaOS/Apps/Devel/src/GEDI";
/* callbacks */
static void _on_about_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_exit(GtkWidget * widget, gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data);
static void _on_file_new(GtkWidget * widget, gpointer data);
static void _on_file_open(GtkWidget * widget, gpointer data);
static void _on_file_preferences(GtkWidget * widget, gpointer data);
static void _on_file_preferences_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_project_new(GtkWidget * widget, gpointer data);
static void _on_project_open(GtkWidget * widget, gpointer data);
static void _on_project_save(GtkWidget * widget, gpointer data);
static void _on_project_save_as(GtkWidget * widget, gpointer data);
static void _on_xkill(GtkWidget * widget, GdkEvent * event, gpointer data);
/* menus */
struct _menu {
	char * name;
	GtkSignalFunc callback;
/*	char * item; */
};
struct _menubar {
	char * name;
	struct _menu * menu;
};
struct _menu _menu_file[] = {
	{ "_New file...", G_CALLBACK(_on_file_new) /*, GTK_STOCK_NEW */ },
	{ "_Open file...", G_CALLBACK(_on_file_open) /*, GTK_STOCK_OPEN */ },
	{ "", NULL /*, NULL */ },
	{ "_Preferences...", G_CALLBACK(_on_file_preferences) /*,
		GTK_STOCK_PREFERENCES */ },
	{ "", NULL /*, NULL */ },
	{ "_Exit GEDI", G_CALLBACK(_on_exit) /*, GTK_STOCK_QUIT */ },
	{ NULL, NULL /*, NULL */ }
};
struct _menu _menu_projects[] = { /* FIXME will certainly be dynamic */
	{ "_New project...", G_CALLBACK(_on_project_new) /*, GTK_STOCK_NEW */ },
	{ "_Open project...", G_CALLBACK(_on_project_open) /*, GTK_STOCK_OPEN */ },
	{ "_Save project", G_CALLBACK(_on_project_save) /*, GTK_STOCK_SAVE */ },
	{ "Save project _as...", G_CALLBACK(_on_project_save_as) /*,
		GTK_STOCK_SAVE_AS */ },
	{ NULL, NULL /*, NULL */ }
};
struct _menu _menu_help[] = {
	{ "_About GEDI...", G_CALLBACK(_on_help_about) /*, GTK_STOCK_ABOUT */ },
	{ NULL, NULL /*, NULL */ }
};
struct _menubar _menubar[] = {
	{ "_File", _menu_file },
	{ "_Projects", _menu_projects },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* gedi_new */
static void _new_config(GEDI * g);
static void _new_toolbar(GEDI * g);
GEDI * gedi_new(void)
{
	GEDI * gedi;

	if((gedi = malloc(sizeof(GEDI))) == NULL)
		return NULL;
	_new_config(gedi);
	_new_toolbar(gedi);
	gedi->ab_window = NULL;
	return gedi;
}

static void _new_config(GEDI * g)
{
	char * filename;

	if((g->config = config_new()) == NULL)
	{
		gedi_error(g, "Could not read configuration",
				strerror(errno));
		return;
	}
#ifdef PREFIX
	config_load(g->config, PREFIX "/etc/GEDI.conf");
#endif
	if((filename = _config_file()) == NULL)
		return;
	config_load(g->config, filename);
	free(filename);
}

static char * _config_file(void)
{
	char conffile[] = ".gedi";
	char * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	if((filename = malloc(strlen(homedir) + 1 + strlen(conffile) + 1))
			== NULL)
		return NULL;
	sprintf(filename, "%s/%s", homedir, conffile);
	return filename;
}

static void _new_toolbar_menu(GEDI * g);
static void _new_toolbar(GEDI * g)
{
	g->tb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(g->tb_window), "GEDI");
	gtk_window_set_resizable(GTK_WINDOW(g->tb_window), FALSE);
	g_signal_connect(G_OBJECT(g->tb_window), "delete_event",
			G_CALLBACK(_on_xkill), g);
	g->tb_vbox = gtk_vbox_new(FALSE, 0);
	_new_toolbar_menu(g);
	g->tb_toolbar = gtk_toolbar_new();
	gtk_toolbar_insert_stock(GTK_TOOLBAR(g->tb_toolbar), GTK_STOCK_QUIT,
			"Exit GEDI", NULL, G_CALLBACK(_on_exit), g, 0);
	gtk_box_pack_start(GTK_BOX(g->tb_vbox), g->tb_toolbar, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->tb_window), g->tb_vbox);
	gtk_widget_show_all(g->tb_window);
}

static void _new_toolbar_menu(GEDI * g)
{
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;

	g->tb_menubar = gtk_menu_bar_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			if(_menubar[i].menu[j].name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else
				menuitem = gtk_menu_item_new_with_mnemonic(
						_menubar[i].menu[j].name);
			if(_menubar[i].menu[j].callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(_menubar[i].menu[j].callback), g);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(g->tb_menubar), menubar);
	}
	gtk_box_pack_start(GTK_BOX(g->tb_vbox), g->tb_menubar, TRUE, TRUE, 0);
}


/* gedi_delete */
void gedi_delete(GEDI * gedi)
{
	char * filename;

	if((filename = _config_file()) != NULL)
	{
		config_save(gedi->config, filename);
		free(filename);
	}
	config_delete(gedi->config);
	free(gedi);
}


/* useful */
/* gedi_error */
void gedi_error(GEDI * gedi, char const * title, char const * message)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", title);
	gtk_message_dialog_format_secondary_text(GTK_DIALOG(dialog), "%s",
			message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

}


/* gedi_file_open */
void gedi_file_open(GEDI * gedi, char const * file)
{
	/* FIXME */
	FILE * fp;

	if((fp = fopen(file, "r")) == NULL)
	{
		gedi_error(gedi, "Could not open file", strerror(errno));
		return;
	}
	fclose(fp);
}


/* gedi_project_open */
void gedi_project_open(GEDI * gedi, char const * file)
{
	/* FIXME */
}


/* gedi_project_save */
void gedi_project_save(GEDI * gedi)
{
	/* FIXME */
}


/* gedi_project_save_as */
void gedi_project_save_as(GEDI * gedi, char const * file)
{
	/* FIXME */
}


/* callbacks */
/* _on_about_xkill */
static void _on_about_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
}


/* _on_exit */
static void _on_exit(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME check that everything is properly saved */
	gtk_main_quit();
}


/* _on_help_about */
static void _on_help_about(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	if(g->ab_window == NULL)
	{
		g->ab_window = gtk_about_dialog_new();
		g_signal_connect(G_OBJECT(g->ab_window), "delete_event",
				G_CALLBACK(_on_about_xkill), NULL);
		gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(g->ab_window),
				_about_authors);
		gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(g->ab_window),
				_about_website);
	}
	/* FIXME */
	gtk_widget_show(g->ab_window);
}


/* _on_file_new */
static void _on_file_new(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}


/* _on_file_open */
static void _on_file_open(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new ("Open file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_file_open(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_file_preferences */
static void _on_file_preferences(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	if(g->pr_window == NULL)
	{
		g->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(g->pr_window), "Preferences");
		g_signal_connect(G_OBJECT(g->pr_window), "delete_event",
				G_CALLBACK(_on_file_preferences_xkill), g);
		/* FIXME */
	}
	gtk_widget_show_all(g->pr_window);
}


/* _on_file_preferences_xkill */
static void _on_file_preferences_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
}


/* _on_project_new */
static void _on_project_new(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}


/* _on_project_open */
static void _on_project_open(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new("Open project...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_project_open(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_project_save */
static void _on_project_save(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
	gedi_project_save(g);
}


/* _on_project_save_as */
static void _on_project_save_as(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new("Save project as...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	/* FIXME add options? (recursive save) */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_project_save_as(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_xkill */
static void _on_xkill(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	_on_exit(widget, data);
}
