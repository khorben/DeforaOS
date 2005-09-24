/* appinterface.h */



#ifndef APP_INTERFACE_H
# define APP_INTERFACE_H


/* AppInterface */
/* types */
typedef struct _AppInterface AppInterface;


/* functions */
AppInterface * appinterface_new(char const * app);
AppInterface * appinterface_new_server(char const * app);
void appinterface_delete(AppInterface * appinterface);

/* returns */
int appinterface_port(AppInterface * appinterface);

/* useful */
int appinterface_call(AppInterface * appinterface, char * call, char buf[],
		int buflen, void ** args);
int appinterface_receive(AppInterface * appinterface, char buf[], int buflen,
		int * ret);

#endif /* !APP_INTERFACE_H */
