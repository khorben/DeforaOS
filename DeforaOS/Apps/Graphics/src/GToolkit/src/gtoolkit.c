/* $Id$ */



#include <stdio.h>
#include <string.h>
#include "common.h"


/* GToolkit */
/* private */
/* variables */
static int _gt_init = 0; /* FIXME pointer(s) to function(s) instead? */


/* protected */
/* variables */
GToolkit gt;


/* public */
/* functions */
/* g_init */
int g_init(void)
{
	int attr[] =
	{
		GLX_RGBA, GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 4,
		GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4,
		GLX_DEPTH_SIZE, 16,
		None
	};

	if(_gt_init != 0)
		g_quit();
	if((gt.display = XOpenDisplay(NULL)) == NULL)
		return g_error("Could not open display", 1);
	gt.screen = DefaultScreen(gt.display);
	if((gt.visual = glXChooseVisual(gt.display, gt.screen, attr)) == NULL)
		return g_error("Could not choose visual", 1);
	gt.loop = 0;
	_gt_init = 1;
	return 0;
}


/* g_quit */
int g_quit(void)
{
	if(_gt_init == 0)
		return 0;
	XCloseDisplay(gt.display);
	memset(&gt, 0, sizeof(gt));
	_gt_init = 0;
	return 0;
}


/* useful */
int g_error(char const * message, int ret)
{
	fprintf(stderr, "%s%s\n", "GToolkit: ", message);
	return ret;
}


/* g_main */
int g_main(void)
{
	XEvent event;

	if(gt.loop != 0)
		return 1;
	for(gt.loop = 1; gt.loop == 1;)
	{
		while(XPending(gt.display) > 0)
		{
			XNextEvent(gt.display, &event);
			fprintf(stderr, "DEBUG: Event %d\n", event.type);
		}
	}
	gt.loop = 0;
	return 0;
}
