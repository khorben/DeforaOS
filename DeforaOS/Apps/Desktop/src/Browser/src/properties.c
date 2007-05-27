/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
/* FIXME:
 * - add dates */



#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <gtk/gtk.h>


/* properties */
/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
static int _properties_error(char const * message, int ret);
static int _properties_do(char const * filename);

/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _properties_on_close(GtkWidget * widget, gpointer data);

static int _properties(int filec, char * const filev[])
{
	int ret = 0;
	int i;

	for(i = 0; i < filec; i++)
	{
		_properties_cnt++;
		/* FIXME if relative path get the full path */
		ret |= _properties_do(filev[i]);
	}
	return ret;
}


/* _properties_error */
static void _error_response(GtkDialog * dialog, gint arg, gpointer data);
static int _properties_error(char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), &_properties_cnt);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkDialog * dialog, gint arg, gpointer data)
{
	unsigned int * cnt = data;

	(*cnt)--;
	if(*cnt == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(GTK_WIDGET(dialog));
}

/* _properties_do */
static char * _do_size(char * buf, size_t buf_cnt, size_t size);
static char * _do_owner(char * buf, size_t buf_cnt, uid_t uid);
static char * _do_group(char * buf, size_t buf_cnt, gid_t gid);
static GtkWidget * _do_mode(PangoFontDescription * bold, char const * name,
		mode_t mode);

static int _properties_do(char const * filename)
{
	char const * gfilename;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	PangoFontDescription * bold;
	struct stat st;
	char buf[256];

	if(lstat(filename, &st) != 0)
		return _properties_error(filename, 1);
	if((gfilename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL))
			== NULL)
		gfilename = filename;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(buf, sizeof(buf), "%s%s", "Properties of ", gfilename);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_properties_on_closex), &_properties_cnt);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(TRUE, 0);
	widget = gtk_image_new_from_stock(S_ISDIR(st.st_mode)
			? GTK_STOCK_DIRECTORY : GTK_STOCK_FILE,
			GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_label_new(gfilename);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(TRUE, 0); /* size */
	widget = gtk_label_new("Size:"); /* XXX justification does not work */
	gtk_widget_modify_font(widget, bold);
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_label_new(_do_size(buf, sizeof(buf), st.st_size));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(TRUE, 0); /* owner */
	widget = gtk_label_new("Owner:");
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_label_new(_do_owner(buf, sizeof(buf), st.st_uid));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(TRUE, 0); /* group */
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	widget = gtk_label_new("Group:");
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_label_new(_do_group(buf, sizeof(buf), st.st_gid));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(TRUE, 0); /* permissions */
	widget = gtk_label_new("Permissions:");
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_label_new(""); /* XXX to balance the columns */
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = _do_mode(bold, "Owner:", (st.st_mode & 0700) >> 6); /* owner */
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = _do_mode(bold, "Group:", (st.st_mode & 0070) >> 3); /* group */
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = _do_mode(bold, "Others:", st.st_mode & 0007); /* others */
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	widget = gtk_hseparator_new(); /* separator */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0); /* bottom box */
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_properties_on_close), &_properties_cnt);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	pango_font_description_free(bold);
	return 0;
}

static char * _do_size(char * buf, size_t buf_cnt, size_t size)
{
	double sz = size;
	char * unit;

	if(sz < 1024)
	{
		snprintf(buf, buf_cnt, "%.0f%s", sz, " bytes");
		return buf;
	}
	else if((sz /= 1024) < 1024)
		unit = "KB";
	else if((sz /= 1024) < 1024)
		unit = "MB";
	else if((sz /= 1024) < 1024)
		unit = "GB";
	else
	{
		sz /= 1024;
		unit = "TB";
	}
	snprintf(buf, buf_cnt, "%.1f %s", sz, unit);
	return buf;
}

static char * _do_owner(char * buf, size_t buf_cnt, uid_t uid)
{
	struct passwd * pw;

	if((pw = getpwuid(uid)) != NULL)
		return pw->pw_name;
	snprintf(buf, buf_cnt, "%lu", (unsigned long)uid);
	return buf;
}

static char * _do_group(char * buf, size_t buf_cnt, gid_t gid)
{
	struct group * gr;

	if((gr = getgrgid(gid)) != NULL)
		return gr->gr_name;
	snprintf(buf, buf_cnt, "%lu", (unsigned long)gid);
	return buf;
}

static GtkWidget * _do_mode(PangoFontDescription * bold, char const * name,
		mode_t mode)
{
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * hbox2;

	hbox = gtk_hbox_new(TRUE, 0);
	widget = gtk_label_new(name);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	hbox2 = gtk_hbox_new(TRUE, 0);
	widget = gtk_check_button_new_with_label("read"); /* read */
	gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IROTH);
	gtk_box_pack_start(GTK_BOX(hbox2), widget, TRUE, TRUE, 4);
	widget = gtk_check_button_new_with_label("write"); /* write */
	gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IWOTH);
	gtk_box_pack_start(GTK_BOX(hbox2), widget, TRUE, TRUE, 4);
	widget = gtk_check_button_new_with_label("execute"); /* execute */
	gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IXOTH);
	gtk_box_pack_start(GTK_BOX(hbox2), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), hbox2, TRUE, TRUE, 4);
	return hbox;
}


/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	_properties_on_close(widget, data);
	return FALSE;
}

static void _properties_on_close(GtkWidget * widget, gpointer data)
{
	unsigned int * cnt = data;

	(*cnt)--;
	if(*cnt == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(gtk_widget_get_toplevel(widget));
}


/* usage */
static int _usage(void)
{
	fputs("Usage: properties file...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int ret;
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	ret = _properties(argc - optind, &argv[optind]) ? 0 : 2;
	gtk_main();
	return ret ? 0 : 2;
}
