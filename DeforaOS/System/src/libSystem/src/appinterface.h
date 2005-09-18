/* appinterface.h */



#ifndef APP_INTERFACE_H
# define APP_INTERFACE_H


/* AppInterface */
/* types */
typedef struct _AppInterface AppInterface;


/* functions */
AppInterface * appinterface_new(char const * app);
void appinterface_delete(AppInterface * appinterface);

/* useful */
int appinterface_call(AppInterface * appinterface, char * call, char buf[],
		int buflen, void ** args);

#endif /* !APP_INTERFACE_H */
