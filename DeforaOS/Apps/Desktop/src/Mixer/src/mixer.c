/* $Id$ */
static char _copyright[] =
"Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>";
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



#if defined(__NetBSD__) || defined(__sun__)
# include <sys/ioctl.h>
# include <sys/audioio.h>
#else
# include <sys/ioctl.h>
# include <sys/soundcard.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "mixer.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Mixer */
/* private */
/* types */
#ifdef AUDIO_MIXER_DEVINFO
typedef struct _MixerClass
{
	int mixer_class;
	audio_mixer_name_t label;
	GtkWidget * hbox;
} MixerClass;
#endif

typedef struct _MixerLevel
{
	uint8_t channels[8];
	size_t channels_cnt;
} MixerLevel;

typedef struct _MixerControl
{
	int index;
	int type;
	union {
		int ord;
		int mask;
		MixerLevel level;
	} un;
} MixerControl;

typedef struct _MixerProperties
{
	char name[32];
	char version[16];
	char device[16];
} MixerProperties;

struct _Mixer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * properties;
	GtkWidget * about;

	/* internals */
	char * device;
#ifdef AUDIO_MIXER_DEVINFO
	int fd;

	MixerClass * mc;
	size_t mc_cnt;
#else
	int fd;
#endif
};


/* constants */
#define MIXER_DEFAULT_DEVICE "/dev/mixer"


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

#ifdef EMBEDDED
static const DesktopAccel _mixer_accel[] =
{
	{ G_CALLBACK(on_file_properties), GDK_MOD1_MASK, GDK_KEY_Return },
	{ G_CALLBACK(on_file_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(on_view_all), GDK_CONTROL_MASK, GDK_KEY_A },
# ifdef AUDIO_MIXER_DEVINFO
	{ G_CALLBACK(on_view_outputs), GDK_CONTROL_MASK, GDK_KEY_O },
	{ G_CALLBACK(on_view_inputs), GDK_CONTROL_MASK, GDK_KEY_I },
	{ G_CALLBACK(on_view_record), GDK_CONTROL_MASK, GDK_KEY_R },
	{ G_CALLBACK(on_view_monitor), GDK_CONTROL_MASK, GDK_KEY_N },
	{ G_CALLBACK(on_view_equalization), GDK_CONTROL_MASK, GDK_KEY_E },
	{ G_CALLBACK(on_view_mix), GDK_CONTROL_MASK, GDK_KEY_X },
	{ G_CALLBACK(on_view_modem), GDK_CONTROL_MASK, GDK_KEY_M },
# endif
	{ NULL, 0, 0 }
};
#endif

#ifndef EMBEDDED
static const DesktopMenu _mixer_menu_file[] =
{
	{ N_("_Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _mixer_menu_view[] =
{
	{ N_("_All"), G_CALLBACK(on_view_all), NULL, GDK_CONTROL_MASK,
		GDK_KEY_A },
# ifdef AUDIO_MIXER_DEVINFO
	{ N_("_Outputs"), G_CALLBACK(on_view_outputs), NULL, GDK_CONTROL_MASK,
		GDK_KEY_O },
	{ N_("_Inputs"), G_CALLBACK(on_view_inputs), NULL, GDK_CONTROL_MASK,
		GDK_KEY_I },
	{ N_("_Record"), G_CALLBACK(on_view_record), NULL, GDK_CONTROL_MASK,
		GDK_KEY_R },
	{ N_("Mo_nitor"), G_CALLBACK(on_view_monitor), NULL, GDK_CONTROL_MASK,
		GDK_KEY_N },
	{ N_("_Equalization"), G_CALLBACK(on_view_equalization), NULL,
		GDK_CONTROL_MASK, GDK_KEY_E },
	{ N_("Mi_x"), G_CALLBACK(on_view_mix), NULL, GDK_CONTROL_MASK,
		GDK_KEY_X },
	{ N_("_Modem"), G_CALLBACK(on_view_modem), NULL, GDK_CONTROL_MASK,
		GDK_KEY_M },
# endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _mixer_menu_help[] =
{
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenubar _mixer_menubar[] =
{
	{ N_("_File"), _mixer_menu_file },
	{ N_("_View"), _mixer_menu_view },
	{ N_("_Help"), _mixer_menu_help },
	{ NULL, NULL },
};
#endif


/* prototypes */
static int _mixer_error(Mixer * mixer, char const * message, int ret);
static int _mixer_get_control(Mixer * mixer, int index, MixerControl * control);
static int _mixer_get_properties(Mixer * mixer, MixerProperties * properties);
static int _mixer_set_control(Mixer * mixer, int index, MixerControl * control);


/* public */
/* mixer_new */
static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label);
#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e);
static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s);
#endif
static GtkWidget * _new_value(Mixer * mixer, int index);

Mixer * mixer_new(char const * device, MixerOrientation orientation)
{
	Mixer * mixer;
	GtkAccelGroup * group;
	GtkWidget * scrolled;
	GtkWidget * vbox;
	GtkWidget * label;
	GtkWidget * widget;
	GtkWidget * hvbox;
	GtkWidget * hbox;
	GtkWidget * control;
	int i;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_devinfo_t md;
	MixerClass * p;
	size_t u;
#else
	int value;
	char const * labels[] = SOUND_DEVICE_LABELS;
	char const * names[] = SOUND_DEVICE_NAMES;
#endif

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	if(device == NULL)
		device = MIXER_DEFAULT_DEVICE;
	mixer->device = strdup(device);
	mixer->fd = open(device, O_RDWR);
	mixer->window = NULL;
	mixer->properties = NULL;
	mixer->about = NULL;
#ifdef AUDIO_MIXER_DEVINFO
	mixer->mc = NULL;
	mixer->mc_cnt = 0;
#endif
	if(mixer->device == NULL || mixer->fd < 0)
	{
		_mixer_error(NULL, device, 0);
		mixer_delete(mixer);
		return NULL;
	}
	/* widgets */
	group = gtk_accel_group_new();
	mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(mixer->window), group);
	gtk_window_set_default_size(GTK_WINDOW(mixer->window), 800, 200);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(mixer->window), "stock_volume");
#endif
	gtk_window_set_title(GTK_WINDOW(mixer->window), PACKAGE);
	g_signal_connect_swapped(G_OBJECT(mixer->window), "delete-event",
			G_CALLBACK(on_closex), mixer);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, orientation == MO_VERTICAL
			? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
			GTK_SHADOW_NONE);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	widget = desktop_menubar_create(_mixer_menubar, mixer, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
#else
	desktop_accel_create(_mixer_accel, mixer, group);
#endif
	/* classes */
	if(orientation == MO_VERTICAL)
		hvbox = gtk_vbox_new(TRUE, 4);
	else
		hvbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hvbox), 4);
	for(i = 0;; i++)
	{
#ifdef AUDIO_MIXER_DEVINFO
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
		p->hbox = NULL;
#else
		hbox = gtk_hbox_new(FALSE, 4);
		gtk_box_pack_start(GTK_BOX(hvbox), hbox, FALSE, TRUE, 0);
		break;
#endif
	}
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled),
			hvbox);
	/* controls */
	for(i = 0;; i++)
	{
#ifdef AUDIO_MIXER_DEVINFO
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
				control = _new_value(mixer, i);
				break;
		}
		if(control == NULL)
			continue;
		gtk_container_set_border_width(GTK_CONTAINER(control), 4);
		label = _new_frame_label(NULL, md.label.name, NULL);
		widget = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(widget), label);
		gtk_container_add(GTK_CONTAINER(widget), control);
		if(hbox == NULL)
		{
			mixer->mc[u].hbox = gtk_hbox_new(FALSE, 4);
			hbox = mixer->mc[u].hbox;
			gtk_box_pack_start(GTK_BOX(hvbox), hbox, FALSE, TRUE,
					0);
		}
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
#else
		if(i == SOUND_MIXER_NONE)
			break;
		if(ioctl(mixer->fd, MIXER_READ(i), &value) != 0)
			continue;
		control = _new_value(mixer, i);
		gtk_container_set_border_width(GTK_CONTAINER(control), 4);
		label = _new_frame_label(NULL, names[i], labels[i]);
		widget = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(widget), label);
		gtk_container_add(GTK_CONTAINER(widget), control);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
#endif
	}
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mixer->window), vbox);
	gtk_widget_show_all(vbox);
#ifdef AUDIO_MIXER_DEVINFO
	mixer_show_class(mixer, AudioCoutputs);
#endif
	gtk_widget_show(mixer->window);
	return mixer;
}

static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label)
{
	GtkIconTheme * icontheme;
	GtkWidget * hbox;
	GtkWidget * widget;
	struct
	{
		char const * name;
		char const * icon;
	} icons[] = {
		{ "cd",		"media-cdrom"		},
		{ "line",	"stock_line-in"		},
		{ "master",	"audio-volume-high"	},
		{ "mic",	"audio-input-microphone"},
		{ "pcm",	"audio-volume-high"	},
		{ "rec",	"gtk-media-record"	},
		{ "source",	"stock_line-in"		},
		{ "vol",	"audio-volume-high"	}
	};
	size_t i;

	icontheme = gtk_icon_theme_get_default();
	hbox = gtk_hbox_new(FALSE, 4);
	for(i = 0; pixbuf == NULL && i < sizeof(icons) / sizeof(*icons); i++)
		if(strncmp(icons[i].name, name, strlen(icons[i].name)) == 0)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					icons[i].icon, 16,
#if GTK_CHECK_VERSION(2, 12, 0)
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
#else
					0, NULL);
#endif
	if(pixbuf == NULL)
	{
		/* more generic fallbacks */
		if(strstr(name, "sel") != NULL)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					"stock_line-in", 16,
#if GTK_CHECK_VERSION(2, 12, 0)
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
#else
					0, NULL);
#endif
		else if(strstr(name, ".mute") != NULL)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					"audio-volume-muted", 16,
#if GTK_CHECK_VERSION(2, 12, 0)
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
#else
					0, NULL);
#endif
	}
	if(pixbuf != NULL)
	{
		widget = gtk_image_new_from_pixbuf(pixbuf);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	}
	if(label == NULL)
		label = name;
	widget = gtk_label_new(label);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e)
{
	MixerControl * mc;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	GSList * group = NULL;
	int * q;

	if(e->num_mem <= 0 || (mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, dev, mc) != 0)
	{
		free(mc);
		return NULL;
	}
	vbox = gtk_vbox_new(TRUE, 0);
	for(i = 0; i < e->num_mem; i++)
	{
		widget = gtk_radio_button_new_with_label(group,
				e->member[i].label.name);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
		if(mc->un.ord == i)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		if((q = malloc(sizeof(*q))) != NULL)
		{
			*q = e->member[i].ord;
			g_object_set_data(G_OBJECT(widget), "ord", mc);
		}
		g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
					on_enum_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s)
{
	MixerControl * mc;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;

	if(s->num_mem <= 0 || (mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, dev, mc) != 0)
	{
		free(mc);
		return NULL;
	}
	vbox = gtk_vbox_new(TRUE, 0);
	for(i = 0; i < s->num_mem; i++)
	{
		widget = gtk_check_button_new_with_label(
				s->member[i].label.name);
		if(mc->un.mask & (1 << i))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(
					on_set_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	return vbox;
}
#endif

static GtkWidget * _new_value(Mixer * mixer, int index)
{
	GtkWidget * align;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * bind;
	GSList * list = NULL;
	size_t i;
	gdouble v;
	MixerControl * mc;

	if((mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, index, mc) != 0
			|| mc->un.level.channels_cnt <= 0)
	{
		free(mc);
		return NULL;
	}
	hbox = gtk_hbox_new(FALSE, 0);
	bind = gtk_toggle_button_new_with_label(_("Bind"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bind), TRUE);
	for(i = 0; i < mc->un.level.channels_cnt; i++)
	{
		widget = gtk_vscale_new_with_range(0.0, 100.0, 1.0);
		gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
		v = (mc->un.level.channels[i] / 255.0) * 100.0;
		gtk_range_set_value(GTK_RANGE(widget), v);
		g_object_set_data(G_OBJECT(widget), "bind", bind);
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		g_object_set_data(G_OBJECT(widget), "channel",
				&mc->un.level.channels[i]);
		g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
					on_value_changed), mixer);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		list = g_slist_append(list, widget);
	}
	g_object_set_data(G_OBJECT(bind), "list", list);
	if(mc->un.level.channels_cnt < 2)
		return hbox;
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), bind, FALSE, TRUE, 0);
	align = gtk_alignment_new(0.5, 0.5, 0.0, 1.0);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	return align;
}


/* mixer_delete */
void mixer_delete(Mixer * mixer)
{
	if(mixer->fd >= 0)
		close(mixer->fd);
	if(mixer->device != NULL)
		free(mixer->device);
	if(mixer->window != NULL)
		gtk_widget_destroy(mixer->window);
	free(mixer);
}


/* accessors */
/* mixer_set_enum */
int mixer_set_enum(Mixer * mixer, GtkWidget * widget)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	int * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void*)mixer,
			mixer->fd);
#endif
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	q = g_object_get_data(G_OBJECT(widget), "ord");
	if(p == NULL || q == NULL)
		return 1;
	p->un.ord = *q;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d ord=%d\n", __func__, (void*)mixer,
			mixer->fd, p->un.ord);
#endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	/* FIXME implement */
#endif
	return 0;
}


/* mixer_set_value */
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value)
{
	GtkWidget * b;
	MixerControl * mc;
	uint8_t * channel;
	size_t i;
	GSList * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %lf) fd=%d\n", __func__, (void*)mixer,
			value, mixer->fd);
#endif
	b = g_object_get_data(G_OBJECT(widget), "bind");
	mc = g_object_get_data(G_OBJECT(widget), "ctrl");
	channel = g_object_get_data(G_OBJECT(widget), "channel");
	if(mc == NULL || channel == NULL)
		return 1;
	*channel = (value / 100.0) * 255;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)))
	{
		for(i = 0; i < mc->un.level.channels_cnt; i++)
			mc->un.level.channels[i] = *channel;
		for(q = g_object_get_data(G_OBJECT(b), "list"); q != NULL;
				q = q->next)
			gtk_range_set_value(GTK_RANGE(q->data), value);
	}
	return _mixer_set_control(mixer, mc->index, mc);
}


/* useful */
/* mixer_about */
static gboolean _about_on_closex(GtkWidget * widget);

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
	g_signal_connect(G_OBJECT(mixer->about), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	desktop_about_dialog_set_authors(mixer->about, _authors);
	desktop_about_dialog_set_copyright(mixer->about, _copyright);
	desktop_about_dialog_set_license(mixer->about, _license);
	desktop_about_dialog_set_logo_icon_name(mixer->about, "stock_volume");
	desktop_about_dialog_set_name(mixer->about, PACKAGE);
	desktop_about_dialog_set_translator_credits(mixer->about,
			_("translator-credits"));
	desktop_about_dialog_set_version(mixer->about, VERSION);
	desktop_about_dialog_set_website(mixer->about,
			"http://www.defora.org/");
	gtk_widget_show(mixer->about);
}

static gboolean _about_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* mixer_properties */
static gboolean _properties_on_closex(GtkWidget * widget);

void mixer_properties(Mixer * mixer)
{
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	MixerProperties mp;

	if(mixer->properties != NULL)
	{
		gtk_widget_show(mixer->properties);
		return;
	}
	if(_mixer_get_properties(mixer, &mp) != 0)
		return;
	mixer->properties = gtk_dialog_new_with_buttons(_("Mixer properties"),
			GTK_WINDOW(mixer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(mixer->properties, "delete-event", G_CALLBACK(
				_properties_on_closex), NULL);
	g_signal_connect(mixer->properties, "response", G_CALLBACK(
				gtk_widget_hide), NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(mixer->properties));
#else
	vbox = GTK_DIALOG(mixer->properties)->vbox;
#endif
	left = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	right = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Name: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(mp.name);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Version: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(mp.version);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Device: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(mp.device);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	gtk_widget_show_all(vbox);
	gtk_widget_show(mixer->properties);
}

static gboolean _properties_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* mixer_show */
void mixer_show(Mixer * mixer, int view)
{
#ifdef AUDIO_MIXER_DEVINFO
	size_t u;

	if(view < 0)
	{
		for(u = 0; u < mixer->mc_cnt; u++)
			if(mixer->mc[u].hbox != NULL)
				gtk_widget_show(mixer->mc[u].hbox);
		return;
	}
	u = view;
	if(u >= mixer->mc_cnt)
		return;
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].hbox == NULL)
			continue;
		else if(u == (size_t)view)
			gtk_widget_show(mixer->mc[u].hbox);
		else
			gtk_widget_hide(mixer->mc[u].hbox);
#endif
}


/* mixer_show_all */
void mixer_show_all(Mixer * mixer)
{
	mixer_show(mixer, -1);
}


/* mixer_show_class */
void mixer_show_class(Mixer * mixer, char const * name)
{
#ifdef AUDIO_MIXER_DEVINFO
	size_t u;

	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].hbox == NULL)
			continue;
		else if(name == NULL
				|| strcmp(mixer->mc[u].label.name, name) == 0)
			gtk_widget_show(mixer->mc[u].hbox);
		else
			gtk_widget_hide(mixer->mc[u].hbox);
#endif
}


/* private */
/* functions */
/* mixer_error */
static int _error_text(char const * message, int ret);

static int _mixer_error(Mixer * mixer, char const * message, int ret)
{
	GtkWidget * dialog;
	char const * error;

	if(mixer == NULL)
		return _error_text(message, ret);
	error = strerror(errno);
	dialog = gtk_message_dialog_new(GTK_WINDOW(mixer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message,
#endif
			error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* mixer_get_control */
static int _mixer_get_control(Mixer * mixer, int index, MixerControl * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	struct mixer_devinfo md;
	int i;
# ifdef DEBUG
	size_t u;
	char * sep = "";
# endif

	md.index = index;
	if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_DEVINFO", 1);
	p.dev = index;
	/* XXX this is necessary for some drivers and I don't like it */
	if((p.type = md.type) == AUDIO_MIXER_VALUE)
		p.un.value.num_channels = md.un.v.num_channels;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_READ", 1);
	control->index = index;
	control->type = p.type;
# ifdef DEBUG
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].mixer_class == md.mixer_class)
			printf("%s", mixer->mc[u].label.name);
	printf(".%s=", md.label.name);
# endif
	switch(p.type)
	{
		case AUDIO_MIXER_ENUM:
			control->un.ord = p.un.ord;
# ifdef DEBUG
			printf("%d", p->un.ord);
# endif
			break;
		case AUDIO_MIXER_SET:
			control->un.mask = p.un.mask;
			break;
		case AUDIO_MIXER_VALUE:
			control->un.level.channels_cnt
				= p.un.value.num_channels;
			for(i = 0; i < p.un.value.num_channels; i++)
			{
				control->un.level.channels[i]
					= p.un.value.level[i];
# ifdef DEBUG
				printf("%s%u", sep, p.un.value.level[i]);
				sep = ",";
# endif
			}
			break;
	}
# ifdef DEBUG
	putchar('\n');
# endif
#else
	int value;

	control->index = index;
	if(ioctl(mixer->fd, MIXER_READ(index), &value) != 0)
		return -_mixer_error(NULL, "MIXER_READ", 1);
	control->type = 0;
	control->un.level.channels_cnt = 2;
	control->un.level.channels[0] = ((value & 0xff) * 255) / 100;
	control->un.level.channels[1] = (((value >> 8) & 0xff) * 255) / 100;
	return 0;
#endif
	return 0;
}


/* mixer_get_properties */
static int _mixer_get_properties(Mixer * mixer, MixerProperties * properties)
{
#ifdef AUDIO_MIXER_DEVINFO
	audio_device_t ad;

	if(ioctl(mixer->fd, AUDIO_GETDEV, &ad) != 0)
		return -_mixer_error(mixer, "AUDIO_GETDEV", 1);
	snprintf(properties->name, sizeof(properties->name), "%s", ad.name);
	snprintf(properties->version, sizeof(properties->version), "%s",
			ad.version);
	snprintf(properties->device, sizeof(properties->device), "%s",
			ad.config);
#else
	struct mixer_info mi;
	int version;

	if(ioctl(mixer->fd, SOUND_MIXER_INFO, &mi) != 0)
		return -_mixer_error(mixer, "SOUND_MIXER_INFO", 1);
	if(ioctl(mixer->fd, OSS_GETVERSION, &version) != 0)
		return -_mixer_error(mixer, "OSS_GETVERSION", 1);
	snprintf(properties->name, sizeof(properties->name), "%s", mi.name);
	snprintf(properties->version, sizeof(properties->version), "%u.%u",
			(version >> 16) & 0xffff, version & 0xffff);
	snprintf(properties->device, sizeof(properties->device), "%s",
			mixer->device);
#endif
	return 0;
}


/* mixer_set_control */
static int _mixer_set_control(Mixer * mixer, int index, MixerControl * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	int i;

	p.dev = index;
	p.type = control->type;
	p.un.value.num_channels = control->un.level.channels_cnt;
	for(i = 0; i < p.un.value.num_channels; i++)
		p.un.value.level[i] = control->un.level.channels[i];
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	int level = 0;

	level |= (control->un.level.channels[0] * 100) / 255;
	level |= ((control->un.level.channels[1] * 100) / 255) << 8;
	if(ioctl(mixer->fd, MIXER_WRITE(index), &level) != 0)
		return -_mixer_error(mixer, "MIXER_WRITE", 1);
#endif
	return 0;
}
