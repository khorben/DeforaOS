/* $Id$ */



#include <stdlib.h>
#include "common.h"
#include "GToolkit/GWidget.h"
#include "GToolkit/GWindow.h"


/* GWindow */
/* private */
/* types */
struct _GWindow
{
#include "gwidget.h"

	/* not portable */
	Window window;
	GLXContext context;
};


/* public */
/* functions */
/* gwindow_new */
GWindow * gwindow_new(void)
{
	GWindow * gwindow;
	XSetWindowAttributes attr;

	if((gwindow = g_alloc(sizeof(*gwindow))) == NULL)
		return NULL; /* FIXME report */
	/* FIXME colormap defined in g_init()? */
	attr.colormap = XCreateColormap(gt.display,
			RootWindow(gt.display, gt.visual->screen),
			gt.visual->visual, AllocNone);
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
		StructureNotifyMask;
	gwindow->window = XCreateWindow(gt.display,
			RootWindow(gt.display, gt.visual->screen), 0, 0,
			200, 200, 0, gt.visual->depth, InputOutput,
			gt.visual->visual,
			CWBorderPixel | CWColormap | CWEventMask, &attr);
	gwindow->context = glXCreateContext(gt.display, gt.visual, 0, GL_TRUE);
	return gwindow;
}


/* gwindow_delete */
void gwindow_delete(GWindow * gwindow)
{
	if(g_alloced(gwindow) != gwindow)
		return;
	glXDestroyContext(gt.display, gwindow->context);
	g_free(gwindow);
}


/* useful */
/* gwindow_show */
void gwindow_show(GWindow * gwindow)
	/* FIXME accept flags (focus...) */
{
	XMapRaised(gt.display, gwindow->window);
	glXMakeCurrent(gt.display, gwindow->window, gwindow->context);
}
