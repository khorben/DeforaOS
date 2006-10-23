/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include "callbacks.h"
#include "common.h"
#include "mailer.h"


/* constants */
struct _menu _menu_file[] =
{
	{ "_New mail", G_CALLBACK(on_file_new_mail), NULL },
	{ "", NULL, NULL },
	{ "_Quit", G_CALLBACK(on_file_quit), GTK_STOCK_QUIT },
	{ NULL, NULL, NULL }
};

struct _menu _menu_edit[] =
{
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES },
	{ NULL, NULL, NULL }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL },
#endif
	{ NULL, NULL, NULL }
};

static struct _menubar _mailer_menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* Mailer */
static GtkWidget * _new_headers(Mailer * mailer);
Mailer * mailer_new(void)
{
	Mailer * mailer;
	GtkWidget * vbox;
	GtkWidget * vbox2;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * hpaned;
	GtkWidget * vpaned;
	GtkWidget * widget;

	if((mailer = malloc(sizeof(*mailer))) == NULL)
		return NULL;
	mailer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mailer->window), 640, 480);
	gtk_window_set_title(GTK_WINDOW(mailer->window), "Mailer");
	g_signal_connect(G_OBJECT(mailer->window), "delete_event", G_CALLBACK(
				on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = common_new_menubar(_mailer_menubar, mailer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_STOP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PRINT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 160);
	/* folders */
	widget = gtk_tree_view_new();
	gtk_paned_add1(GTK_PANED(hpaned), widget);
	vpaned = gtk_vpaned_new();
	gtk_paned_set_position(GTK_PANED(vpaned), 160);
	/* messages list */
	widget = gtk_tree_view_new();
	gtk_paned_add1(GTK_PANED(vpaned), widget);
	/* messages body */
	vbox2 = _new_headers(mailer);
	mailer->view_body = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(mailer->view_body), FALSE);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), mailer->view_body);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	gtk_paned_add2(GTK_PANED(vpaned), vbox2);
	gtk_paned_add2(GTK_PANED(hpaned), vpaned);
	gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
	mailer->statusbar = gtk_statusbar_new();
	mailer->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), mailer->statusbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mailer->window), vbox);
	gtk_widget_show_all(mailer->window);
	gtk_widget_hide(mailer->hdr_vbox);
	return mailer;
}

static GtkWidget * _new_headers(Mailer * mailer)
{
	struct {
		char * hdr;
		GtkWidget ** widget;
	} widgets[] =
	{
		{ " Subject: ", &mailer->hdr_subject },
		{ " From: ", &mailer->hdr_from },
		{ " To: ", &mailer->hdr_to },
		{ " Date: ", &mailer->hdr_date },
		{ NULL, NULL }
	};
	int i;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;

	vbox = gtk_vbox_new(FALSE, 0);
	mailer->hdr_vbox = gtk_vbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	for(i = 0; widgets[i].hdr != NULL; i++)
	{
		hbox = gtk_hbox_new(FALSE, 0);
		widget = gtk_label_new(widgets[i].hdr);
		gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.0);
		gtk_size_group_add_widget(GTK_SIZE_GROUP(group), widget);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		*(widgets[i].widget) = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(hbox), *(widgets[i].widget), TRUE,
				TRUE, 0);
		gtk_box_pack_start(GTK_BOX(mailer->hdr_vbox), hbox, FALSE,
				FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), mailer->hdr_vbox, FALSE, FALSE, 0);
	return vbox;
}


void mailer_delete(Mailer * mailer)
{
	free(mailer);
}


/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(mailer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}
