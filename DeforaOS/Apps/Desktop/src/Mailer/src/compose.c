/* compose.c */



#include <unistd.h>
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

static struct _menu _menu_view[] =
{
	/* FIXME CC and BCC should be toggle menu entries */
	{ "_CC field", G_CALLBACK(on_compose_view_cc), NULL },
	{ "_BCC field", G_CALLBACK(on_compose_view_bcc), NULL },
	{ NULL, NULL, NULL }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_compose_help_about), GTK_STOCK_ABOUT },
#else
	{ "_About", G_CALLBACK(on_compose_help_about), NULL },
#endif
	{ NULL, NULL, NULL }
};

static struct _menubar _compose_menubar[] =
{
	{ "_File", _menu_file },
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* compose_new */
Compose * compose_new(Mailer * mailer)
{
	Compose * compose;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkSizeGroup * group;
	GtkWidget * widget;
	PangoFontDescription * desc;

	if((compose = malloc(sizeof(*compose))) == NULL)
	{
		mailer_error(mailer, strerror(errno), 0);
		return NULL;
	}
	compose->mailer = mailer;
	compose->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(compose->window), "Mailer - Compose");
	gtk_window_set_default_size(GTK_WINDOW(compose->window), 512, 384);
	g_signal_connect(G_OBJECT(compose->window), "delete_event", G_CALLBACK(
				on_compose_closex), compose);
	vbox = gtk_vbox_new(FALSE, 0);
	widget = common_new_menubar(_compose_menubar, compose);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_button_new(NULL, "Send");
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_compose_send), compose);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_compose_save), compose);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* from */
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" From: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->from = gtk_combo_box_entry_new_text();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->from);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* to */
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" To: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->to = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->to);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* cc */
	compose->tb_cc = gtk_toolbar_new();
	widget = gtk_label_new(" CC: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_cc), toolitem, -1);
	compose->cc = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->cc);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_cc), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), compose->tb_cc, FALSE, FALSE, 0);
	/* bcc */
	compose->tb_bcc = gtk_toolbar_new();
	widget = gtk_label_new(" BCC: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_bcc), toolitem, -1);
	compose->bcc = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->bcc);
	gtk_toolbar_insert(GTK_TOOLBAR(compose->tb_bcc), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), compose->tb_bcc, FALSE, FALSE, 0);
	/* subject */
	toolbar = gtk_toolbar_new();
	widget = gtk_label_new(" Subject: ");
	gtk_size_group_add_widget(group, widget);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	compose->subject = gtk_entry_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), compose->subject);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	compose->view = gtk_text_view_new();
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(compose->view, desc);
	pango_font_description_free(desc);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), compose->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	compose->statusbar = gtk_statusbar_new();
	compose->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), compose->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(compose->window), vbox);
	gtk_widget_show_all(compose->window);
	/* FIXME should not be showed in the first place */
	gtk_widget_hide(compose->tb_cc);
	gtk_widget_hide(compose->tb_bcc);
	return compose;
}


/* compose_delete */
void compose_delete(Compose * compose)
{
	gtk_widget_hide(compose->window);
	free(compose);
}


/* useful */
/* compose_save */
void compose_save(Compose * compose)
{
	/* FIXME implement */
}


/* compose_send */
static char * _send_headers(Compose * compose);
static char * _send_body(GtkWidget * view);
static int _send_mail(Compose * compose, char * msg, size_t msg_len);
void compose_send(Compose * compose)
{
	char * msg;
	size_t msg_len;
	char * body;
	size_t body_len;
	char * p;

	if((msg = _send_headers(compose)) == NULL)
		return;
	if((body = _send_body(compose->view)) == NULL)
	{
		free(msg);
		return;
	}
	msg_len = strlen(msg);
	body_len = strlen(body);
	if((p = realloc(msg, msg_len + body_len + 8)) == NULL)
		mailer_error(compose->mailer, "Memory allocation", 0);
	else
	{
		msg = p;
		snprintf(&msg[msg_len], body_len + 8, "\r\n%s\r\n.\r\n", body);
		msg_len+=body_len+7;
	}
	g_free(body);
	_send_mail(compose, msg, msg_len);
	free(msg);
}

static int _send_mail(Compose * compose, char * msg, size_t msg_len)
{
	int fd[2];
	pid_t pid;

	if(pipe(fd) != 0)
		return mailer_error(compose->mailer, strerror(errno), 1);
	if((pid = fork()) == -1)
		return mailer_error(compose->mailer, strerror(errno), 1);
	if(pid == 0)
	{
		close(0);
		dup2(fd[0], 0);
		execl("/usr/sbin/sendmail", "sendmail", "-bm", "-t", NULL);
		exit(2);
	}
	/* FIXME send mail progressively, get sendmail's output */
	write(1, msg, msg_len);
	write(fd[1], msg, msg_len);
	return 0;
}

static char * _send_headers(Compose * compose)
{
	struct {
		char * hdr;
		GtkWidget * wgt;
	} widgets[] =
	{
/* FIXME	{ "From: ", compose->from }, is not an entry */
		{ "To: ", compose->to },
		{ "Cc: ", compose->cc },
		{ "Bcc: ", compose->bcc },
		{ "Subject: ", compose->subject },
		{ NULL, NULL }
	};
	int i;
	char * msg = NULL;
	size_t msg_len = 0;
	char const * p;
	size_t len;
	size_t hdr_len;
	char * q;

	for(i = 0; widgets[i].hdr != NULL; i++)
	{
		p = gtk_entry_get_text(GTK_ENTRY(widgets[i].wgt));
		if((len = strlen(p)) == 0)
			continue;
		hdr_len = strlen(widgets[i].hdr);
		if((q = realloc(msg, msg_len + hdr_len + len + 3)) == NULL)
		{
			free(msg);
			mailer_error(compose->mailer, "Memory allocation", 0);
			return NULL;
		}
		msg = q;
		snprintf(&msg[msg_len], hdr_len + len + 3, "%s%s\r\n",
				widgets[i].hdr, p);
		msg_len+=hdr_len+len+2;
	}
	if(msg != NULL)
		return msg;
	if((msg = strdup("")) == NULL)
		mailer_error(compose->mailer, "Memory allocation", 0);
	return msg;
}

static char * _send_body(GtkWidget * view)
{
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;

	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	/* FIXME allocating the complete message is not optimal */
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tbuf), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tbuf), &end);
	return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tbuf), &start, &end,
			FALSE);
}
