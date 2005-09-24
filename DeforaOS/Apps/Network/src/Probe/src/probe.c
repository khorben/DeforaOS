/* probe.c */



#include <System.h>
#include <sys/sysinfo.h>
#include <stdio.h>

#define PROBE_REFRESH 5


/* globals */
struct sysinfo info;


/* Probe */
static int _probe_error(char * message, int ret);
static int _probe_timeout(struct sysinfo * info);
static int _probe(void)
{
	AppServer * appserver;
	Event * event;
	struct timeval tv;

	if(sysinfo(&info) != 0)
		return _probe_error("sysinfo", 2);
	if((event = event_new()) == NULL)
		return _probe_error("Event", 2);
	if((appserver = appserver_new_event("Probe", ASO_REMOTE, event))
			== NULL)
	{
		event_delete(event);
		return _probe_error("AppServer", 2);
	}
	tv.tv_sec = PROBE_REFRESH;
	tv.tv_usec = 0;
	if(event_register_timeout(event, tv, (EventTimeoutFunc)_probe_timeout,
			&info) != 0)
		_probe_error("timeout", 0);
	else
		event_loop(event);
	appserver_delete(appserver);
	event_delete(event);
	return 2;
}

static int _probe_error(char * message, int ret)
{
	fprintf(stderr, "%s", "Probe: ");
	perror(message);
	return ret;
}

static int _probe_timeout(struct sysinfo * info)
{
#ifdef DEBUG
	static unsigned int count = 0;

	fprintf(stderr, "%s%d%s", "_probe_timeout(", count++, ")\n");
#endif
	if(sysinfo(info) != 0)
		return _probe_error("sysinfo", 0);
	return 0;
}


/* AppInterface */
int uptime(void)
{
	printf("%s%ld%s", "Uptime: ", info.uptime, "\n");
	return info.uptime;
}


int load1(void)
{
	printf("%s%lu%s", "Load 1: ", info.loads[0], "\n");
	return info.loads[0];
}


int load5(void)
{
	printf("%s%lu%s", "Load 5: ", info.loads[1], "\n");
	return info.loads[1];
}


int load15(void)
{
	printf("%s%lu%s", "Load 15: ", info.loads[2], "\n");
	return info.loads[2];
}


/* main */
int main(int argc, char * argv[])
{
	return _probe();
}
