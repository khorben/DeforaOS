/* service.c */



#include "inetd.h"
#include "service.h"


Service * service_new(char * name, ServiceSocket socket, ServiceProtocol proto,
		ServiceWait wait, ServiceId id, char ** program)
{
	Service * s;

	if((s = malloc(sizeof(Service))) == NULL)
		return NULL;
	s->name = strdup(name);
	s->socket = socket;
	s->protocol = proto;
	s->wait = wait;
	s->id = id;
	/* FIXME */
	s->program = NULL;
	return s;
}


void service_delete(Service * service)
{
	char * p;

	free(name);
	if(program != NULL)
	{
		free(program);
		for(p = *program; p != NULL; p++)
			free(p);
	}
	free(service);
}


/* useful */
void service_exec(Service * s, int fd, struct sockaddr_in * addr, int addrlen)
{
	if(close(0) != 0 || close(1) != 0
			|| dup2(fd, 0) != 0 || dup2(fd, 1) != 1)
	{
		inetd_error("dup2", 0);
		return;
	}
	execv(s->name, s->program);
	inetd_error(s->name);
	exit(2);
}
