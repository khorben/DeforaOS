/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
/* TODO:
 * - attempt to open the video device at regular intervals
 * - display a window even if it was impossible to open a capture device */



#include <sys/ioctl.h>
#ifdef __NetBSD__
# include <sys/videoio.h>
#else
# include <linux/videodev2.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include <Desktop.h>
#include "Phone.h"


/* Video */
/* private */
/* types */
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;

	int fd;
	guint source;
	Buffer * buffer;

	/* widgets */
	GtkWidget * window;
	GtkWidget * area;
} VideoPhonePlugin;


/* prototypes */
/* plug-in */
static VideoPhonePlugin * _video_init(PhonePluginHelper * helper);
static void _video_destroy(VideoPhonePlugin * video);

/* useful */
static int _video_ioctl(VideoPhonePlugin * video, unsigned long request,
		void * data);

/* callbacks */
static gboolean _video_on_closex(gpointer data);
static gboolean _video_on_refresh(gpointer data);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"Video",
	"camera-video",
	NULL,
	_video_init,
	_video_destroy,
	NULL,
	NULL
};


/* private */
/* functions */
/* video_init */
static VideoPhonePlugin * _video_init(PhonePluginHelper * helper)
{
	VideoPhonePlugin * video;
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format format;

	if((video = object_new(sizeof(*video))) == NULL)
		return NULL;
	video->helper = helper;
	/* FIXME let this be configurable */
	video->fd = open("/dev/video0", O_RDWR);
	video->buffer = buffer_new(0, NULL);
	video->source = 0;
	video->window = NULL;
	/* check for errors */
	if(video->buffer == NULL
			|| video->fd < 0
			|| _video_ioctl(video, VIDIOC_QUERYCAP, &cap) == -1
			|| (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0
			/* FIXME also implement mmap() and streaming */
			|| (cap.capabilities & V4L2_CAP_READWRITE) == 0)
	{
		_video_destroy(video);
		return NULL;
	}
	/* reset cropping */
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(_video_ioctl(video, VIDIOC_CROPCAP, &cropcap) == 0)
	{
		/* reset to default */
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;
		if(_video_ioctl(video, VIDIOC_S_CROP, &crop) == -1
				&& errno == EINVAL)
			helper->error(helper->phone, "Cropping not supported",
					0);
	}
	/* obtain the current format */
	if(_video_ioctl(video, VIDIOC_G_FMT, &format) == -1
			|| buffer_set_size(video->buffer,
				format.fmt.pix.sizeimage) != 0)
	{
		_video_destroy(video);
		return NULL;
	}
	/* create the window */
	video->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(video->window), FALSE);
	g_signal_connect_swapped(video->window, "delete-event", G_CALLBACK(
				_video_on_closex), video);
	video->area = gtk_drawing_area_new();
	gtk_widget_set_size_request(video->area, format.fmt.pix.width,
			format.fmt.pix.height);
	gtk_container_add(GTK_CONTAINER(video->window), video->area);
	gtk_widget_show_all(video->window);
	video->source = g_timeout_add(1000, _video_on_refresh, video);
	return video;
}


/* video_destroy */
static void _video_destroy(VideoPhonePlugin * video)
{
	if(video->window != NULL)
		gtk_widget_destroy(video->window);
	if(video->source != 0)
		g_source_remove(video->source);
	if(video->fd >= 0)
		close(video->fd);
	buffer_delete(video->buffer);
	object_delete(video);
}


/* useful */
/* video_ioctl */
static int _video_ioctl(VideoPhonePlugin * video, unsigned long request,
		void * data)
{
	int ret;

	for(;;)
		if((ret = ioctl(video->fd, request, data)) != -1
				|| errno != EINTR)
			break;
	return ret;
}


/* callbacks */
/* video_on_closex */
static gboolean _video_on_closex(gpointer data)
{
	VideoPhonePlugin * video = data;

	gtk_widget_hide(video->window);
	if(video->source != 0)
		g_source_remove(video->source);
	video->source = 0;
	return TRUE;
}


/* video_on_refresh */
static gboolean _video_on_refresh(gpointer data)
{
	VideoPhonePlugin * video = data;

	/* FIXME no longer block on read() */
	if(read(video->fd, buffer_get_data(video->buffer),
				buffer_get_size(video->buffer)) <= 0)
	{
		/* this error can be ignored */
		if(errno == EAGAIN)
			return TRUE;
		close(video->fd);
		video->fd = -1;
		return FALSE;
	}
	return TRUE;
}
