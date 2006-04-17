/* appclient.h */



#ifndef _APPCLIENT_H_
# define _APPCLIENT_H_

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

#endif /* !_APPCLIENT_H_ */
