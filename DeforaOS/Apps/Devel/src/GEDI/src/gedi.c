/* gedi.c */



#include <stdlib.h>
#include "gedi.h"


/* GEDI */
char const * _about_authors[] = { "Fabien Dombard", "Pierre Pronchery", NULL };
char const _about_website[] =
"http://cvs.defora.org/cgi-bin/cvsweb/DeforaOS/Apps/Devel/src/GEDI";
/* callbacks */
static void _on_about_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_exit(GtkWidget * widget, gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data);
static void _on_file_new(GtkWidget * widget, gpointer data);
static void _on_file_open(GtkWidget * widget, gpointer data);
static void _on_project_new(GtkWidget * widget, gpointer data);
static void _on_project_open(GtkWidget * widget, gpointer data);
static void _on_project_save(GtkWidget * widget, gpointer data);
static void _on_project_save_as(GtkWidget * widget, gpointer data);
static void _on_xkill(GtkWidget * widget, GdkEvent * event, gpointer data);
/* menus */
struct _menu {
	char * name;
	GtkSignalFunc callback;
};
struct _menubar {
	char * name;
	struct _menu * menu;
};
struct _menu _menu_file[] = {
	{ "New file...", G_CALLBACK(_on_file_new) },
	{ "Open file...", G_CALLBACK(_on_file_open) },
	{ "Exit GEDI", G_CALLBACK(_on_exit) },
	{ NULL, NULL }
};
struct _menu _menu_projects[] = { /* FIXME will certainly be dynamic */
	{ "New project...", G_CALLBACK(_on_project_new) },
	{ "Open project...", G_CALLBACK(_on_project_open) },
	{ "Save project", G_CALLBACK(_on_project_save) },
	{ "Save project as...", G_CALLBACK(_on_project_save_as) },
	{ NULL, NULL }
};
struct _menu _menu_help[] = {
	{ "About GEDI...", G_CALLBACK(_on_help_about) },
	{ NULL, NULL }
};
struct _menubar _menubar[] = {
	{ "File", _menu_file },
	{ "Projects", _menu_projects },
	{ "Help", _menu_help },
	{ NULL, NULL }
};

/* gedi_new */
static void _new_toolbar(GEDI * g);
GEDI * gedi_new(void)
{
	GEDI * gedi;

	if((gedi = malloc(sizeof(GEDI))) == NULL)
		return NULL;
	gedi->config = config_new();
	_new_toolbar(gedi);
	gedi->ab_window = NULL;
	return gedi;
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
		menubar = gtk_menu_item_new_with_label(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			menuitem = gtk_menu_item_new_with_label(
					_menubar[i].menu[j].name);
			g_signal_connect(G_OBJECT(menuitem), "activate",
					G_CALLBACK(_menubar[i].menu[j].callback),
					g);
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
	config_delete(gedi->config);
	free(gedi);
}


/* callbacks */
static void _on_about_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
}

static void _on_exit(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	gtk_main_quit();
}

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

static void _on_file_new(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_file_open(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_project_new(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_project_open(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_project_save(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_project_save_as(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
}

static void _on_xkill(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	_on_exit(widget, data);
}
