/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <System.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* CVS */
/* private */
/* types */
typedef struct _CVS
{
	char * filename;

	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * name;
	GtkWidget * status;
	/* directory */
	GtkWidget * directory;
	GtkWidget * d_root;
	GtkWidget * d_repository;
	/* file */
	GtkWidget * file;
} CVS;


/* prototypes */
static GtkWidget * _cvs_init(BrowserPlugin * plugin);
static void _cvs_destroy(BrowserPlugin * plugin);
static void _cvs_refresh(BrowserPlugin * plugin, char const * path);


/* public */
/* variables */
/* plug-in */
BrowserPlugin plugin =
{
	NULL,
	N_("CVS"),
	"applications-development",
	_cvs_init,
	_cvs_destroy,
	_cvs_refresh,
	NULL
};


/* private */
/* functions */
static GtkWidget * _init_label(GtkSizeGroup * group, char const * label,
		GtkWidget ** widget);

static GtkWidget * _cvs_init(BrowserPlugin * plugin)
{
	CVS * cvs;
	PangoFontDescription * font;
	GtkSizeGroup * group;
	GtkWidget * widget;

	if((cvs = object_new(sizeof(*cvs))) == NULL)
		return NULL;
	plugin->priv = cvs;
	cvs->filename = NULL;
	cvs->widget = gtk_vbox_new(FALSE, 4);
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* label */
	cvs->name = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(cvs->name), 0.0, 0.5);
	gtk_widget_modify_font(cvs->name, font);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->name, FALSE, TRUE, 0);
	cvs->status = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(cvs->status), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->status, FALSE, TRUE, 0);
	/* directory */
	cvs->directory = gtk_vbox_new(FALSE, 4);
	widget = _init_label(group, _("Root:"), &cvs->d_root);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	widget = _init_label(group, _("Repository:"), &cvs->d_repository);
	gtk_box_pack_start(GTK_BOX(cvs->directory), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(cvs->directory);
	gtk_widget_set_no_show_all(cvs->directory, TRUE);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->directory, FALSE, TRUE,
			0);
	/* file */
	cvs->file = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_no_show_all(cvs->file, TRUE);
	gtk_box_pack_start(GTK_BOX(cvs->widget), cvs->file, FALSE, TRUE, 0);
	gtk_widget_show_all(cvs->widget);
	cvs->source = 0;
	pango_font_description_free(font);
	return cvs->widget;
}

static GtkWidget * _init_label(GtkSizeGroup * group, char const * label,
		GtkWidget ** widget)
{
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 4);
	*widget = gtk_label_new(label);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, *widget);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, FALSE, TRUE, 0);
	*widget = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(*widget), PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, TRUE, TRUE, 0);
	return hbox;
}


static void _cvs_destroy(BrowserPlugin * plugin)
{
	CVS * cvs = plugin->priv;

	if(cvs->source != 0)
		g_source_remove(cvs->source);
	object_delete(cvs);
}


/* cvs_refresh */
static void _refresh_dir(CVS * cvs, struct stat * st);
static void _refresh_file(CVS * cvs);
static void _refresh_status(CVS * cvs, char const * status);

static void _cvs_refresh(BrowserPlugin * plugin, char const * path)
{
	CVS * cvs = plugin->priv;
	struct stat st;
	gchar * p;

	if(cvs->source != 0)
		g_source_remove(cvs->source);
	free(cvs->filename);
	cvs->filename = NULL;
	if(lstat(path, &st) != 0)
		return;
	if((cvs->filename = strdup(path)) == NULL)
		return;
	p = g_filename_display_basename(path);
	gtk_label_set_text(GTK_LABEL(cvs->name), p);
	g_free(p);
	_refresh_status(cvs, NULL);
	gtk_widget_hide(cvs->directory);
	gtk_widget_hide(cvs->file);
	if(S_ISDIR(st.st_mode))
		_refresh_dir(cvs, &st);
	else
		_refresh_file(cvs);
}

static void _refresh_dir(CVS * cvs, struct stat * st)
{
	char const dir[] = "CVS";
	char const root[] = "CVS/Root";
	char const repository[] = "CVS/Repository";
	size_t len = strlen(cvs->filename);
	char * p;
	gchar * q;

	gtk_label_set_text(GTK_LABEL(cvs->d_root), NULL);
	gtk_label_set_text(GTK_LABEL(cvs->d_repository), NULL);
	len = strlen(cvs->filename) + sizeof(dir) + 1;
	if((p = malloc(len)) != NULL)
	{
		snprintf(p, len, "%s/%s", cvs->filename, dir);
		if(lstat(p, st) != 0)
		{
			_refresh_status(cvs, _("Not a CVS repository"));
			free(p);
			return;
		}
	}
	gtk_widget_show(cvs->directory);
	len = strlen(cvs->filename) + sizeof(root) + 1;
	if((p = realloc(p, len)) != NULL)
	{
		snprintf(p, len, "%s/%s", cvs->filename, root);
		if(g_file_get_contents(p, &q, NULL, NULL) == TRUE)
		{
			gtk_label_set_text(GTK_LABEL(cvs->d_root), q);
			g_free(q);
		}
	}
	len = strlen(cvs->filename) + sizeof(repository) + 1;
	if((p = realloc(p, len)) != NULL)
	{
		snprintf(p, len, "%s/%s", cvs->filename, repository);
		if(g_file_get_contents(p, &q, NULL, NULL) == TRUE)
		{
			gtk_label_set_text(GTK_LABEL(cvs->d_repository), q);
			g_free(q);
		}
	}
	free(p);
}

static void _refresh_file(CVS * cvs)
{
	/* FIXME implement */
	gtk_widget_show(cvs->file);
}

static void _refresh_status(CVS * cvs, char const * status)
{
	if(status == NULL)
		status = "";
	gtk_label_set_text(GTK_LABEL(cvs->status), status);
}
