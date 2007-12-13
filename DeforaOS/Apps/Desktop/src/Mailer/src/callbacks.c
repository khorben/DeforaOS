/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Mailer */
static char const _license[] =
"Mailer is free software; you can redistribute it and/or modify it under the\n"
"terms of the GNU General Public License version 2 as published by the Free\n"
"Software Foundation.\n"
"\n"
"Mailer is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
"FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more\n"
"details.\n"
"\n"
"You should have received a copy of the GNU General Public License along with\n"
"Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple\n"
"Place, Suite 330, Boston, MA  02111-1307  USA\n";



#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "compose.h"
#include "mailer.h"
#include "callbacks.h"


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

const char * title[3] =
{
	"New account", "Account settings", "Account confirmation"
};


/* callbacks */
/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_widget_hide(widget);
	/* FIXME may be composing */
	gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_new_mail(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;

	compose_new(mailer);
}


void on_file_send_receive(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_file_print(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_file_print_preview(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_file_quit(GtkWidget * widget, gpointer data)
{
	/* FIXME may be composing */
	gtk_main_quit();
}


/* message menu */
void on_message_reply(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_message_reply_to_all(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_message_forward(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_message_delete(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


/* edit menu */
typedef enum _AccountColumn
{
	AC_DATA,
	AC_ACTIVE,
	AC_ENABLED,
	AC_TITLE,
	AC_TYPE
} AccountColumn;
#define AC_LAST AC_TYPE
static void _preferences_set(Mailer * mailer);
static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);

void on_edit_preferences(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	GtkWidget * notebook;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * vbox2;
	GtkWidget * vbox3;
	GtkSizeGroup * group;
	GtkListStore * store;
	size_t i;
	Account * ac;
	GtkTreeIter iter;

	if(mailer->pr_window != NULL)
	{
		gtk_widget_show(mailer->pr_window);
		return;
	}
	mailer->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mailer->pr_window), 300, 200);
	gtk_container_set_border_width(GTK_CONTAINER(mailer->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(mailer->pr_window),
			"Mailer preferences");
	gtk_window_set_transient_for(GTK_WINDOW(mailer->pr_window), GTK_WINDOW(
				mailer->window));
	g_signal_connect(G_OBJECT(mailer->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), NULL);
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
	store = gtk_list_store_new(AC_LAST + 1, G_TYPE_POINTER, G_TYPE_BOOLEAN,
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
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Enabled",
				gtk_cell_renderer_toggle_new(), "active",
				AC_ENABLED, NULL));
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Name",
				gtk_cell_renderer_text_new(), "text", AC_TITLE,
				NULL));
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Type",
				gtk_cell_renderer_text_new(), "text", AC_TYPE,
				NULL));
	gtk_container_add(GTK_CONTAINER(widget), mailer->pr_accounts);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	vbox3 = gtk_vbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_NEW);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_account_new), mailer);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, FALSE, TRUE, 0);
#if GTK_CHECK_VERSION(2, 6, 0)
	widget = gtk_button_new_from_stock(GTK_STOCK_EDIT);
#else
	widget = gtk_button_new_with_mnemonic("_Edit");
#endif
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_account_edit), mailer);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_account_delete), mailer);
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
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_preferences_ok), mailer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_preferences_cancel), mailer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mailer->pr_window), vbox);
	_preferences_set(mailer);
	gtk_widget_show_all(mailer->pr_window);
}

static void _preferences_set(Mailer * mailer)
{
	char * p;

	if((p = config_get(mailer->config, "", "message_font")) == NULL)
		p = MAILER_MESSAGES_FONT;
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(mailer->pr_messages_font),
			p);
}

static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* help menu */
static gboolean _on_about_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);

void on_help_about(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	static GtkWidget * window = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		mailer_error(mailer, "malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				mailer->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_about_closex), NULL);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	/* FIXME implement */
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */

static gboolean _on_about_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* toolbar */
void on_new_mail(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;

	compose_new(mailer);
}


void on_stop(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_reply(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_reply_to_all(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_forward(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_delete(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_print(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


/* folder view */
void on_folder_change(GtkTreeSelection * selection, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkListStore * store;
	Account * a;
	AccountFolder * af;

#ifdef DEBUG
	fputs("DEBUG: on_folder_change()\n", stderr);
#endif
	if(!gtk_tree_selection_get_selected(selection, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, MF_COL_ACCOUNT, &a, MF_COL_FOLDER, &af,
			-1);
	store = account_get_store(a, af);
	gtk_tree_view_set_model(GTK_TREE_VIEW(mailer->view_headers),
			GTK_TREE_MODEL(store));
}


/* header view */
void on_header_change(GtkTreeSelection * selection, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GList * sel;
	GtkTreeIter iter;
	char * p;

	sel = gtk_tree_selection_get_selected_rows(selection, &model);
	if(sel == NULL || sel->next != NULL) /* empty or multiple */
	{
		gtk_widget_hide(mailer->hdr_vbox);
		return;
	}
	gtk_tree_model_get_iter(model, &iter, sel->data);
	gtk_tree_model_get(model, &iter, MH_COL_SUBJECT, &p, -1);
	gtk_label_set_text(GTK_LABEL(mailer->hdr_subject), p);
	gtk_tree_model_get(model, &iter, MH_COL_FROM, &p, -1);
	gtk_label_set_text(GTK_LABEL(mailer->hdr_from), p);
	gtk_tree_model_get(model, &iter, MH_COL_TO, &p, -1);
	gtk_label_set_text(GTK_LABEL(mailer->hdr_to), p);
	gtk_tree_model_get(model, &iter, MH_COL_DATE, &p, -1);
	gtk_label_set_text(GTK_LABEL(mailer->hdr_date), p);
	gtk_widget_show(mailer->hdr_vbox);
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
}


/* preferences window */
void on_preferences_ok(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeModel * view_model;
	Account * account;
	gboolean active;
	gboolean enabled;
	char * title;
	char * accounts = NULL;
	size_t accounts_len = 0;
	char const * p;
	char * q;

	gtk_widget_hide(mailer->pr_window);
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->pr_accounts));
	view_model = gtk_tree_view_get_model(GTK_TREE_VIEW(
				mailer->view_folders));
	if(gtk_tree_model_get_iter_first(model, &iter) == FALSE)
		return; /* FIXME can no longer return here */
	do
	{
		gtk_tree_model_get(model, &iter, AC_DATA, &account,
				AC_ACTIVE, &active, AC_ENABLED, &enabled,
				AC_TITLE, &title, -1);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: title=\"%s\"\n", title);
#endif
		if((q = realloc(accounts, accounts_len + strlen(title) + 2))
				!= NULL) /* FIXME catch error */
		{
			accounts = q;
			sprintf(&accounts[accounts_len], "%s%s", accounts_len
					? "," : "", title);
			accounts_len += strlen(title) + (accounts_len ? 1 : 0);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: accounts=\"%s\"\n", accounts);
#endif
		}
		account_config_save(account, mailer->config);
		if(active)
		{
			if(enabled)
				continue;
			if(mailer_account_disable(mailer, account) == 0)
				gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						AC_ACTIVE, FALSE, -1);
		}
		else if(enabled && mailer_account_add(mailer, account) == 0)
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					AC_ACTIVE, TRUE, -1);
	}
	while(gtk_tree_model_iter_next(model, &iter) == TRUE);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: saved accounts \"%s\"\n", accounts);
#endif
	config_set(mailer->config, "", "accounts", accounts);
	free(accounts);
	p = gtk_font_button_get_font_name(GTK_FONT_BUTTON(
				mailer->pr_messages_font));
	config_set(mailer->config, "", "messages_font", p);
	q = mailer_get_config_filename(mailer);
	config_save(mailer->config, q);
	free(q);
}


void on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;

	gtk_widget_hide(mailer->pr_window);
	_preferences_set(mailer);
}


/* accounts */
/* on_account_new */
/* types */
typedef struct _AccountData
{
	Mailer * mailer;
	char * title;
	AccountIdentity identity;
	unsigned int available;
	Account * account;
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

void on_account_new(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	AccountData * ad;
	GtkWidget * assistant;
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
	ad->title = NULL;
	memset(&(ad->identity), 0, sizeof(ad->identity));
	ad->available = 0;
	ad->account = NULL;
	assistant = gtk_assistant_new();
	g_signal_connect(G_OBJECT(assistant), "cancel", G_CALLBACK(
				_on_assistant_cancel), ad);
	g_signal_connect(G_OBJECT(assistant), "close", G_CALLBACK(
				_on_assistant_close), ad);
	g_signal_connect(G_OBJECT(assistant), "apply", G_CALLBACK(
				_on_assistant_apply), ad);
	g_signal_connect(G_OBJECT(assistant), "prepare", G_CALLBACK(
				_on_assistant_prepare), ad);
	/* plug-in selection */
	page = _assistant_account_select(ad);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), page);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page,
			title[0]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page,
			GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), page, TRUE);
	/* plug-in preferences */
	page = gtk_vbox_new(FALSE, 0);
	ad->settings = page;
	gtk_widget_show(page);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), page);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page,
			title[1]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page,
			GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), page, TRUE);
	/* confirmation page */
	page = gtk_vbox_new(FALSE, 0);
	ad->confirm = page;
	gtk_widget_show(page);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), page);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page, title[2]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page,
			GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), page, TRUE);
	gtk_widget_show(assistant);
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
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			AC_DATA, ad->account, AC_ACTIVE, FALSE,
			AC_ENABLED, TRUE, AC_TITLE, ad->account->title,
			AC_TYPE, ad->account->plugin->type, -1);
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
	gtk_window_set_title(GTK_WINDOW(widget), title[i]);
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
static GtkWidget * _assistant_account_select(AccountData * ad)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * group;
	GtkWidget * widget;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Account name:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &(ad->title));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Your name:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &(ad->identity.from));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("e-mail address:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_entry_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &(ad->identity.email));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Type of account:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_combo_box_new_text();
	gtk_size_group_add_widget(group, widget);
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
	gtk_widget_show_all(vbox);
	return vbox;
}

/* _assistant_account_config */
static GtkWidget * _update_string(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _update_password(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _update_file(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _update_uint16(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _update_boolean(AccountConfig * config);

static GtkWidget * _assistant_account_config(AccountConfig * config)
	/* FIXME append ":" to labels */
{
	GtkWidget * vbox;
	GtkSizeGroup * group;
	GtkWidget * widget;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	for(i = 0; config[i].name != NULL; i++)
	{
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				widget = _update_string(&config[i], group);
				break;
			case ACT_PASSWORD:
				widget = _update_password(&config[i], group);
				break;
			case ACT_FILE:
				widget = _update_file(&config[i], group);
				break;
			case ACT_UINT16:
				widget = _update_uint16(&config[i], group);
				break;
			case ACT_BOOLEAN:
				widget = _update_boolean(&config[i]);
				break;
			default: /* should not happen */
				continue;
		}
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _update_string(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_entry_new();
	if(config->value != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), config->value);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _update_password(AccountConfig * config,
		GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(config->value != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), config->value);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_entry_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _update_file(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_file_chooser_button_new("Choose file",
			GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_button_set_title(GTK_FILE_CHOOSER_BUTTON(widget),
			config->title);
	/* FIXME implement signal handler */
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static void _on_uint16_changed(GtkWidget * widget, gpointer data);

static GtkWidget * _update_uint16(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	uint16_t u16 = (uint16_t)(config->value);
	gdouble value = u16;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_spin_button_new_with_range(0, 65535, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), value);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
				_on_uint16_changed), &config->value);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static void _on_uint16_changed(GtkWidget * widget, gpointer data)
{
	uint16_t * value = data;

	*value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
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
		GtkSizeGroup * group);
static GtkWidget * _display_password(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _display_file(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _display_uint16(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _display_boolean(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _account_display(Account * account)
	/* FIXME append ":" to labels */
{
	AccountConfig * config = account->plugin->config;
	AccountConfig p;
	GtkWidget * vbox;
	GtkSizeGroup * group;
	GtkWidget * widget;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	p.name = NULL;
	p.title = "Account name";
	p.value = account->title;
	widget = _display_string(&p, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	for(i = 0; config[i].name != NULL; i++)
	{
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				widget = _display_string(&config[i], group);
				break;
			case ACT_PASSWORD:
				widget = _display_password(&config[i], group);
				break;
			case ACT_FILE:
				widget = _display_file(&config[i], group);
				break;
			case ACT_UINT16:
				widget = _display_uint16(&config[i], group);
				break;
			case ACT_BOOLEAN:
				widget = _display_boolean(&config[i], group);
				break;
			default: /* should not happen */
				assert(0);
				continue;
		}
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	gtk_widget_show_all(vbox);
	return vbox;
}

static GtkWidget * _display_string(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(config->value);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_file(AccountConfig * config, GtkSizeGroup * group)
{
	return _display_string(config, group);
}

static GtkWidget * _display_password(AccountConfig * config,
		GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	PangoFontDescription * desc;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new("hidden");
	desc = pango_font_description_new();
	pango_font_description_set_style(desc, PANGO_STYLE_ITALIC);
	gtk_widget_modify_font(widget, desc);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_uint16(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	char buf[6];
	uint16_t u16 = (uint16_t)config->value;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	snprintf(buf, sizeof(buf), "%u", u16);
	widget = gtk_label_new(buf);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_boolean(AccountConfig * config,
		GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(config->value != 0 ? "Yes" : "No");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
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

void on_account_edit(GtkWidget * widget, gpointer data)
{
	/* FIXME */
}


void on_account_delete(GtkWidget * widget, gpointer data)
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
	/* FIXME non-interface code */
}


/* compose window */
gboolean on_compose_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Compose * c = data;

	compose_delete(c);
	return TRUE;
}


void on_compose_save(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_save(c);
}


void on_compose_send(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_send(c);
}


/* compose file menu */
void on_compose_file_send(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_compose_file_save(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


void on_compose_file_close(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	compose_delete(c);
}


/* compose view menu */
void on_compose_view_cc(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	gtk_widget_show(c->tb_cc);
}


void on_compose_view_bcc(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	gtk_widget_show(c->tb_bcc);
}


void on_compose_help_about(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	on_help_about(widget, c->mailer);
}


/* send mail */
gboolean on_send_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Compose * c = data;

	on_send_cancel(widget, c);
	return FALSE;
}


void on_send_cancel(GtkWidget * widget, gpointer data)
{
	Compose * c = data;

	g_io_channel_shutdown(c->channel, TRUE, NULL);
	gtk_widget_destroy(c->snd_window);
	free(c->buf);
}


gboolean on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Compose * c = data;
	gsize i;

	if((i = (c->buf_len - c->buf_pos) % 512) == 0)
		i = 512;
	if(g_io_channel_write_chars(source, &c->buf[c->buf_pos], i, &i, NULL)
			!= G_IO_STATUS_NORMAL)
	{
		mailer_error(c->mailer, strerror(errno), FALSE);
		on_send_cancel(c->snd_window, c);
		return FALSE;
	}
	c->buf_pos+=i;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(c->snd_progress),
			c->buf_pos / c->buf_len);
	if(c->buf_pos >= c->buf_len)
	{
		on_send_cancel(c->snd_window, c);
		compose_delete(c);
		return FALSE;
	}
	return TRUE;
}
