/* $Id$ */
static char _copyright[] =
"Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Mixer */
static char _license[] =
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
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "mixer.h"
#include "../config.h"


/* Mixer */
/* private */
/* types */
typedef struct _MixerClass
{
	int mixer_class;
	audio_mixer_name_t label;
	GtkWidget * hbox;
} MixerClass;

struct _Mixer
{
	int fd;

	/* widgets */
	GtkWidget * window;
	GtkWidget * properties;
	GtkWidget * about;

	/* internals */
	MixerClass * mc;
	size_t mc_cnt;
};


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static DesktopMenu _mixer_menu_file[] =
{
	{ "_Properties", G_CALLBACK(on_file_properties), GTK_STOCK_PROPERTIES,
		GDK_P },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _mixer_menu_view[] =
{
	{ "_All", G_CALLBACK(on_view_all), NULL, GDK_A },
	{ "_Outputs", G_CALLBACK(on_view_outputs), NULL, GDK_O },
	{ "_Inputs", G_CALLBACK(on_view_inputs), NULL, GDK_I },
	{ "_Record", G_CALLBACK(on_view_record), NULL, GDK_R },
	{ "Mo_nitor", G_CALLBACK(on_view_monitor), NULL, GDK_N },
	{ "_Equalization", G_CALLBACK(on_view_equalization), NULL, GDK_E },
	{ "Mi_x", G_CALLBACK(on_view_mix), NULL, GDK_X },
	{ "_Modem", G_CALLBACK(on_view_modem), NULL, GDK_M },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _mixer_menu_help[] =
{
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenubar _mixer_menubar[] =
{
	{ "_File", _mixer_menu_file },
	{ "_View", _mixer_menu_view },
	{ "_Help", _mixer_menu_help },
	{ NULL, NULL },
};


/* prototypes */
mixer_ctrl_t * _mixer_get(Mixer * mixer, int dev);


/* public */
/* mixer_new */
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e);
static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s);
static GtkWidget * _new_value(Mixer * mixer, int dev,
		struct audio_mixer_value * v);

Mixer * mixer_new(void)
{
	Mixer * mixer;
	GtkAccelGroup * group;
	GtkWidget * scrolled;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * hbox;
	GtkWidget * control;
	int i;
	mixer_devinfo_t md;
	MixerClass * p;
	size_t u;

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	mixer->fd = open("/dev/mixer", O_RDWR);
	mixer->window = NULL;
	mixer->properties = NULL;
	mixer->about = NULL;
	mixer->mc = NULL;
	mixer->mc_cnt = 0;
	if(mixer->fd < 0)
	{
		mixer_delete(mixer);
		return NULL;
	}
	/* widgets */
	group = gtk_accel_group_new();
	mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(mixer->window), group);
	gtk_window_set_default_size(GTK_WINDOW(mixer->window), 800, 200);
	gtk_window_set_title(GTK_WINDOW(mixer->window), PACKAGE);
	g_signal_connect(G_OBJECT(mixer->window), "delete-event", G_CALLBACK(
				on_closex), mixer);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
			GTK_SHADOW_NONE);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_mixer_menubar, mixer, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* classes */
	hbox = gtk_hbox_new(FALSE, 0);
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type != AUDIO_MIXER_CLASS)
			continue;
		if((p = realloc(mixer->mc, sizeof(*mixer->mc)
						* (mixer->mc_cnt + 1))) == NULL)
		{
			/* FIXME report error */
			mixer_delete(mixer);
			return NULL;
		}
		mixer->mc = p;
		p = &mixer->mc[mixer->mc_cnt++];
		p->mixer_class = md.mixer_class;
		memcpy(&p->label, &md.label, sizeof(md.label));
		p->hbox = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), p->hbox, FALSE, TRUE, 0);
	}
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled),
			hbox);
	/* controls */
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type == AUDIO_MIXER_CLASS)
			continue;
		for(u = 0; u < mixer->mc_cnt; u++)
			if(mixer->mc[u].mixer_class == md.mixer_class)
				break;
		if(u == mixer->mc_cnt)
			continue;
		hbox = mixer->mc[u].hbox;
		control = NULL;
		switch(md.type)
		{
			case AUDIO_MIXER_ENUM:
				control = _new_enum(mixer, i, &md.un.e);
				break;
			case AUDIO_MIXER_SET:
				control = _new_set(mixer, i, &md.un.s);
				break;
			case AUDIO_MIXER_VALUE:
				control = _new_value(mixer, i, &md.un.v);
				break;
		}
		if(control == NULL)
			continue;
		widget = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(widget), gtk_label_new(
					md.label.name), FALSE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(widget), control, TRUE, TRUE, 4);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mixer->window), vbox);
	gtk_widget_show_all(vbox);
	mixer_show_class(mixer, AudioCoutputs);
	gtk_widget_show(mixer->window);
	return mixer;
}

static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e)
{
	mixer_ctrl_t * p;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	GSList * group = NULL;

	if(e->num_mem <= 0)
		return NULL;
	if((p = _mixer_get(mixer, dev)) == NULL)
		return NULL;
	vbox = gtk_vbox_new(TRUE, 0);
	for(i = 0; i < e->num_mem; i++)
	{
		widget = gtk_radio_button_new_with_label(group,
				e->member[i].label.name);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
		if(p->un.ord == i)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", p);
		g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
					on_enum_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s)
{
	mixer_ctrl_t * p;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;

	if(s->num_mem <= 0)
		return NULL;
	if((p = _mixer_get(mixer, dev)) == NULL)
		return NULL;
	vbox = gtk_vbox_new(TRUE, 0);
	for(i = 0; i < s->num_mem; i++)
	{
		widget = gtk_check_button_new_with_label(
				s->member[i].label.name);
		if(p->un.mask & (1 << i))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", p);
		g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
					on_set_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_value(Mixer * mixer, int dev,
		struct audio_mixer_value * v)
{
	mixer_ctrl_t * p;
	GtkWidget * vbox;
	GtkWidget * hbox;
	int i;
	GtkWidget * widget;

	if(v->num_channels <= 0)
		return NULL;
	if((p = _mixer_get(mixer, dev)) == NULL)
		return NULL;
	hbox = gtk_hbox_new(TRUE, 0);
	for(i = 0; i < v->num_channels; i++)
	{
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.02);
		gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
		gtk_range_set_value(GTK_RANGE(widget),
				p->un.value.level[i] / 255.0);
		g_object_set_data(G_OBJECT(widget), "ctrl", p);
		g_object_set_data(G_OBJECT(widget), "channel",
				&p->un.value.level[i]);
		g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
					on_value_changed), mixer);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	}
	if(v->num_channels < 2)
		return hbox;
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	widget = gtk_toggle_button_new_with_label("Bind");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	return vbox;
}


/* mixer_delete */
void mixer_delete(Mixer * mixer)
{
	if(mixer->fd >= 0)
		close(mixer->fd);
	if(mixer->window != NULL)
		gtk_widget_destroy(mixer->window);
	free(mixer);
}


/* accessors */
/* mixer_set_enum */
int mixer_set_enum(Mixer * mixer, GtkWidget * widget)
{
	mixer_ctrl_t * p;
	GSList * group;
	int ord;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
	for(ord = 0; group != NULL; ord++)
		if(group->data == widget)
			break;
		else
			group = group->next;
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	if(group == NULL || p == NULL)
		return 1;
	p->un.ord = ord;
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) == 0)
		return 0;
	fprintf(stderr, "%s: %s: %s\n", PACKAGE, "AUDIO_MIXER_WRITE",
			strerror(errno));
	return 1;
}


/* mixer_set_value */
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value)
{
	mixer_ctrl_t * p;
	u_char * level;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%lf)\n", __func__, value);
#endif
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	level = g_object_get_data(G_OBJECT(widget), "channel");
	if(p == NULL || level == NULL)
		return 1;
	*level = value * 255;
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) == 0)
		return 0;
	fprintf(stderr, "%s: %s: %s\n", PACKAGE, "AUDIO_MIXER_WRITE",
			strerror(errno));
	return 1;
}


/* useful */
void mixer_about(Mixer * mixer)
{

	if(mixer->about != NULL)
	{
		gtk_widget_show(mixer->about);
		return;
	}
	mixer->about = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(mixer->about), GTK_WINDOW(
				mixer->window));
	desktop_about_dialog_set_authors(mixer->about, _authors);
	desktop_about_dialog_set_copyright(mixer->about, _copyright);
	desktop_about_dialog_set_license(mixer->about, _license);
	desktop_about_dialog_set_logo_icon_name(mixer->about, "gnome-mixer");
	desktop_about_dialog_set_name(mixer->about, PACKAGE);
	desktop_about_dialog_set_version(mixer->about, VERSION);
	gtk_widget_show(mixer->about);
}


/* mixer_properties */
void mixer_properties(Mixer * mixer)
{
	audio_device_t ad;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if(mixer->properties != NULL)
	{
		gtk_widget_show(mixer->properties);
		return;
	}
	if(ioctl(mixer->fd, AUDIO_GETDEV, &ad) != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", PACKAGE, "AUDIO_GETDEV",
				strerror(errno));
		return;
	}
	mixer->properties = gtk_dialog_new_with_buttons("Mixer properties",
			GTK_WINDOW(mixer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(mixer->properties));
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Name: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.name);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Version: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.version);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Config: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.config);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	g_signal_connect(mixer->properties, "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(mixer->properties);
}


/* mixer_show */
void mixer_show(Mixer * mixer, int view)
{
	size_t u;

	if(view < 0)
	{
		for(u = 0; u < mixer->mc_cnt; u++)
			gtk_widget_show(mixer->mc[u].hbox);
		return;
	}
	u = view;
	if(u >= mixer->mc_cnt)
		return;
	for(u = 0; u < mixer->mc_cnt; u++)
		gtk_widget_hide(mixer->mc[u].hbox);
	gtk_widget_show(mixer->mc[view].hbox);
}


/* mixer_show_all */
void mixer_show_all(Mixer * mixer)
{
	mixer_show(mixer, -1);
}


/* mixer_show_all */
void mixer_show_class(Mixer * mixer, char const * name)
{
	size_t u;

	for(u = 0; u < mixer->mc_cnt; u++)
		if(name == NULL || strcmp(mixer->mc[u].label.name, name) == 0)
			gtk_widget_show(mixer->mc[u].hbox);
		else
			gtk_widget_hide(mixer->mc[u].hbox);
}


/* private */
/* functions */
/* mixer_get */
mixer_ctrl_t * _mixer_get(Mixer * mixer, int dev)
{
	mixer_ctrl_t * p;
#ifdef DEBUG
	int i;
	size_t u;
	struct mixer_devinfo md;
	char * sep = "";
#endif

	if((p = malloc(sizeof(*p))) == NULL)
		return NULL;
	p->dev = dev;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, p) != 0)
	{
		fprintf(stderr, "%s: %s: %s\n", PACKAGE, "AUDIO_MIXER_READ",
				strerror(errno));
		free(p);
		return NULL;
	}
#ifdef DEBUG
	md.index = dev;
	if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) != 0)
		return p; /* XXX ignore error */
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].mixer_class == md.mixer_class)
			printf("%s", mixer->mc[u].label.name);
	printf(".%s=", md.label.name);
	switch(p->type)
	{
		case AUDIO_MIXER_ENUM:
			printf("%d", p->un.ord);
			break;
		case AUDIO_MIXER_SET:
			break;
		case AUDIO_MIXER_VALUE:
			for(i = 0; i < p->un.value.num_channels; i++)
			{
				printf("%s%u", sep, p->un.value.level[i]);
				sep = ",";
			}
			break;
	}
	putchar('\n');
#endif
	return p;
}
