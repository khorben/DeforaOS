/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include <string.h>
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
static GtkWidget * _new_folders_view(void);
static GtkWidget * _new_headers(Mailer * mailer);
static gboolean _new_accounts(gpointer data);
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
	mailer->account = NULL;
	mailer->account_cnt = 0;
	g_idle_add(_new_accounts, mailer);
	/* widgets */
	mailer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mailer->window), 640, 480);
	gtk_window_set_title(GTK_WINDOW(mailer->window), "Mailer");
	g_signal_connect(G_OBJECT(mailer->window), "delete_event", G_CALLBACK(
				on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	/* theme */
	mailer->theme = gtk_icon_theme_new();
	gtk_icon_theme_set_custom_theme(mailer->theme, "gnome");
	/* menubar */
	widget = common_new_menubar(_mailer_menubar, mailer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	widget = gtk_image_new_from_pixbuf(gtk_icon_theme_load_icon(
				mailer->theme, "stock_mail", 32, 0,
				NULL));
	toolitem = gtk_tool_button_new(widget, "New mail");
	g_signal_connect(toolitem, "clicked", G_CALLBACK(on_new_mail), mailer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	widget = gtk_image_new_from_pixbuf(gtk_icon_theme_load_icon(
				mailer->theme, "stock_mail-send-receive", 32, 0,
				NULL));
	toolitem = gtk_tool_button_new(widget, "Send / Receive");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
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
	mailer->view_folders = _new_folders_view();
	gtk_paned_add1(GTK_PANED(hpaned), mailer->view_folders);
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

static GtkWidget * _new_folders_view(void)
{
	GtkWidget * widget;
	GtkTreeStore * model;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn *column;

	model = gtk_tree_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	g_object_unref(model);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(
				widget), -1, NULL, renderer, "pixbuf", 0,
			NULL);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(
				widget), -1, "Accounts", renderer, "text", 1,
			NULL);
	return widget;
}

static GtkWidget * _new_headers(Mailer * mailer)
{
	struct
	{
		char * hdr;
		GtkWidget ** widget;
	} widgets[] =
	{
		{ " Subject: ",	&mailer->hdr_subject	},
		{ " From: ",	&mailer->hdr_from	},
		{ " To: ",	&mailer->hdr_to		},
		{ " Date: ",	&mailer->hdr_date	},
		{ NULL,		NULL			}
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

static gboolean _new_accounts(gpointer data)
{
	Mailer * mailer = data;
	Account ** p;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GdkPixbuf * pixbuf;
	GtkTreeIter child_iter;
	AccountFolder ** f;
	char * icon;

	/* FIXME hard-coded */
	if((p = realloc(mailer->account, sizeof(*p) * (mailer->account_cnt+1)))
			== NULL)
		return mailer_error(mailer, "realloc", FALSE);
	mailer->account = p;
	if((mailer->account[mailer->account_cnt] = account_new("unix",
					"Local folders")) == NULL)
		return FALSE;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->view_folders));
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
	pixbuf = gtk_icon_theme_load_icon(mailer->theme,
			"stock_mail-accounts", 16, 0, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 0, pixbuf, 1,
			"Local folders", -1);
	for(f = account_folders(mailer->account[mailer->account_cnt]);
			*f != NULL; f++)
	{
		gtk_tree_store_append(GTK_TREE_STORE(model), &child_iter,
				&iter);
		if((*f)->type == AF_INBOX)
			icon = "stock_inbox";
		else if((*f)->type == AF_DRAFTS)
			icon = "stock_mail-handling";
		else if((*f)->type == AF_SENT)
			icon = "stock_sent-mail";
		else if((*f)->type == AF_TRASH)
			icon = "stock_trash_full";
		else
			icon = "stock_folder";
		pixbuf = gtk_icon_theme_load_icon(mailer->theme, icon, 16, 0,
				NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &child_iter, 0,
				pixbuf, 1, (*f)->name, -1);
	}
	mailer->account_cnt++;
	return FALSE;
}


void mailer_delete(Mailer * mailer)
{
	int i;

	for(i = 0; i < mailer->account_cnt; i++)
		account_delete(mailer->account[i]);
	free(mailer->account);
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
