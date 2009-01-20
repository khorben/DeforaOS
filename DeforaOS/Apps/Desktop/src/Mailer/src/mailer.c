/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* Mailer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Mailer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "common.h"
#include "mailer.h"


/* constants */
struct _menu _menu_file[] =
{
	{ "_New mail", G_CALLBACK(on_file_new_mail), "stock_mail-compose",
		GDK_N },
	{ "", NULL, NULL, 0 },
	{ "Send / Receive", G_CALLBACK(on_file_send_receive),
		"stock_mail-send-receive", GDK_R },
	{ "", NULL, NULL, 0 },
	{ "_Print", G_CALLBACK(on_file_print), GTK_STOCK_PRINT, GDK_P },
	{ "Print pre_view", G_CALLBACK(on_file_print_preview),
		GTK_STOCK_PRINT_PREVIEW, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Quit", G_CALLBACK(on_file_quit), GTK_STOCK_QUIT, GDK_Q },
	{ NULL, NULL, NULL, 0 }
};

struct _menu _menu_edit[] =
{
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, 0 },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_message[] =
{
	{ "_Reply", G_CALLBACK(on_message_reply), "stock_mail-reply", 0 },
	{ "Reply to _all", G_CALLBACK(on_message_reply_to_all),
		"stock_mail-reply-to-all", 0 },
	{ "_Forward", G_CALLBACK(on_message_forward), "stock_mail-forward", 0 },
	{ "_Delete", G_CALLBACK(on_message_delete), GTK_STOCK_DELETE, 0 },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menubar _mailer_menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Message", _menu_message },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};

static struct _toolbar _mailer_toolbar[] =
{
	{ "New mail", G_CALLBACK(on_file_new_mail), "stock_mail-compose" },
	{ "", NULL, NULL },
	{ "Send / Receive", G_CALLBACK(on_file_send_receive),
		"stock_mail-send-receive" },
	{ "Stop", G_CALLBACK(on_stop), GTK_STOCK_STOP },
	{ "", NULL, NULL },
	{ "Reply", G_CALLBACK(on_reply), "stock_mail-reply" },
	{ "Reply to all", G_CALLBACK(on_reply_to_all),
		"stock_mail-reply-to-all" },
	{ "Forward", G_CALLBACK(on_forward), "stock_mail-forward" },
	{ "Delete", G_CALLBACK(on_delete), GTK_STOCK_DELETE },
	{ "Print", G_CALLBACK(on_print), GTK_STOCK_PRINT },
	{ NULL, NULL, NULL }
};


/* Mailer */
/* private */
/* prototypes */
static int _mailer_config_load_account(Mailer * mailer, char const * name);


/* functions */
static int _mailer_config_load_account(Mailer * mailer, char const * name)
{
	Account * account;
	char const * type;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: mailer_config_load_account(\"%s\")\n", name);
#endif
	if((type = config_get(mailer->config, name, "type")) == NULL)
		return 1;
	if((account = account_new(type, name)) == NULL)
		return 1;
	account_config_load(account, mailer->config);
	mailer_account_add(mailer, account);
	return 0;
}


/* public */
/* functions */
/* mailer_new */
static int _new_plugins(Mailer * mailer);
static GtkWidget * _new_folders_view(Mailer * mailer);
static GtkWidget * _new_headers_view(Mailer * mailer);
static GtkWidget * _new_headers(Mailer * mailer);
static void _new_config_load(Mailer * mailer);

Mailer * mailer_new(void)
{
	Mailer * mailer;
	GtkWidget * vbox;
	GtkWidget * vbox2;
	GtkWidget * hpaned;
	GtkWidget * vpaned;
	GtkWidget * widget;

	if((mailer = object_new(sizeof(*mailer))) == NULL)
	{
		error_print(PACKAGE);
		return NULL;
	}
	_new_plugins(mailer);
	mailer->account = NULL;
	mailer->account_cnt = 0;
	mailer->account_cur = NULL;
	mailer->folder_cur = NULL;
	/* widgets */
	mailer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mailer->window), 800, 600);
	gtk_window_set_title(GTK_WINDOW(mailer->window), "Mailer");
	g_signal_connect(G_OBJECT(mailer->window), "delete-event", G_CALLBACK(
				on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = common_new_menubar(GTK_WINDOW(mailer->window), _mailer_menubar,
			mailer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	widget = common_new_toolbar(_mailer_toolbar, mailer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 160);
	/* folders */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	mailer->view_folders = _new_folders_view(mailer);
	gtk_container_add(GTK_CONTAINER(widget), mailer->view_folders);
	gtk_paned_add1(GTK_PANED(hpaned), widget);
	vpaned = gtk_vpaned_new();
	gtk_paned_set_position(GTK_PANED(vpaned), 160);
	/* messages list */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	mailer->view_headers = _new_headers_view(mailer);
	gtk_container_add(GTK_CONTAINER(widget), mailer->view_headers);
	gtk_paned_add1(GTK_PANED(vpaned), widget);
	/* messages body */
	vbox2 = _new_headers(mailer);
	mailer->view_buffer = gtk_text_buffer_new(NULL);
	mailer->view_body = gtk_text_view_new_with_buffer(mailer->view_buffer);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(mailer->view_body), FALSE);
	widget = gtk_scrolled_window_new(NULL, NULL);
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
	/* load configuration */
	_new_config_load(mailer);
	/* show window */
	gtk_widget_show_all(mailer->window);
	gtk_widget_hide(mailer->hdr_vbox);
	mailer->pr_window = NULL;
	return mailer;
}

static int _new_plugins(Mailer * mailer)
{
	int ret = 0;
	char * dirname;
	DIR * dir;
	struct dirent * de;
	size_t len;
	char * filename;
	Plugin * handle;
	AccountPlugin * plugin;
	Account * p;

	mailer->available = NULL;
	mailer->available_cnt = 0;
	if((dirname = malloc(sizeof(PLUGINDIR) + strlen("/account")))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	sprintf(dirname, "%s%s", PLUGINDIR, "/account");
	if((dir = opendir(dirname)) == NULL)
	{
		error_set_code(1, "%s: %s", dirname, strerror(errno));
		free(dirname);
		return error_print(PACKAGE);
	}
	for(de = readdir(dir); de != NULL; de = readdir(dir))
	{
		if((len = strlen(de->d_name)) < 4
				|| strcmp(".so", &de->d_name[len - 3]) != 0)
			continue;
		if((filename = malloc(len - 2)) == NULL)
		{
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
			continue;
		}
		snprintf(filename, len - 2, "%s", de->d_name);
		if((handle = plugin_new(LIBDIR, PACKAGE, "account", filename))
				== NULL
				|| (plugin = plugin_lookup(handle,
						"account_plugin")) == NULL)
		{
			error_print(PACKAGE);
			if(handle != NULL)
				plugin_delete(handle);
			free(filename);
			continue;
		}
		free(filename);
		if((p = realloc(mailer->available, sizeof(*p)
						* (mailer->available_cnt + 1)))
				== NULL)
		{
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
			plugin_delete(handle);
			continue;
		}
		mailer->available = p;
		p = &p[mailer->available_cnt];
		p->name = strdup(de->d_name);
		p->title = strdup(plugin->name);
		if(p->name == NULL || p->title == NULL)
		{
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
			free(p->name);
			free(p->title);
		}
		else
		{
			p->name[len - 3] = '\0';
			p->handle = NULL;
			p->plugin = NULL;
			mailer->available_cnt++;
#ifdef DEBUG
			fprintf(stderr, "DEBUG: loaded %s: %s (%s)\n", filename,
					plugin->name, plugin->type);
#endif
		}
		plugin_delete(handle);
	}
	if(closedir(dir) != 0)
		ret = error_set_print(PACKAGE, 1, "%s: %s", dirname, strerror(
					errno));
	free(dirname);
	return ret;
}

static GtkWidget * _new_folders_view(Mailer * mailer)
{
	GtkWidget * widget;
	GtkTreeStore * model;
	GtkCellRenderer * renderer;
	GtkTreeSelection * treesel;

	model = gtk_tree_store_new(MF_COL_COUNT, G_TYPE_POINTER, G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN, G_TYPE_POINTER, GDK_TYPE_PIXBUF,
			G_TYPE_STRING);
	widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	g_object_unref(model);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(widget), -1,
			NULL, renderer, "pixbuf", MF_COL_ICON, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(widget), -1,
			"Folders", renderer, "text", MF_COL_NAME, NULL);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	g_signal_connect(G_OBJECT(treesel), "changed", G_CALLBACK(
				on_folder_change), mailer);
	return widget;
}

static void _headers_view_column_text(GtkTreeView * view, char const * title,
		int id);

static GtkWidget * _new_headers_view(Mailer * mailer)
{
	GtkWidget * widget;
	GtkTreeView * treeview;
	GtkTreeSelection * treesel;

	widget = gtk_tree_view_new();
	treeview = GTK_TREE_VIEW(widget);
	_headers_view_column_text(treeview, "Subject", MH_COL_SUBJECT);
	_headers_view_column_text(treeview, "From", MH_COL_FROM);
	_headers_view_column_text(treeview, "Date", MH_COL_DATE);
	treesel = gtk_tree_view_get_selection(treeview);
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(treesel), "changed", G_CALLBACK(
				on_header_change), mailer);
	return widget;
}

static void _headers_view_column_text(GtkTreeView * view, char const * title,
		int id)
	/* FIXME ellipsize text */
{
	GtkTreeViewColumn * column;

	column = gtk_tree_view_column_new_with_attributes(title,
			gtk_cell_renderer_text_new(), "text", id, NULL);
	gtk_tree_view_column_set_sort_column_id(column, id);
	gtk_tree_view_append_column(view, column);
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
	PangoFontDescription * bold;

	vbox = gtk_vbox_new(FALSE, 0);
	mailer->hdr_vbox = gtk_vbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	for(i = 0; widgets[i].hdr != NULL; i++)
	{
		hbox = gtk_hbox_new(FALSE, 0);
		widget = gtk_label_new(widgets[i].hdr);
		gtk_widget_modify_font(widget, bold);
		gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.0);
		gtk_size_group_add_widget(GTK_SIZE_GROUP(group), widget);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		*(widgets[i].widget) = gtk_label_new("");
		gtk_misc_set_alignment(GTK_MISC(*(widgets[i].widget)), 0.0,
				0.0);
		gtk_box_pack_start(GTK_BOX(hbox), *(widgets[i].widget), TRUE,
				TRUE, 0);
		gtk_box_pack_start(GTK_BOX(mailer->hdr_vbox), hbox, FALSE,
				FALSE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), mailer->hdr_vbox, FALSE, FALSE, 0);
	pango_font_description_free(bold);
	return vbox;
}

static void _new_config_load(Mailer * mailer)
{
	char * filename;
	char const * accounts;
	char * p;
	char * q;

	if((mailer->config = config_new()) == NULL)
		return;
	if((filename = mailer_get_config_filename(mailer)) == NULL)
		return;
	config_load(mailer->config, filename);
	free(filename);
	if((accounts = config_get(mailer->config, "", "accounts")) == NULL
			|| accounts[0] == '\0')
		return;
	if((p = strdup(accounts)) == NULL)
		return;
	accounts = p;
	for(q = p; *q != '\0'; q++)
	{
		if(*q != ',')
			continue;
		*q = '\0';
		_mailer_config_load_account(mailer, accounts);
		accounts = q + 1;
	}
	if(accounts[0] != '\0')
		_mailer_config_load_account(mailer, accounts);
	free(p);
}


/* mailer_delete */
void mailer_delete(Mailer * mailer)
{
	unsigned int i;

	for(i = 0; i < mailer->available_cnt; i++)
	{
		free(mailer->available[i].name);
		free(mailer->available[i].title);
	}
	free(mailer->available);
	for(i = 0; i < mailer->account_cnt; i++)
		account_delete(mailer->account[i]);
	free(mailer->account);
	object_delete(mailer);
}


/* accessors */
/* mailer_get_config */
char const * mailer_get_config(Mailer * mailer, char const * variable)
{
	return config_get(mailer->config, "", variable);
}


/* mailer_get_config_filename */
char * mailer_get_config_filename(Mailer * mailer)
	/* FIXME consider replacing with mailer_save_config() */
{
	char const * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	if((filename = malloc(strlen(homedir) + sizeof(MAILER_CONFIG_FILE) + 1))
			== NULL)
		return NULL;
	sprintf(filename, "%s/%s", homedir, MAILER_CONFIG_FILE);
	return filename;
}


/* useful */
/* mailer_error */
int mailer_error(Mailer * mailer, char const * message, int ret)
{
	GtkWidget * dialog;

	if(mailer == NULL)
		return error_set_print(PACKAGE, ret, "%s", message);
	dialog = gtk_message_dialog_new(GTK_WINDOW(mailer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* mailer_account_add */
int mailer_account_add(Mailer * mailer, Account * account)
	/* FIXME */
{
	Account ** p;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: mailer_account_add(%p)\n", account);
#endif
	if((p = realloc(mailer->account, sizeof(*p) * (mailer->account_cnt
						+ 1))) == NULL)
		return mailer_error(mailer, "realloc", FALSE);
	mailer->account = p;
	mailer->account[mailer->account_cnt] = account;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->view_folders));
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
	theme = gtk_icon_theme_get_default();
	pixbuf = gtk_icon_theme_load_icon(theme, "stock_mail-accounts", 16, 0,
			NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MF_COL_ACCOUNT,
			account, MF_COL_ENABLED, account_get_enabled(account),
			MF_COL_DELETE, FALSE, MF_COL_FOLDER, NULL, MF_COL_ICON,
			pixbuf, MF_COL_NAME, account->title, -1);
	account_init(account, GTK_TREE_STORE(model), &iter);
	mailer->account_cnt++;
	return FALSE;
}


#if 0 /* FIXME deprecate? */
/* mailer_account_disable */
int mailer_account_disable(Mailer * mailer, Account * account)
{
	unsigned int i;

	for(i = 0; i < mailer->account_cnt; i++)
		if(mailer->account[i] == account)
			break;
	if(i == mailer->account_cnt)
		return 1;
	account_set_enabled(account, 0);
	return 0;
}


/* mailer_account_enable */
int mailer_account_enable(Mailer * mailer, Account * account)
{
	unsigned int i;

	for(i = 0; i < mailer->account_cnt; i++)
		if(mailer->account[i] == account)
			break;
	if(i == mailer->account_cnt)
		return 1;
	account_set_enabled(account, 1);
	return 0;
}
#endif
