/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Graphics GServer */



#ifndef GSERVER_GSERVER_H
# define GSERVER_GSERVER_H

# include <System.h>


/* GServer */
/* public */
/* types */
typedef struct _GServer GServer;


/* functions */
GServer * gserver_new(AppServerOptions options, Event * event);
void gserver_delete(GServer * gserver);


/* useful */
int gserver_loop(GServer * gserver);

#endif /* !GSERVER_GSERVER_H */
