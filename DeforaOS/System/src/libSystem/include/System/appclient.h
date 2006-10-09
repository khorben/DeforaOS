/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_APPCLIENT_H
# define LIBSYSTEM_APPCLIENT_H

# include "event.h"


/* AppClient */
/* types */
typedef struct _AppClient AppClient;


/* functions */
AppClient * appclient_new(char * service);
AppClient * appclient_new_event(char * service, Event * event);
void appclient_delete(AppClient * appclient);

/* useful */
int appclient_call(AppClient * appclient, char * function, int args_cnt, ...);

#endif /* !LIBSYSTEM_APPCLIENT_H */
