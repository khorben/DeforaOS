/* compose.c */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "callbacks.h"
#include "common.h"
#include "compose.h"


/* constants */
static struct _menu _menu_file[] =
{
	{ "_Close", G_CALLBACK(on_compose_file_close), GTK_STOCK_CLOSE },
	{ NULL, NULL, NULL }
};

static struct _menubar _compose_menubar[] =
{
	{ "_File", _menu_file },
	{ NULL, NULL }
};


/* compose_new */
Compose * compose_new(Mailer * mailer)
{
	Compose * compose;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * widget;
	GtkWidget * view;

	if((compose = malloc(sizeof(*compose))) == NULL)
	{
		mailer_error(mailer, strerror(errno), 0);
		return NULL;
	}
	compose->mailer = mailer;
	compose->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(compose->window), "Mailer - Compose");
	gtk_window_set_default_size(GTK_WINDOW(compose->window), 400, 400);
	g_signal_connect(G_OBJECT(compose->window), "delete_event", G_CALLBACK(
				on_compose_closex), compose);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = common_new_menubar(_compose_menubar, compose);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_compose_save), compose);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	view = gtk_text_view_new();
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	compose->statusbar = gtk_statusbar_new();
	compose->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), compose->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(compose->window), vbox);
	gtk_widget_show_all(compose->window);
	return compose;
}


/* compose_delete */
void compose_delete(Compose * compose)
{
	gtk_widget_hide(compose->window);
	free(compose);
}
