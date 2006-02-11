/* video/video.h */



#ifndef __VIDEO_VIDEO_H
# define __VIDEO_VIDEO_H


/* Video */
/* types */
typedef struct _VideoPlugin
{
	int (* init)(void);
	void (* destroy)(void);
} VideoPlugin;

#endif /* !__VIDEO_VIDEO_H */
