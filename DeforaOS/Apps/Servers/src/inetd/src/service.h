/* service.h */



#ifndef __SERVICE_H
# define __SERVICE_H

# include <sys/types.h>
# include <sys/socket.h>
# include <stdint.h>
# include <netinet/in.h>


/* Service */
/* types */
typedef enum _ServiceSocket { SS_TCP, SS_UDP } ServiceSocket;
typedef uint16_t ServiceProtocol;
typedef enum _ServiceWait { SW_WAIT, SW_NOWAIT } ServiceWait;
typedef struct _ServiceId
{
	uid_t uid;
	gid_t gid;
} ServiceId;

typedef struct _Service
{
	char * name;
	ServiceSocket socket;
	ServiceProtocol proto;
	ServiceWait wait;
	ServiceId id;
	char ** program;
} Service;


/* functions */
Service * service_new(char * name, ServiceSocket socket, ServiceProtocol proto,
		ServiceWait wait, ServiceId id, char ** program);
void service_delete(Service * service);

/* useful */
void service_exec(Service * s, int fd, struct sockaddr_in * addr, int addrlen);

#endif /* !__SERVICE_H */
