/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Mailer */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";



#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "mailer.h"


/* Mailer */
/* private */
/* types */
struct _Mailer
{
	Account * available; /* XXX consider using another data type */
	unsigned int available_cnt;

	Account ** account;
	unsigned int account_cnt;
	Account * account_cur;
	AccountFolder * folder_cur;

	/* configuration */
	Config * config;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view_folders;
	GtkWidget * view_headers;
	GtkWidget * hdr_vbox;
	GtkWidget * hdr_subject;
	GtkWidget * hdr_from;
	GtkWidget * hdr_to;
	GtkWidget * hdr_date;
	GtkTextBuffer * view_buffer;
	GtkWidget * view_body;
	GtkWidget * statusbar;
	gint statusbar_id;
	/* about */
	GtkWidget * ab_window;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_accounts;
	GtkWidget * pr_messages_font;
};


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static const char * _title[3] =
{
	"New account", "Account settings", "Account confirmation"
};

static DesktopMenu _menu_file[] =
{
	{ "_New mail", G_CALLBACK(on_file_new_mail), "stock_mail-compose",
		GDK_CONTROL_MASK, GDK_N },
	{ "", NULL, NULL, 0, 0 },
	{ "Send / Receive", NULL, "stock_mail-send-receive", GDK_CONTROL_MASK,
		GDK_R },
	{ "", NULL, NULL, 0, 0 },
	{ "_Print", NULL, GTK_STOCK_PRINT, GDK_CONTROL_MASK, GDK_P },
	{ "Print pre_view", NULL, GTK_STOCK_PRINT_PREVIEW, GDK_CONTROL_MASK,
		0 },
	{ "", NULL, NULL, 0, 0 },
	{ "_Quit", G_CALLBACK(on_file_quit), GTK_STOCK_QUIT, GDK_CONTROL_MASK,
		GDK_Q },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_edit[] =
{
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_message[] =
{
	{ "_Reply", G_CALLBACK(on_message_reply), "stock_mail-reply", 0, 0 },
	{ "Reply to _all", G_CALLBACK(on_message_reply_to_all),
		"stock_mail-reply-to-all", 0, 0 },
	{ "_Forward", G_CALLBACK(on_message_forward), "stock_mail-forward", 0,
		0 },
	{ "", NULL, NULL, 0, 0 },
	{ "_Delete", NULL, GTK_STOCK_DELETE, 0, GDK_Delete },
	{ "", NULL, NULL, 0, 0 },
	{ "_View source", G_CALLBACK(on_message_view_source), NULL,
		GDK_CONTROL_MASK, GDK_U },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _mailer_menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Message", _menu_message },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};

static DesktopToolbar _mailer_toolbar[] =
{
	{ "New mail", G_CALLBACK(on_file_new_mail), "stock_mail-compose", 0,
		0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ "Send / Receive", NULL, "stock_mail-send-receive", 0, 0, NULL },
	{ "Stop", NULL, GTK_STOCK_STOP, 0, GDK_Escape, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ "Reply", G_CALLBACK(on_reply), "stock_mail-reply", 0, 0, NULL },
	{ "Reply to all", G_CALLBACK(on_reply_to_all),
		"stock_mail-reply-to-all", 0, 0, NULL },
	{ "Forward", G_CALLBACK(on_forward), "stock_mail-forward", 0, 0,
		NULL},
	{ "Delete", NULL, GTK_STOCK_DELETE, 0, 0, NULL },
	{ "Print", NULL, GTK_STOCK_PRINT, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* Mailer */
/* private */
/* prototypes */
static int _mailer_config_load_account(Mailer * mailer, char const * name);
static char * _mailer_get_config_filename(Mailer * mailer);


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
static void _on_folders_changed(GtkTreeSelection * selection, gpointer data);
static GtkWidget * _new_headers_view(Mailer * mailer);
static GtkWidget * _new_headers(Mailer * mailer);
static void _headers_view_column_text(GtkTreeView * view, char const * title,
		int id);
static void _on_headers_changed(GtkTreeSelection * selection, gpointer data);
static void _new_config_load(Mailer * mailer);

Mailer * mailer_new(void)
{
	Mailer * mailer;
	GtkAccelGroup * group;
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
	group = gtk_accel_group_new();
	mailer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(mailer->window), group);
	gtk_window_set_default_size(GTK_WINDOW(mailer->window), 800, 600);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(mailer->window), "stock_mail");
#endif
	gtk_window_set_title(GTK_WINDOW(mailer->window), "Mailer");
	g_signal_connect_swapped(G_OBJECT(mailer->window), "delete-event",
			G_CALLBACK(on_closex), mailer);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_mailer_menubar, mailer, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	widget = desktop_toolbar_create(_mailer_toolbar, mailer, group);
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
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(mailer->view_body),
			FALSE);
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
	/* widgets */
	mailer->ab_window = NULL;
	mailer->pr_window = NULL;
	/* show window */
	gtk_widget_show_all(vbox);
	gtk_widget_hide(mailer->hdr_vbox);
	gtk_widget_show(mailer->window);
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
				_on_folders_changed), mailer);
	return widget;
}

static void _on_folders_changed(GtkTreeSelection * selection, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkListStore * store;
	GtkTreePath * path;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		mailer->account_cur = NULL;
		mailer->folder_cur = NULL;
		gtk_tree_view_set_model(GTK_TREE_VIEW(mailer->view_headers),
				NULL);
		return;
	}
	/* get current folder */
	gtk_tree_model_get(model, &iter, MF_COL_FOLDER, &mailer->folder_cur,
			-1);
	/* get current account */
	path = gtk_tree_model_get_path(model, &iter);
	while(gtk_tree_path_get_depth(path) > 1 && gtk_tree_path_up(path));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, MF_COL_ACCOUNT, &mailer->account_cur,
			-1);
	gtk_tree_path_free(path);
	/* display headers */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() account_get_store()\n", __func__);
#endif
	store = account_get_store(mailer->account_cur, mailer->folder_cur);
	gtk_tree_view_set_model(GTK_TREE_VIEW(mailer->view_headers),
			GTK_TREE_MODEL(store));
}

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
				_on_headers_changed), mailer);
	return widget;
}

static void _on_headers_changed(GtkTreeSelection * selection, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GList * sel;
	GtkTreeIter iter;
	char * p;
	AccountMessage * message;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	sel = gtk_tree_selection_get_selected_rows(selection, &model);
	if(sel == NULL || sel->next != NULL) /* empty or multiple */
	{
		gtk_widget_hide(mailer->hdr_vbox);
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(mailer->view_body),
				mailer->view_buffer);
	}
	else
	{
		gtk_tree_model_get_iter(model, &iter, sel->data);
		gtk_tree_model_get(model, &iter, MH_COL_MESSAGE, &message, -1);
		gtk_tree_model_get(model, &iter, MH_COL_SUBJECT, &p, -1);
		gtk_label_set_text(GTK_LABEL(mailer->hdr_subject), p);
		gtk_tree_model_get(model, &iter, MH_COL_FROM, &p, -1);
		gtk_label_set_text(GTK_LABEL(mailer->hdr_from), p);
		gtk_tree_model_get(model, &iter, MH_COL_TO, &p, -1);
		gtk_label_set_text(GTK_LABEL(mailer->hdr_to), p);
		gtk_tree_model_get(model, &iter, MH_COL_DATE, &p, -1);
		gtk_label_set_text(GTK_LABEL(mailer->hdr_date), p);
		gtk_widget_show(mailer->hdr_vbox);
		account_select(mailer->account_cur, mailer->folder_cur,
				message);
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(mailer->view_body),
				mailer->account_cur->buffer);
	}
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
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
		{ " Subject: ",	NULL	},
		{ " From: ",	NULL	},
		{ " To: ",	NULL	},
		{ " Date: ",	NULL	},
		{ NULL,		NULL	}
	};
	int i;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;
	PangoFontDescription * bold;

	widgets[0].widget = &mailer->hdr_subject;
	widgets[1].widget = &mailer->hdr_from;
	widgets[2].widget = &mailer->hdr_to;
	widgets[3].widget = &mailer->hdr_date;
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
	char const * value;
	PangoFontDescription * font;
	char * p;
	char * q;

	if((mailer->config = config_new()) == NULL)
		return;
	if((filename = _mailer_get_config_filename(mailer)) == NULL)
		return;
	config_load(mailer->config, filename);
	free(filename);
	if((value = config_get(mailer->config, NULL, "messages_font")) != NULL)
	{
		font = pango_font_description_from_string(value);
		gtk_widget_modify_font(mailer->view_body, font);
		pango_font_description_free(font);
	}
	if((value = config_get(mailer->config, "", "accounts")) == NULL
			|| value[0] == '\0')
		return;
	if((p = strdup(value)) == NULL)
		return;
	value = p;
	for(q = p; *q != '\0'; q++)
	{
		if(*q != ',')
			continue;
		*q = '\0';
		_mailer_config_load_account(mailer, value);
		value = q + 1;
	}
	if(value[0] != '\0')
		_mailer_config_load_account(mailer, value);
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


/* mailer_show_about */
static gboolean _on_about_closex(gpointer data);

void mailer_show_about(Mailer * mailer, gboolean show)
{
	GtkWidget * dialog;

#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(mailer->ab_window != NULL)
	{
		if(show)
			gtk_widget_show(mailer->ab_window);
		else
			gtk_widget_hide(mailer->ab_window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		mailer_error(NULL, "malloc", 0);
		return;
	}
	dialog = gtk_about_dialog_new();
	mailer->ab_window = dialog;
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
				mailer->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), _authors);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), _copyright);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog),
			"stock_mail");
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog),
				_license);
	free(buf);
	g_signal_connect_swapped(G_OBJECT(dialog), "delete-event", G_CALLBACK(
				_on_about_closex), mailer);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(dialog);
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	/* FIXME implement */
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */
}

static gboolean _on_about_closex(gpointer data)
{
	Mailer * mailer = data;

	gtk_widget_hide(mailer->ab_window);
	return TRUE;
}


/* mailer_show_preferences */
typedef enum _AccountColumn
{
	AC_DATA = 0,
	AC_ACTIVE,
	AC_ENABLED,
	AC_TITLE,
	AC_TYPE
} AccountColumn;
#define AC_LAST AC_TYPE
#define AC_COUNT (AC_LAST + 1)
static void _preferences_set(Mailer * mailer);
static gboolean _on_preferences_closex(gpointer data);
static void _on_preferences_account_delete(gpointer data);
static void _on_preferences_account_edit(gpointer data);
static void _on_preferences_account_new(gpointer data);
static void _on_preferences_account_toggle(GtkCellRendererToggle * renderer,
		char * path, gpointer data);
static void _on_preferences_cancel(gpointer data);
static void _on_preferences_ok(gpointer data);
static int _preferences_ok_accounts(Mailer * mailer);
static int _preferences_ok_display(Mailer * mailer);
static int _preferences_ok_save(Mailer * mailer);

void mailer_show_preferences(Mailer * mailer, gboolean show)
{
	GtkWidget * notebook;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * vbox2;
	GtkWidget * vbox3;
	GtkWidget * widget;
	GtkSizeGroup * group;
	GtkListStore * store;
	size_t i;
	Account * ac;
	GtkTreeIter iter;
	GtkCellRenderer * renderer;

	if(mailer->pr_window != NULL)
	{
		if(show)
			gtk_widget_show(mailer->pr_window);
		else
			gtk_widget_hide(mailer->pr_window);
		return;
	}
	mailer->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mailer->pr_window), 300, 200);
	gtk_container_set_border_width(GTK_CONTAINER(mailer->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(mailer->pr_window),
			"Mailer preferences");
	gtk_window_set_transient_for(GTK_WINDOW(mailer->pr_window), GTK_WINDOW(
				mailer->window));
	g_signal_connect_swapped(G_OBJECT(mailer->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), mailer);
	vbox = gtk_vbox_new(FALSE, 4);
	notebook = gtk_notebook_new();
	/* accounts */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	store = gtk_list_store_new(AC_COUNT, G_TYPE_POINTER, G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
	for(i = 0; i < mailer->account_cnt; i++)
	{
		ac = mailer->account[i];
		gtk_list_store_insert_with_values(store, &iter, -1,
				AC_DATA, ac, AC_ACTIVE, TRUE,
				AC_ENABLED, ac->enabled, AC_TITLE, ac->title,
				AC_TYPE, ac->plugin->type, -1);
	}
	mailer->pr_accounts = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(mailer->pr_accounts),
			TRUE);
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(
				_on_preferences_account_toggle), store);
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Enabled",
				renderer, "active", AC_ENABLED, NULL));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Name",
				renderer, "text", AC_TITLE, NULL));
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Type",
				renderer, "text", AC_TYPE, NULL));
	gtk_container_add(GTK_CONTAINER(widget), mailer->pr_accounts);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	vbox3 = gtk_vbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_NEW);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_account_new), mailer);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, FALSE, TRUE, 0);
#if GTK_CHECK_VERSION(2, 6, 0)
	widget = gtk_button_new_from_stock(GTK_STOCK_EDIT);
#else
	widget = gtk_button_new_with_mnemonic("_Edit");
#endif
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_account_edit), mailer);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_account_delete), mailer);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox3, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, TRUE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new(
				"Accounts"));
	/* display */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* default font */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Messages font:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	mailer->pr_messages_font = gtk_font_button_new();
	widget = mailer->pr_messages_font;
	gtk_size_group_add_widget(group, widget);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(widget), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new(
				"Display"));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	/* dialog */
	hbox = gtk_hbox_new(FALSE, 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_ok), mailer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_cancel), mailer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mailer->pr_window), vbox);
	_preferences_set(mailer);
	gtk_widget_show_all(vbox);
	if(show)
		gtk_widget_show(mailer->pr_window);
	else
		gtk_widget_hide(mailer->pr_window);
}

static void _preferences_set(Mailer * mailer)
{
	char const * p;

	if((p = mailer_get_config(mailer, "messages_font")) == NULL)
		p = MAILER_MESSAGES_FONT;
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(mailer->pr_messages_font),
			p);
}

static gboolean _on_preferences_closex(gpointer data)
{
	Mailer * mailer = data;

	gtk_widget_hide(mailer->pr_window);
	return TRUE;
}


/* _on_preferences_account_new */
/* types */
typedef struct _AccountData
{
	Mailer * mailer;
	char * title;
	AccountIdentity identity;
	unsigned int available;
	Account * account;
	GtkWidget * assistant;
	GtkWidget * settings;
	GtkWidget * confirm;
} AccountData;

/* functions */
static GtkWidget * _assistant_account_select(AccountData * ad);
static GtkWidget * _assistant_account_config(AccountConfig * config);

#if !GTK_CHECK_VERSION(2, 10, 0)
# include "gtkassistant.c"
#endif
static void _on_assistant_cancel(GtkWidget * widget, gpointer data);
static void _on_assistant_close(GtkWidget * widget, gpointer data);
static void _on_assistant_apply(GtkWidget * widget, gpointer data);
static void _on_assistant_prepare(GtkWidget * widget, GtkWidget * page,
		gpointer data);
static void _on_entry_changed(GtkWidget * widget, gpointer data);
static void _on_account_type_changed(GtkWidget * widget, gpointer data);

static void _on_preferences_account_new(gpointer data)
{
	Mailer * mailer = data;
	AccountData * ad;
	GtkAssistant * assistant;
	GtkWidget * page;

	if(mailer->available_cnt == 0)
	{
		mailer_error(mailer, "No account plug-in available", 0);
		return;
	}
	if((ad = malloc(sizeof(*ad))) == NULL)
	{
		mailer_error(mailer, strerror(errno), 0);
		return;
	}
	ad->mailer = mailer;
	ad->title = strdup("");
	memset(&(ad->identity), 0, sizeof(ad->identity));
	ad->available = 0;
	ad->account = NULL;
	ad->assistant = gtk_assistant_new();
	assistant = GTK_ASSISTANT(ad->assistant);
	g_signal_connect(G_OBJECT(ad->assistant), "cancel", G_CALLBACK(
				_on_assistant_cancel), ad);
	g_signal_connect(G_OBJECT(ad->assistant), "close", G_CALLBACK(
				_on_assistant_close), ad);
	g_signal_connect(G_OBJECT(ad->assistant), "apply", G_CALLBACK(
				_on_assistant_apply), ad);
	g_signal_connect(G_OBJECT(ad->assistant), "prepare", G_CALLBACK(
				_on_assistant_prepare), ad);
	/* plug-in selection */
	page = _assistant_account_select(ad);
	gtk_assistant_append_page(assistant, page);
	gtk_assistant_set_page_title(assistant, page, _title[0]);
	gtk_assistant_set_page_type(assistant, page, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_complete(assistant, page, FALSE);
	/* plug-in preferences */
	page = gtk_vbox_new(FALSE, 0);
	ad->settings = page;
	gtk_widget_show(page);
	gtk_assistant_append_page(assistant, page);
	gtk_assistant_set_page_title(assistant, page, _title[1]);
	gtk_assistant_set_page_type(assistant, page,
			GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_complete(assistant, page, TRUE);
	/* confirmation page */
	page = gtk_vbox_new(FALSE, 0);
	ad->confirm = page;
	gtk_widget_show(page);
	gtk_assistant_append_page(assistant, page);
	gtk_assistant_set_page_title(assistant, page, _title[2]);
	gtk_assistant_set_page_type(assistant, page,
			GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_complete(assistant, page, TRUE);
	gtk_widget_show(ad->assistant);
}

static void _on_assistant_cancel(GtkWidget * widget, gpointer data)
{
	_on_assistant_close(widget, data);
}

static void _on_assistant_close(GtkWidget * widget, gpointer data)
{
	AccountData * ad = data;

	if(ad->account != NULL)
		account_delete(ad->account);
	free(ad);
	gtk_widget_destroy(widget);
}

static void _on_assistant_apply(GtkWidget * widget, gpointer data)
{
	AccountData * ad = data;
	GtkTreeModel * model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(ad->mailer->pr_accounts));
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
#ifdef DEBUG
	fprintf(stderr, "%s%p%s%s%s%s\n", "AC_DATA ", ad->account,
			", AC_ACTIVE FALSE, AC_ENABLED TRUE, AC_TITLE ",
			ad->account->title, ", AC_TYPE ",
			ad->account->plugin->type);
#endif
#if 0 /* FIXME move to mailer.c */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			AC_DATA, ad->account, AC_ACTIVE, FALSE,
			AC_ENABLED, TRUE, AC_TITLE, ad->account->title,
			AC_TYPE, ad->account->plugin->type, -1);
#endif
	ad->account = NULL;
	/* _on_assistant_close is then automatically called */
}

/* on_assistant_prepare */
static GtkWidget * _account_display(Account * account);

static void _on_assistant_prepare(GtkWidget * widget, GtkWidget * page,
		gpointer data)
{
	static int old = 0;
	AccountData * ad = data;
	unsigned int i;
	Account * ac;

	i = gtk_assistant_get_current_page(GTK_ASSISTANT(widget));
	gtk_window_set_title(GTK_WINDOW(widget), _title[i]);
	if(i == 1)
	{
		/* XXX something is wrong with gtk_container_remove */
		gtk_container_remove(GTK_CONTAINER(page), ad->settings);
		if(old == 0)
		{
			if(ad->account != NULL)
				account_delete(ad->account);
			ac = &(ad->mailer->available[ad->available]);
			ad->account = account_new(ac->name, ad->title);
		}
		if(ad->account == NULL)
		{
			mailer_error(ad->mailer, "Could not load plug-in", 0);
			gtk_assistant_set_current_page(GTK_ASSISTANT(widget),
					0);
			ad->settings = _assistant_account_select(ad);
		}
		else
			ad->settings = _assistant_account_config(
					ad->account->plugin->config);
		gtk_container_add(GTK_CONTAINER(page), ad->settings);
		gtk_widget_show_all(ad->settings);
	}
	else if(i == 2)
	{
		gtk_container_remove(GTK_CONTAINER(page), ad->confirm);
		ad->confirm = _account_display(ad->account);
		gtk_container_add(GTK_CONTAINER(page), ad->confirm);
	}
	old = i;
}

/* _assistant_account_select */
static void _on_account_name_changed(GtkWidget * widget, gpointer data);
static void _account_add_label(GtkWidget * box, PangoFontDescription * desc,
		GtkSizeGroup * group, char const * text);

static GtkWidget * _assistant_account_select(AccountData * ad)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * group;
	PangoFontDescription * desc;
	GtkWidget * widget;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 4);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	_account_add_label(hbox, desc, group, "Account name");
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_account_name_changed), ad);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	_account_add_label(hbox, desc, group, "Your name");
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &(ad->identity.from));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	_account_add_label(hbox, desc, group, "e-mail address");
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &(ad->identity.email));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	_account_add_label(hbox, desc, group, "Type of account");
	widget = gtk_combo_box_new_text();
	/* XXX this works because there is no plug-in list reload
	 *     would it be implemented this will need validation later */
	for(i = 0; i < ad->mailer->available_cnt; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(widget),
				ad->mailer->available[i].title);
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_account_type_changed), ad);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	pango_font_description_free(desc);
	gtk_widget_show_all(vbox);
	return vbox;
}

static void _on_account_name_changed(GtkWidget * widget, gpointer data)
{
	AccountData * ad = data;
	int current;
	GtkWidget * page;

	_on_entry_changed(widget, &ad->title);
	current = gtk_assistant_get_current_page(GTK_ASSISTANT(ad->assistant));
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT(ad->assistant),
			current);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(ad->assistant), page,
			strlen(ad->title) ? TRUE : FALSE);
}

static void _account_add_label(GtkWidget * box, PangoFontDescription * desc,
		GtkSizeGroup * group, char const * text)
{
	static char buf[80]; /* XXX hard-coded size */
	GtkWidget * label;

	snprintf(buf, sizeof(buf), "%s:", text);
	label = gtk_label_new(buf);
	if(desc != NULL)
		gtk_widget_modify_font(label, desc);
	if(group != NULL)
		gtk_size_group_add_widget(group, label);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);
}

/* _assistant_account_config */
static GtkWidget * _update_string(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _update_password(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _update_file(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _update_uint16(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _update_boolean(AccountConfig * config);

static GtkWidget * _assistant_account_config(AccountConfig * config)
{
	GtkWidget * vbox;
	GtkSizeGroup * group;
	PangoFontDescription * desc;
	GtkWidget * widget;
	size_t i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	for(i = 0; config[i].name != NULL; i++)
	{
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				widget = _update_string(&config[i], desc,
						group);
				break;
			case ACT_PASSWORD:
				widget = _update_password(&config[i], desc,
						group);
				break;
			case ACT_FILE:
				widget = _update_file(&config[i], desc, group);
				break;
			case ACT_UINT16:
				widget = _update_uint16(&config[i], desc,
						group);
				break;
			case ACT_BOOLEAN:
				widget = _update_boolean(&config[i]);
				break;
			default: /* should not happen */
				continue;
		}
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	pango_font_description_free(desc);
	return vbox;
}

static GtkWidget * _update_string(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_entry_new();
	if(config->value != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), config->value);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static GtkWidget * _update_password(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(config->value != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), config->value);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static void _on_file_activated(GtkWidget * widget, gpointer data);

static GtkWidget * _update_file(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_file_chooser_button_new("Choose file",
			GTK_FILE_CHOOSER_ACTION_OPEN);
	if(config->value != NULL)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget),
				config->value);
	gtk_file_chooser_button_set_title(GTK_FILE_CHOOSER_BUTTON(widget),
			config->title);
	g_signal_connect(G_OBJECT(widget), "file-set", G_CALLBACK(
				_on_file_activated), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static void _on_file_activated(GtkWidget * widget, gpointer data)
{
	char * filename;
	char ** value = data;
	char * p;

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, filename);
#endif
	if((p = realloc(*value, strlen(filename) + 1)) == NULL)
	{
		mailer_error(NULL, strerror(errno), 0);
		return;
	}
	*value = p;
	strcpy(p, filename);
}

static void _on_uint16_changed(GtkWidget * widget, gpointer data);

static GtkWidget * _update_uint16(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	uint16_t u16 = (intptr_t)(config->value);
	gdouble value = u16;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_spin_button_new_with_range(0, 65535, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), value);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
				_on_uint16_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static void _on_uint16_changed(GtkWidget * widget, gpointer data)
{
	int * value = data;

	*value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: new value is %d\n", *value);
#endif
}

static void _on_boolean_toggled(GtkWidget * widget, gpointer data);
static GtkWidget * _update_boolean(AccountConfig * config)
{
	GtkWidget * widget;

	widget = gtk_check_button_new_with_label(config->title);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
			config->value != NULL);
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
				_on_boolean_toggled), &config->value);
	return widget;
}

static void _on_boolean_toggled(GtkWidget * widget, gpointer data)
{
	int * value = data;

	*value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static GtkWidget * _display_string(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _display_password(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _display_file(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _display_uint16(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _display_boolean(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group);
static GtkWidget * _account_display(Account * account)
{
	AccountConfig * config = account->plugin->config;
	AccountConfig p;
	GtkWidget * vbox;
	GtkSizeGroup * group;
	PangoFontDescription * desc;
	GtkWidget * widget;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	p.name = NULL;
	p.title = "Account name";
	p.value = account->title;
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	widget = _display_string(&p, desc, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	for(i = 0; config[i].name != NULL; i++)
	{
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				widget = _display_string(&config[i], desc,
						group);
				break;
			case ACT_PASSWORD:
				widget = _display_password(&config[i], desc,
						group);
				break;
			case ACT_FILE:
				widget = _display_file(&config[i], desc, group);
				break;
			case ACT_UINT16:
				widget = _display_uint16(&config[i], desc,
						group);
				break;
			case ACT_BOOLEAN:
				widget = _display_boolean(&config[i], desc,
						group);
				break;
			default: /* should not happen */
				assert(0);
				continue;
		}
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	pango_font_description_free(desc);
	gtk_widget_show_all(vbox);
	return vbox;
}

static GtkWidget * _display_string(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_label_new(config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_file(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	return _display_string(config, desc, group);
}

static GtkWidget * _display_password(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_label_new("hidden");
	desc = pango_font_description_new();
	pango_font_description_set_style(desc, PANGO_STYLE_ITALIC);
	gtk_widget_modify_font(widget, desc);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_uint16(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	char buf[6];
	uint16_t u16 = (intptr_t)config->value;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	snprintf(buf, sizeof(buf), "%hu", u16);
	widget = gtk_label_new(buf);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_boolean(AccountConfig * config,
		PangoFontDescription * desc, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	_account_add_label(hbox, desc, group, config->title);
	widget = gtk_label_new(config->value != 0 ? "Yes" : "No");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

static void _on_entry_changed(GtkWidget * widget, gpointer data)
{
	const char * text;
	char ** value = data;
	char * p;

	text = gtk_entry_get_text(GTK_ENTRY(widget));
	if((p = realloc(*value, strlen(text) + 1)) == NULL)
	{
		mailer_error(NULL, strerror(errno), 0);
		return;
	}
	*value = p;
	strcpy(p, text);
}

static void _on_account_type_changed(GtkWidget * widget, gpointer data)
{
	AccountData * ad = data;

	ad->available = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
#ifdef DEBUG
	fprintf(stderr, "%s%u%s", "Account type ", ad->available, " active\n");
#endif
}

static void _on_preferences_account_toggle(GtkCellRendererToggle * renderer,
		char * path, gpointer data)
{
	GtkListStore * store = data;
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &iter, path);
	gtk_list_store_set(store, &iter, AC_ENABLED, 
			!gtk_cell_renderer_toggle_get_active(renderer), -1);
}

/* _on_preferences_account_edit */
static void _account_edit(Mailer * mailer, Account * account);

static void _on_preferences_account_edit(gpointer data)
{
	Mailer * mailer = data;
	GtkTreeSelection * selection;
	GtkTreeModel * model;
	GtkTreeIter iter;
	Account * account;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				mailer->pr_accounts));
	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, AC_DATA, &account, -1);
	_account_edit(mailer, account);
}

static void _account_edit(Mailer * mailer, Account * account)
{
	GtkWidget * window;
	char buf[80];
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * hbox;
	GtkSizeGroup * group;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", "Edit account: ", account_get_title(
				account));
	gtk_window_set_title(GTK_WINDOW(window), buf);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				mailer->window));
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	/* FIXME this affects the account directly (eg cancel does not) */
	widget = _assistant_account_config(account->plugin->config);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	/* FIXME implement properly */
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				gtk_widget_destroy), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				gtk_widget_destroy), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _on_preferences_account_delete(gpointer data)
{
	Mailer * mailer = data;
	GtkTreePath * path;
	GtkTreeModel * model;
	GtkTreeIter iter;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(mailer->pr_accounts), &path,
			NULL);
	if(path == NULL)
		return;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->pr_accounts));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	/* FIXME non-interface code (flag account as deleted and on ok apply) */
}

static void _on_preferences_cancel(gpointer data)
{
	Mailer * mailer = data;

	gtk_widget_hide(mailer->pr_window);
	_preferences_set(mailer);
}

static void _on_preferences_ok(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_preferences(mailer, FALSE);
	if(_preferences_ok_accounts(mailer) != 0
			|| _preferences_ok_display(mailer) != 0
			|| _preferences_ok_save(mailer) != 0)
		mailer_error(mailer, "An error occured while saving"
				" preferences", 0);
}

static int _preferences_ok_accounts(Mailer * mailer)
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeModel * view_model;
	gboolean loop;
	Account * account;
	gboolean active;
	gboolean enabled;
	char * title;
	size_t title_len;
	char * accounts = NULL;
	size_t accounts_len = 0;
	char * p;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->pr_accounts));
	view_model = gtk_tree_view_get_model(GTK_TREE_VIEW(
				mailer->view_folders));
	for(loop = gtk_tree_model_get_iter_first(model, &iter); loop == TRUE;
			loop = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, AC_DATA, &account,
				AC_ACTIVE, &active, AC_ENABLED, &enabled,
				AC_TITLE, &title, -1);
		title_len = strlen(title);
		if(account_config_save(account, mailer->config) != 0)
			return 1;
		if((p = realloc(accounts, accounts_len + title_len + 2))
				== NULL)
		{
			free(accounts);
			return 1;
		}
		accounts = p;
		sprintf(&accounts[accounts_len], "%s%s", accounts_len ? ","
				: "", title);
		accounts_len += title_len + (accounts_len ? 1 : 0);
		if(active)
		{
			if(enabled)
				continue;
#if 0 /* FIXME API+behaviour change here */
			if(mailer_account_disable(mailer, account) == 0)
				gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						AC_ACTIVE, FALSE, -1);
#endif
		}
		else if(enabled && mailer_account_add(mailer, account) == 0)
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					AC_ACTIVE, TRUE, -1);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: saved accounts \"%s\"\n", accounts);
#endif
	config_set(mailer->config, "", "accounts", accounts);
	free(accounts);
	return 0;
}

static int _preferences_ok_display(Mailer * mailer)
{
	char const * p;

	p = gtk_font_button_get_font_name(GTK_FONT_BUTTON(
				mailer->pr_messages_font));
	config_set(mailer->config, "", "messages_font", p);
	return 0;
}

static int _preferences_ok_save(Mailer * mailer)
{
	int ret;
	char * p;

	if((p = _mailer_get_config_filename(mailer)) == NULL)
		return 1;
	ret = config_save(mailer->config, p);
	free(p);
	return ret;
}


/* private */
/* functions */
/* mailer_get_config_filename */
static char * _mailer_get_config_filename(Mailer * mailer)
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
