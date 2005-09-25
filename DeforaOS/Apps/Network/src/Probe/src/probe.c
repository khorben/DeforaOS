/* probe.c */



#include <System.h>
#include <stdio.h>
#ifdef LINUX
# include <sys/sysinfo.h>
#endif
#ifdef BSD
# include <sys/param.h>
# include <sys/sysctl.h>
# include <sys/resource.h>
# include <string.h>
#endif

#define PROBE_REFRESH 5


/* globals */
#ifdef BSD
struct sysinfo
{
	long uptime;
	unsigned long loads[3];
	unsigned short procs;
};

static int sysinfo(struct sysinfo * info)
{
	struct timeval now;
	struct timeval tv;
	struct loadavg la;
	int mib[2];
	int len;
	int ret = 0;

	/* uptime */
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	len = sizeof(tv);
	if(gettimeofday(&now, NULL) != 0
			|| sysctl(mib, 2, &tv, &len, NULL, 0) == -1)
	{
		info->uptime = 0;
		ret++;
	}
	else
		info->uptime = now.tv_sec - tv.tv_sec;

	/* loads */
	/* FIXME getloadavg() looks portable */
	mib[0] = CTL_VM;
	mib[1] = VM_LOADAVG;
	len = sizeof(la);
	if(sysctl(mib, 2, &la, &len, NULL, 0) == -1)
	{
		memset(info->loads, 0, sizeof(info->loads));
		ret++;
	}
	else
	{
		info->loads[0] = la.ldavg[0];
		info->loads[1] = la.ldavg[1];
		info->loads[2] = la.ldavg[2];
	}

	/* procs */
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	if(sysctl(mib, 2, NULL, &len, NULL, 0) == -1)
	{
		info->procs = 0;
		ret++;
	}
	else
		info->procs = len;
	return ret;
}
#endif
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


int procs(void)
{
	printf("%s%u%s", "Procs: ", info.procs, "\n");
	return info.procs;
}


/* main */
int main(int argc, char * argv[])
{
	return _probe();
}
