/* $Id$ */
static char _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
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
# include <sys/soundcard.h>
#endif
#include <fcntl.h>
#include <unistd.h>
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

struct _Mixer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * properties;
	GtkWidget * about;

	/* internals */
#ifdef AUDIO_MIXER_DEVINFO
	int fd;

	MixerClass * mc;
	size_t mc_cnt;
#else
	int fd;
#endif
};


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static DesktopMenu _mixer_menu_file[] =
{
	{ N_("_Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_Return },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _mixer_menu_view[] =
{
	{ N_("_All"), G_CALLBACK(on_view_all), NULL, GDK_CONTROL_MASK, GDK_A },
#ifdef AUDIO_MIXER_DEVINFO
	{ N_("_Outputs"), G_CALLBACK(on_view_outputs), NULL, GDK_CONTROL_MASK,
		GDK_O },
	{ N_("_Inputs"), G_CALLBACK(on_view_inputs), NULL, GDK_CONTROL_MASK,
		GDK_I },
	{ N_("_Record"), G_CALLBACK(on_view_record), NULL, GDK_CONTROL_MASK,
		GDK_R },
	{ N_("Mo_nitor"), G_CALLBACK(on_view_monitor), NULL, GDK_CONTROL_MASK,
		GDK_N },
	{ N_("_Equalization"), G_CALLBACK(on_view_equalization), NULL,
		GDK_CONTROL_MASK, GDK_E },
	{ N_("Mi_x"), G_CALLBACK(on_view_mix), NULL, GDK_CONTROL_MASK, GDK_X },
	{ N_("_Modem"), G_CALLBACK(on_view_modem), NULL, GDK_CONTROL_MASK,
		GDK_M },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _mixer_menu_help[] =
{
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _mixer_menubar[] =
{
	{ N_("_File"), _mixer_menu_file },
	{ N_("_View"), _mixer_menu_view },
	{ N_("_Help"), _mixer_menu_help },
	{ NULL, NULL },
};


/* prototypes */
static int _mixer_error(Mixer * mixer, char const * message, int ret);
#ifdef AUDIO_MIXER_DEVINFO
static mixer_ctrl_t * _mixer_get(Mixer * mixer, int dev);
#endif


/* public */
/* mixer_new */
#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e);
static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s);
static GtkWidget * _new_value(Mixer * mixer, int dev,
		struct audio_mixer_value * value);
#else
static GtkWidget * _new_value(Mixer * mixer, int dev, int value);
#endif

Mixer * mixer_new(char const * device, MixerOrientation orientation)
{
	Mixer * mixer;
	GtkAccelGroup * group;
	GtkWidget * scrolled;
	GtkWidget * vbox;
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
	char const * names[] = SOUND_DEVICE_LABELS;
#endif

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	if(device == NULL)
		device = "/dev/mixer";
	mixer->fd = open(device, O_RDWR);
	mixer->window = NULL;
	mixer->properties = NULL;
	mixer->about = NULL;
#ifdef AUDIO_MIXER_DEVINFO
	mixer->mc = NULL;
	mixer->mc_cnt = 0;
#endif
	if(mixer->fd < 0)
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
	gtk_window_set_icon_name(GTK_WINDOW(mixer->window), "gnome-mixer");
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
	/* menubar */
	widget = desktop_menubar_create(_mixer_menubar, mixer, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* classes */
	if(orientation == MO_VERTICAL)
		hvbox = gtk_vbox_new(TRUE, 0);
	else
		hvbox = gtk_hbox_new(FALSE, 0);
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
				control = _new_value(mixer, i, &md.un.v);
				break;
		}
		if(control == NULL)
			continue;
		widget = gtk_frame_new(md.label.name);
		gtk_container_add(GTK_CONTAINER(widget), control);
		if(hbox == NULL)
		{
			mixer->mc[u].hbox = gtk_hbox_new(FALSE, 0);
			hbox = mixer->mc[u].hbox;
			gtk_box_pack_start(GTK_BOX(hvbox), hbox, FALSE, TRUE,
					0);
		}
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
#else
		if(i == SOUND_MIXER_NONE)
			break;
		if(ioctl(mixer->fd, MIXER_READ(i), &value) < 0)
		{
			_mixer_error(NULL, "MIXER_READ", 0);
			continue;
		}
		control = _new_value(mixer, i, value);
		widget = gtk_frame_new(names[i]);
		gtk_container_add(GTK_CONTAINER(widget), control);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
#endif
	}
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mixer->window), vbox);
	gtk_widget_show_all(vbox);
#ifdef AUDIO_MIXER_DEVINFO
	mixer_show_class(mixer, AudioCoutputs);
#endif
	gtk_widget_show(mixer->window);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %p\n", __func__, (void*)mixer);
#endif
	return mixer;
}

#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e)
{
	mixer_ctrl_t * p;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	GSList * group = NULL;
	int * q;

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
		if((q = malloc(sizeof(*q))) != NULL)
		{
			*q = e->member[i].ord;
			g_object_set_data(G_OBJECT(widget), "ord", q);
		}
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
#endif

#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_value(Mixer * mixer, int dev,
		struct audio_mixer_value * value)
#else
static GtkWidget * _new_value(Mixer * mixer, int dev, int value)
#endif
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	int i;
	GtkWidget * widget;
	GtkWidget * bind;
	GSList * list = NULL;
	gdouble v;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	int num_channels = value->num_channels;
#else
	unsigned int * channel;
	const int num_channels = 2;
#endif

	if(num_channels <= 0)
		return NULL;
#ifdef AUDIO_MIXER_DEVINFO
	if((p = _mixer_get(mixer, dev)) == NULL)
		return NULL;
#endif
	hbox = gtk_hbox_new(TRUE, 0);
	bind = gtk_toggle_button_new_with_label(_("Bind"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bind), TRUE);
	for(i = 0; i < num_channels; i++)
	{
		widget = gtk_vscale_new_with_range(0.0, 100.0, 1.0);
		gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
#ifdef AUDIO_MIXER_DEVINFO
		v = (p->un.value.level[i] / 255.0) * 100.0;
#else
		v = ((value >> (i * 8)) & 0xff) + 0.0;
#endif
		gtk_range_set_value(GTK_RANGE(widget), v);
		g_object_set_data(G_OBJECT(widget), "bind", bind);
#ifdef AUDIO_MIXER_DEVINFO
		g_object_set_data(G_OBJECT(widget), "ctrl", p);
		g_object_set_data(G_OBJECT(widget), "channel",
				&p->un.value.level[i]);
#else
		if((channel = malloc(sizeof(*channel))) != NULL)
			*channel = dev;
		g_object_set_data(G_OBJECT(widget), "channel", channel);
#endif
		g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
					on_value_changed), mixer);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
		list = g_slist_append(list, widget);
	}
	g_object_set_data(G_OBJECT(bind), "list", list);
	if(num_channels < 2)
		return hbox;
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), bind, FALSE, TRUE, 0);
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
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	int * q;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void*)mixer,
			mixer->fd);
#endif
#ifdef AUDIO_MIXER_DEVINFO
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	q = g_object_get_data(G_OBJECT(widget), "ord");
	if(p == NULL || q == NULL)
		return 1;
	p->un.ord = *q;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d ord=%d\n", __func__, (void*)mixer,
			mixer->fd, p->un.ord);
#endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) == 0)
		return 0;
	return _mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	/* FIXME implement */
	return 0;
#endif
}


/* mixer_set_value */
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value)
{
	GtkWidget * b;
	int i;
	GSList * q;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	u_char * level;
#else
	unsigned int * channel;
	int level;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %lf) fd=%d\n", __func__, (void*)mixer,
			value, mixer->fd);
#endif
	b = g_object_get_data(G_OBJECT(widget), "bind");
#ifdef AUDIO_MIXER_DEVINFO
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	level = g_object_get_data(G_OBJECT(widget), "channel");
	if(p == NULL || level == NULL)
		return 1;
	/* FIXME check this one */
	*level = (value / 100.0) * 255;
	if(p->type == AUDIO_MIXER_VALUE && b != NULL
			&& gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)))
	{
		for(i = 0; i < p->un.value.num_channels; i++)
			p->un.value.level[i] = *level;
		for(q = g_object_get_data(G_OBJECT(b), "list"); q != NULL;
				q = q->next)
			gtk_range_set_value(GTK_RANGE(q->data), value);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d level=%u\n", __func__,
			(void*)mixer, mixer->fd, *level);
#endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) == 0)
		return 0;
	return _mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	channel = g_object_get_data(G_OBJECT(widget), "channel");
	if(b != NULL && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)))
	{
		for(i = 0, q = g_object_get_data(G_OBJECT(b), "list");
				q != NULL; i++, q = q->next)
		{
			level |= (int)((value / 100.0) * 127) << (i * 8);
			gtk_range_set_value(GTK_RANGE(q->data), value);
		}
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() level=0x%x\n", __func__, level);
#endif
		if(ioctl(mixer->fd, MIXER_WRITE(*channel), &level) != 0)
			return _mixer_error(mixer, "MIXER_WRITE", 1);
	}
	return 0;
#endif
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
	desktop_about_dialog_set_logo_icon_name(mixer->about, "gnome-mixer");
	desktop_about_dialog_set_name(mixer->about, PACKAGE);
	desktop_about_dialog_set_version(mixer->about, VERSION);
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
#ifdef AUDIO_MIXER_DEVINFO
	audio_device_t ad;
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
#endif

	if(mixer->properties != NULL)
	{
		gtk_widget_show(mixer->properties);
		return;
	}
#ifdef AUDIO_MIXER_DEVINFO
	if(ioctl(mixer->fd, AUDIO_GETDEV, &ad) != 0)
	{
		_mixer_error(mixer, "AUDIO_GETDEV", 1);
		return;
	}
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
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Name: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.name);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Version: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.version);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Config: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(ad.config);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	gtk_widget_show_all(vbox);
	gtk_widget_show(mixer->properties);
#endif
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


/* mixer_show_all */
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


#ifdef AUDIO_MIXER_DEVINFO
/* mixer_get */
static mixer_ctrl_t * _mixer_get(Mixer * mixer, int dev)
{
	mixer_ctrl_t * p;
	struct mixer_devinfo md;
#ifdef DEBUG
	int i;
	size_t u;
	char * sep = "";
#endif

	md.index = dev;
	if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) != 0)
		return NULL;
	if((p = malloc(sizeof(*p))) == NULL)
		return NULL;
	p->dev = dev;
	/* XXX this is necessary for some drivers and I don't like it */
	if((p->type = md.type) == AUDIO_MIXER_VALUE)
		p->un.value.num_channels = md.un.v.num_channels;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, p) != 0)
	{
		_mixer_error(mixer, "AUDIO_MIXER_READ", 0);
		free(p);
		return NULL;
	}
#ifdef DEBUG
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
#endif
