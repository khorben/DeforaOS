/* probe.c */



#include <System.h>
#include <utmpx.h>
#include <stdio.h>
#ifdef _GNU_SOURCE
# include <sys/sysinfo.h>
#else
# include <sys/param.h>
# include <sys/sysctl.h>
# include <sys/resource.h>
# include <string.h>
#endif

#define PROBE_REFRESH 10


/* globals */
#ifndef _GNU_SOURCE
struct sysinfo
{
	long uptime;
	unsigned long loads[3];
	unsigned long totalram;
	unsigned long freeram;
	unsigned long sharedram;
	unsigned long bufferram;
	unsigned long totalswap;
	unsigned long freeswap;
	unsigned short procs;
};

static int sysinfo(struct sysinfo * info)
{
	struct timeval now;
	struct timeval tv;
	struct loadavg la;
	struct uvmexp ue;
	int mib[3];
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

	/* ram */
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;
	len = sizeof(ue);
	if(sysctl(mib, 2, &ue, &len, NULL, 0) == -1)
	{
		info->totalram = 0;
		info->freeram = 0;
		info->sharedram = 0;
		info->bufferram = 0;
		info->totalswap = 0;
		info->freeswap = 0;
		ret++;
	}
	else
	{
		info->totalram = ue.pagesize * ue.npages;
		info->freeram = ue.pagesize * ue.free;
		info->sharedram = ue.pagesize * ue.execpages;
		info->bufferram = ue.pagesize * ue.filepages;
		info->totalswap = ue.pagesize * ue.swpages;
		info->freeswap = ue.pagesize * ue.swpgavail;
	}

	/* procs */
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;
	len = 0;
	if(sysctl(mib, 3, NULL, &len, NULL, 0) == -1)
	{
		info->procs = 0;
		ret++;
	}
	else
		info->procs = len / sizeof(struct kinfo_proc);
	return ret;
}
#endif


/* Probe */
typedef struct _Probe
{
	struct sysinfo sysinfo;
	unsigned int users;
} Probe;
Probe probe;
static int _probe_error(char * message, int ret);
static int _probe_timeout(Probe * probe);
static int _probe(void)
{
	AppServer * appserver;
	Event * event;
	struct timeval tv;

	if(_probe_timeout(&probe) != 0)
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
			&probe) != 0)
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

static int _probe_timeout(Probe * probe)
{
	struct utmpx * ut;
#ifdef DEBUG
	static unsigned int count = 0;

	fprintf(stderr, "%s%d%s", "_probe_timeout(", count++, ")\n");
#endif
	if(sysinfo(&probe->sysinfo) != 0)
		return _probe_error("sysinfo", 1);
	for(probe->users = 0; (ut = getutxent()) != NULL;)
		if(ut->ut_type == USER_PROCESS)
			probe->users++;
	endutxent();
	return 0;
}


/* AppInterface */
int uptime(void)
{
	printf("%s%ld%s", "Uptime: ", probe.sysinfo.uptime, "\n");
	return probe.sysinfo.uptime;
}


int load_1(void)
{
	printf("%s%lu%s", "Load 1: ", probe.sysinfo.loads[0], "\n");
	return probe.sysinfo.loads[0];
}


int load_5(void)
{
	printf("%s%lu%s", "Load 5: ", probe.sysinfo.loads[1], "\n");
	return probe.sysinfo.loads[1];
}


int load_15(void)
{
	printf("%s%lu%s", "Load 15: ", probe.sysinfo.loads[2], "\n");
	return probe.sysinfo.loads[2];
}


int ram_total(void)
{
	printf("%s%lu%s", "Total RAM: ", probe.sysinfo.totalram, "\n");
	return probe.sysinfo.totalram;
}

int ram_free(void)
{
	printf("%s%lu%s", "Free RAM: ", probe.sysinfo.freeram, "\n");
	return probe.sysinfo.freeram;
}

int ram_shared(void)
{
	printf("%s%lu%s", "Shared RAM: ", probe.sysinfo.sharedram, "\n");
	return probe.sysinfo.sharedram;
}

int ram_buffer(void)
{
	printf("%s%lu%s", "Buffered RAM: ", probe.sysinfo.bufferram, "\n");
	return probe.sysinfo.bufferram;
}


int swap_total(void)
{
	return probe.sysinfo.totalswap;
}

int swap_free(void)
{
	return probe.sysinfo.freeswap;
}


int procs(void)
{
	printf("%s%u%s", "Procs: ", probe.sysinfo.procs, "\n");
	return probe.sysinfo.procs;
}


int users(void)
{
	printf("%s%u%s", "Users: ", probe.users, "\n");
	return probe.users;
}


/* main */
int main(void)
{
	return _probe();
}
