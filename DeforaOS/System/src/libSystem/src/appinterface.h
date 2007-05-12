/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#ifndef LIBSYSTEM_APPINTERFACE_H
# define LIBSYSTEM_APPINTERFACE_H

# include <stdarg.h>


/* AppInterface */
/* types */
typedef struct _AppInterface AppInterface;


/* functions */
AppInterface * appinterface_new(char const * app);
AppInterface * appinterface_new_server(char const * app);
void appinterface_delete(AppInterface * appinterface);

/* accessors */
int appinterface_get_port(AppInterface * appinterface);
int appinterface_get_args_count(AppInterface * appinterface,
		char const * function);

/* useful */
int appinterface_call(AppInterface * appinterface, char buf[], size_t buflen,
		char const * function, void ** args, va_list arg);
int appinterface_call_receive(AppInterface * appinterface, char buf[],
		size_t buflen, int32_t * ret, char const * function,
		void ** args);
int appinterface_receive(AppInterface * appinterface, char buf[], size_t buflen,
		char bufw[], size_t bufwlen, size_t * bufwpos, int * ret);

#endif /* !LIBSYSTEM_APPINTERFACE_H */
