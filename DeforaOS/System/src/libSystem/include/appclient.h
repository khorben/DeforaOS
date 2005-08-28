/* appclient.h */



#ifndef _APPCLIENT_H
# define _APPCLIENT_H

# include "event.h"


/* AppClient */
/* types */
typedef struct _AppClient AppClient;


/* functions */
AppClient * appclient_new(void);
AppClient * appclient_new_event(Event * event);
void appclient_delete(AppClient * appclient);

#endif /* !_APPCLIENT_H */
