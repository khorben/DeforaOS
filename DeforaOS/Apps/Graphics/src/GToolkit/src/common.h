/* $Id$ */



#ifndef GTOOLKIT_COMMON_H
# define GTOOLKIT_COMMON_H

# include <X11/Xlib.h>
# include <GL/glx.h>
# include <GToolkit.h>


/* types */
typedef struct _GToolkit
{
	/* memory */
	void ** alloced;
	size_t alloced_cnt;

	/* main loop */
	int loop;

	/* not portable */
	Display * display;
	int screen;
	XVisualInfo * visual;
} GToolkit;

/* variables */
extern GToolkit gt;


/* useful */
/* FIXME memory management useful for users of the library as well? */
void * g_alloc(size_t size);
void * g_alloced(void * ptr);
void g_free(void * ptr);

#endif
