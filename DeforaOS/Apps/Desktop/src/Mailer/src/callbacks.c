/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "compose.h"
#include "mailer.h"
#include "callbacks.h"
#include "../config.h"


/* constants */
static char const * _authors[] =
{
	"Pierre 'khorben' Pronchery",
	NULL
};

/* FIXME */
static char const _license[] = "GPLv2";


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

void on_file_quit(GtkWidget * widget, gpointer data)
{
	/* FIXME may be composing */
	gtk_main_quit();
}


/* edit menu */
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

	if(mailer->pr_window != NULL)
	{
		gtk_widget_show(mailer->pr_window);
		return;
	}
	mailer->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(mailer->pr_window), 4);
	gtk_window_set_resizable(GTK_WINDOW(mailer->pr_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(mailer->pr_window),
			"Mailer preferences");
	gtk_window_set_transient_for(GTK_WINDOW(mailer->pr_window), GTK_WINDOW(
				mailer->window));
	g_signal_connect(G_OBJECT(mailer->pr_window), "delete_event",
			G_CALLBACK(_on_preferences_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	notebook = gtk_notebook_new();
	/* accounts */
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	store = gtk_list_store_new(1, G_TYPE_STRING);
	mailer->pr_accounts = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(mailer->pr_accounts),
			FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(mailer->pr_accounts),
			gtk_tree_view_column_new_with_attributes("Account name",
				gtk_cell_renderer_text_new(), "text", 0, NULL));
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
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_font_button_new();
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
	/* FIXME */
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
	char const copyright[] = "Copyright (c) 2006 khorben";
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
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
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
	/* FIXME */
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


/* preferences window */
void on_preferences_ok(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeModel * view_model;
	GtkTreeIter view_iter;

	gtk_widget_hide(mailer->pr_window);
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(mailer->pr_accounts));
	view_model = gtk_tree_view_get_model(GTK_TREE_VIEW(
				mailer->view_folders));
	if(gtk_tree_model_get_iter_first(model, &iter) != FALSE)
		do
		{
			/* FIXME check if already present, update if needed */
			/*       else add account with full information */
			mailer_account_add(mailer);
		}
		while(gtk_tree_model_iter_next(model, &iter) == TRUE);
	/* FIXME remove remaining accounts  */
}


void on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;

	gtk_widget_hide(mailer->pr_window);
	_preferences_set(mailer);
}


/* accounts */
/* types */
typedef struct _AccountData
{
	Mailer * mailer;
	unsigned int available;
	GtkWidget * settings;
	GtkWidget * confirm;
} AccountData;
#if !GTK_CHECK_VERSION(2, 10, 0)
/* macros */
# define GTK_ASSISTANT(widget) (widget)

/* types */
typedef enum _GtkAssistantPageType
{
	GTK_ASSISTANT_PAGE_CONTENT,
	GTK_ASSISTANT_PAGE_INTRO,
	GTK_ASSISTANT_PAGE_CONFIRM,
	GTK_ASSISTANT_PAGE_SUMMARY,
	GTK_ASSISTANT_PAGE_PROGRESS
} GtkAssistantPageType;

/* functions */
static GtkWidget * gtk_assistant_new(void);
static gint gtk_assistant_get_current_page(GtkWidget * assistant);
static gint gtk_assistant_append_page(GtkWidget * assistant,
		GtkWidget * widget);
static void gtk_assistant_set_page_type(GtkWidget * assistant, GtkWidget * page,
		GtkAssistantPageType type);
static void gtk_assistant_set_page_title(GtkWidget * assistant,
		GtkWidget * page, const gchar * title);
static void gtk_assistant_set_page_complete(GtkWidget *assistant,
		GtkWidget * page, gboolean complete);
#endif
static void _on_assistant_cancel(GtkWidget * widget, gpointer data);
static void _on_assistant_close(GtkWidget * widget, gpointer data);
static void _on_assistant_apply(GtkWidget * widget, gpointer data);
static void _on_assistant_prepare(GtkWidget * widget, GtkWidget * page,
		gpointer data);
static void _on_account_type_change(GtkWidget * widget, gpointer data);
void on_account_new(GtkWidget * widget, gpointer data)
{
	Mailer * mailer = data;
	AccountData * ad;
	GtkWidget * assistant;
	GtkWidget * vbox;
	unsigned int i;

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
	ad->available = 0;
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
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	widget = gtk_label_new("Please select the type of account\n"
			"you want to create now:"),
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	widget = gtk_combo_box_new_text();
	/* XXX this works because there is no plug-in list reload
	 *     would it be implemented this will need validation later */
	for(i = 0; i < mailer->available_cnt; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(widget),
				mailer->available[i].title);
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_on_account_type_change), ad);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), vbox,
			"Account type");
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), vbox,
			GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), vbox, TRUE);
	/* plug-in preferences */
	vbox = gtk_vbox_new(FALSE, 0);
	ad->settings = vbox;
	gtk_widget_show(vbox);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), vbox,
			"Account settings");
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), vbox,
			GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), vbox, TRUE);
	/* confirmation page */
	vbox = gtk_vbox_new(FALSE, 0);
	ad->confirm = vbox;
	gtk_widget_show(vbox);
	gtk_assistant_append_page(GTK_ASSISTANT(assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), vbox,
			"Account confirmation");
	gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), vbox,
			GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_complete(GTK_ASSISTANT(assistant), vbox, TRUE);
	gtk_widget_show(assistant);
}

#if !GTK_CHECK_VERSION(2, 10, 0)
/* types */
typedef struct _GtkAssistant
{
	GtkWidget * window;
	GtkWidget * frame;
	GtkWidget * label;
	GtkWidget ** page;
	unsigned int page_cnt;
	unsigned int page_cur;
} GtkAssistant;

/* functions */
static int _assistant_error(char const * message, int ret);
static gboolean _on_assistant_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_assistant_cancel(GtkWidget * widget, gpointer data);
static void _on_assistant_forward(GtkWidget * widget, gpointer data);
static void _on_assistant_back(GtkWidget * widget, gpointer data);
static GtkWidget * gtk_assistant_new(void)
{
	GtkAssistant * assistant;
	GtkWidget * window;
	GtkWidget * frame;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * hbox;

	if((assistant = calloc(0, sizeof(*assistant))) == NULL)
		_assistant_error("out of memory", 0);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_object_set_data(G_OBJECT(window), "assistant", assistant);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_assistant_closex), assistant);
	vbox = gtk_vbox_new(FALSE, 0);
	/* frame */
	frame = gtk_frame_new("");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	/* navigation buttons */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_assistant_cancel), assistant);
	gtk_widget_show(widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_BACK);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_assistant_back), assistant);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_assistant_forward), assistant);
	gtk_widget_show(widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	if(assistant == NULL)
		return window;
	assistant->window = window;
	assistant->frame = frame;
	assistant->label = gtk_frame_get_label_widget(GTK_FRAME(frame));
	return window;
}

static int _assistant_error(char const * message, int ret)
{
	fprintf(stderr, "%s%s%s", "*** GtkAssistant wrapper ", message,
			" ***\n");
	return ret;
}

static gboolean _on_assistant_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	_on_assistant_cancel(NULL, data);
	return TRUE;
}

static void _on_assistant_cancel(GtkWidget * widget, gpointer data)
{
	GtkAssistant * assistant = data;

	/* FIXME signals to handle? */
	gtk_widget_destroy(assistant->window);
	free(assistant->page);
	free(assistant);
}

static void _on_assistant_forward(GtkWidget * widget, gpointer data)
{
	GtkAssistant * assistant = data;

	/* FIXME */
}

static void _on_assistant_back(GtkWidget * widget, gpointer data)
{
	GtkAssistant * assistant = data;

	/* FIXME */
}


static gint gtk_assistant_append_page(GtkWidget * assistant, GtkWidget * widget)
{
	GtkAssistant * assistant;
	GtkWidget ** p;

	if((assistant = g_object_get_data(G_OBJECT(assistant), "assistant"))
			== NULL)
		return _assistant_error("data not found", -1);
	if((p = realloc(assistant->page, sizeof(*p) * (assistant->page_cnt+1)))
			== NULL)
		return _assistant_error(strerror(errno), -1);
	assistant->page = p;
	assistant->page[assistant->page_cnt] = widget;
	if(assistant->page_cnt == 0)
		gtk_container_add(GTK_CONTAINER(assistant->frame), widget);
	gtk_widget_show(widget);
	return assistant->page_cnt++;
}


static void gtk_assistant_set_page_title(GtkWidget * assistant,
		GtkWidget * page, const gchar * title)
{
	/* FIXME implement */
}
#endif

static void _on_assistant_cancel(GtkWidget * widget, gpointer data)
{
	_on_assistant_close(widget, data);
}

static void _on_assistant_close(GtkWidget * widget, gpointer data)
{
	AccountData * ad = data;

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
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, "account", -1);
}

static GtkWidget * _account_config_update(AccountConfig * config);
static GtkWidget * _account_config_display(AccountConfig * config);
static void _on_assistant_prepare(GtkWidget * widget, GtkWidget * page,
		gpointer data)
{
	AccountData * ad = data;
	/* FIXME should be defined in just one place */
	const char * title[3] = { "Account type", "Account settings",
		"Account confirmation" };
	unsigned int i;
	AccountConfig * config;

	i = gtk_assistant_get_current_page(GTK_ASSISTANT(widget));
	gtk_window_set_title(GTK_WINDOW(widget), title[i]);
	if(i == 1)
	{
		gtk_container_remove(GTK_CONTAINER(page), ad->settings);
		config = ad->mailer->available[ad->available].plugin->config;
		ad->settings = _account_config_update(config);
		gtk_container_add(GTK_CONTAINER(page), ad->settings);
	}
	else if(i == 2)
	{
		gtk_container_remove(GTK_CONTAINER(page), ad->confirm);
		config = ad->mailer->available[ad->available].plugin->config;
		ad->confirm = _account_config_display(config);
		gtk_container_add(GTK_CONTAINER(page), ad->confirm);
	}
}

static GtkWidget * _update_string(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _update_password(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _update_uint16(AccountConfig * config, GtkSizeGroup * group);
static GtkWidget * _update_boolean(AccountConfig * config);
static GtkWidget * _account_config_update(AccountConfig * config)
	/* FIXME append ":" to labels */
	/* FIXME consider working on a temporary plug-in instance instead
	 *       of the available ones */
{
	GtkWidget * vbox;
	GtkSizeGroup * group;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	for(i = 0; config[i].name != NULL; i++)
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				gtk_box_pack_start(GTK_BOX(vbox),
						_update_string(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_PASSWORD:
				gtk_box_pack_start(GTK_BOX(vbox),
						_update_password(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_UINT16:
				gtk_box_pack_start(GTK_BOX(vbox),
						_update_uint16(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_BOOLEAN:
				gtk_box_pack_start(GTK_BOX(vbox),
						_update_boolean(&config[i]),
							FALSE, TRUE, 0);
				break;
		}
	gtk_widget_show_all(vbox);
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
	gtk_size_group_add_widget(group, widget);
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
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _update_uint16(AccountConfig * config, GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_spin_button_new_with_range(0, 65535, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _update_boolean(AccountConfig * config)
{
	return gtk_check_button_new_with_label(config->title);
}

static GtkWidget * _display_string(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _display_password(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _display_uint16(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _display_boolean(AccountConfig * config,
		GtkSizeGroup * group);
static GtkWidget * _account_config_display(AccountConfig * config)
	/* FIXME append ":" to labels */
{
	GtkWidget * vbox;
	GtkSizeGroup * group;
	unsigned int i;

	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	for(i = 0; config[i].name != NULL; i++)
		switch(config[i].type)
		{
			case ACT_NONE:
				continue;
			case ACT_STRING:
				gtk_box_pack_start(GTK_BOX(vbox),
						_display_string(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_PASSWORD:
				gtk_box_pack_start(GTK_BOX(vbox),
						_display_password(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_UINT16:
				gtk_box_pack_start(GTK_BOX(vbox),
						_display_uint16(&config[i],
							group), FALSE, TRUE, 0);
				break;
			case ACT_BOOLEAN:
				gtk_box_pack_start(GTK_BOX(vbox),
						_display_boolean(&config[i],
							group), FALSE, TRUE, 0);
				break;
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
	widget = gtk_label_new("FIXME");
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	return hbox;
}

static GtkWidget * _display_password(AccountConfig * config,
		GtkSizeGroup * group)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new("FIXME");
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

	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(config->title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	snprintf(buf, sizeof(buf), "%u", (uint16_t)config->value);
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

static void _on_account_type_change(GtkWidget * widget, gpointer data)
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

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(mailer->pr_accounts), &path, NULL);
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
