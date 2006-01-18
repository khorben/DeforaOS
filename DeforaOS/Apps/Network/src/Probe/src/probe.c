/* probe.c */
/* TODO:
 * - check ifinfo code (and thus volinfo)
 * - free memory allocated */



#include <System.h>
#include <utmpx.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _GNU_SOURCE
# include <sys/sysinfo.h>
#else
# include <sys/param.h>
# include <sys/sysctl.h>
# include <sys/resource.h>
#endif

#define PROBE_REFRESH 10


/* globals */
/* sysinfo */
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

static int _sysinfo_uptime(struct sysinfo * info);
static int _sysinfo_loads(struct sysinfo * info);
static int _sysinfo_ram(struct sysinfo * info);
static int _sysinfo_procs(struct sysinfo * info);
static int sysinfo(struct sysinfo * info)
{
	int ret = 0;

	ret += _sysinfo_uptime(info);
	ret += _sysinfo_loads(info);
	ret += _sysinfo_ram(info);
	ret += _sysinfo_procs(info);
	return ret;
}

static int _sysinfo_uptime(struct sysinfo * info)
{
	int mib[2];
	size_t len;
	struct timeval now;
	struct timeval tv;

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	len = sizeof(tv);
	if(gettimeofday(&now, NULL) != 0
			|| sysctl(mib, 2, &tv, &len, NULL, 0) == -1)
	{
		info->uptime = 0;
		return 1;
	}
	info->uptime = now.tv_sec - tv.tv_sec;
	return 0;
}

static int _sysinfo_loads(struct sysinfo * info)
{
	int mib[2];
	size_t len;
	struct loadavg la;

	/* FIXME getloadavg() looks portable */
	mib[0] = CTL_VM;
	mib[1] = VM_LOADAVG;
	len = sizeof(la);
	if(sysctl(mib, 2, &la, &len, NULL, 0) == -1)
	{
		memset(info->loads, 0, sizeof(info->loads));
		return 1;
	}
	info->loads[0] = la.ldavg[0];
	info->loads[1] = la.ldavg[1];
	info->loads[2] = la.ldavg[2];
	return 0;
}

static int _sysinfo_ram(struct sysinfo * info)
{
	int mib[2];
	size_t len;
	struct uvmexp ue;

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
		return 1;
	}
	info->totalram = ue.pagesize * ue.npages;
	info->freeram = ue.pagesize * ue.free;
	info->sharedram = ue.pagesize * ue.execpages;
	info->bufferram = ue.pagesize * ue.filepages;
	info->totalswap = ue.pagesize * ue.swpages;
	info->freeswap = info->totalswap - (ue.pagesize * ue.swpgonly);
	return 0;
}

static int _sysinfo_procs(struct sysinfo * info)
{
	int mib[3];
	size_t len;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;
	len = 0;
	if(sysctl(mib, 3, NULL, &len, NULL, 0) == -1)
	{
		info->procs = 0;
		return 1;
	}
	info->procs = len / sizeof(struct kinfo_proc);
	return 0;
}
#endif


/* ifinfo */
enum InterfaceInfo
{
	IF_RX_BYTES = 0, IF_RX_PACKETS, IF_RX_ERRS, IF_RX_DROP, IF_RX_FIFO,
	IF_RX_FRAME, IF_RX_COMPRESSED, IF_RX_MULTICAST,
	IF_TX_BYTES, IF_TX_PACKETS, IF_TX_ERRS, IF_TX_DROP, IF_TX_FIFO,
	IF_TX_FRAME, IF_TX_COMPRESSED
};
#define IF_LAST IF_TX_COMPRESSED
struct ifinfo
{
	char name[6];
	unsigned int stats[IF_LAST+1];
};

#ifdef _GNU_SOURCE
static int _ifinfo_append(struct ifinfo ** dev, char * buf, int nb);
#endif
static int _ifinfo(struct ifinfo ** dev)
{
	int ret = 0;
#ifdef _GNU_SOURCE
	FILE * fp;
	char buf[200];
	int i;
	int len;

	if((fp = fopen("/proc/net/dev", "r")) == NULL)
		return -1;
	for(i = 0; fgets(buf, sizeof(buf), fp) != NULL; i++)
	{
		len = string_length(buf);
		if(buf[len-1] != '\n')
		{
			ret = -1;
			break;
		}
		if(i < 2)
			continue;
		if(_ifinfo_append(dev, buf, i - 1) != 0)
		{
			ret = -1;
			break;
		}
		ret++;
	}
	fclose(fp);
#endif
	return ret;
}

#ifdef _GNU_SOURCE
static int _ifinfo_append(struct ifinfo ** dev, char * buf, int nb)
{
	struct ifinfo * p;
	size_t i;
	char * q;
	int j = 0;

	if((p = realloc(*dev, sizeof(struct ifinfo) * (nb + 1))) == NULL)
		return 1;
	*dev = p;
	for(i = 0; i < sizeof(p->name) && buf[i] != '\0'; i++);
	if(i != sizeof(p->name))
		return 1;
	buf[sizeof(p->name)] = '\0';
	for(q = buf; q[0] == ' '; q++);
	strcpy(p[nb].name, q);
#ifdef DEBUG
	fprintf(stderr, "_ifinfo_append: %s\n", p[nb].name);
#endif
	for(i++; buf[i] != '\0'; i++)
	{
		if(j > IF_LAST)
			break;
		if(buf[i] == ' ')
			continue;
		q = &buf[i];
		for(; buf[i] >= '0' && buf[i] <= '9'; i++);
		buf[i] = '\0';
		(*dev)[nb].stats[j++] = strtoll(q, &q, 10);
		if(*q != '\0')
			return 1;
	}
	return 0;
}
#endif


/* volinfo */
enum VolInfo
{
	VI_DEVICE = 0, VI_MOUNTPOINT, VI_FS, VI_OPTIONS, VI_DUMP, VI_PASS
};
#define VI_LAST VI_PASS
struct volinfo
{
	char name[256];
	unsigned long block_size;
	fsblkcnt_t total;
	fsblkcnt_t free;
};

static int _volinfo_append(struct volinfo ** dev, char * buf, int nb);
static int _volinfo(struct volinfo ** dev)
{
	int ret = 0;
	FILE * fp;
	char buf[200];
	int i;
	int len;

	if((fp = fopen("/etc/mtab", "r")) == NULL)
		return -1;
	for(i = 0; fgets(buf, sizeof(buf), fp) != NULL; i++)
	{
		len = string_length(buf);
		if(buf[len-1] != '\n')
		{
			ret = -1;
			break;
		}
		if(_volinfo_append(dev, buf, i) != 0)
		{
			ret = -1;
			break;
		}
		ret++;
	}
	fclose(fp);
	return ret;
}

static int _volinfo_append(struct volinfo ** dev, char * buf, int nb)
{
	int i;
	int j;
	struct volinfo * p;
	struct statvfs sv;

	for(i = 0; buf[i] != '\0' && buf[i] != ' '; i++);
	if(buf[i] == '\0')
		return 1;
	for(j = ++i; buf[j] != '\0' && buf[j] != ' '; j++);
	if(buf[j] == '\0')
		return 1;
	if((p = realloc(*dev, sizeof(struct volinfo) * (nb + 1))) == NULL)
		return 1;
	*dev = p;
	memset(&p[nb], 0, sizeof(struct volinfo));
	strncpy(p[nb].name, &buf[i], j-i); /* FIXME overflow possible */
	p[nb].name[j-i] = '\0';
#ifdef DEBUG
	fprintf(stderr, "_volinfo_append: %s\n", p[nb].name);
#endif
	if(statvfs(p[nb].name, &sv) != 0)
		return 1;
	p[nb].block_size = sv.f_bsize;
	p[nb].total = sv.f_blocks;
	p[nb].free = sv.f_bavail;
	return 0;
}


/* Probe */
/* types */
typedef struct _Probe
{
	struct sysinfo sysinfo;
	unsigned int users;
	struct ifinfo * ifinfo;
	unsigned int ifinfo_cnt;
	struct volinfo * volinfo;
	unsigned int volinfo_cnt;
} Probe;

/* variables */
Probe probe;

/* functions */
static int _probe_error(char * message, int ret);
static int _probe_timeout(Probe * probe);
static int _probe(void)
{
	AppServer * appserver;
	Event * event;
	struct timeval tv;

	memset(&probe, 0, sizeof(Probe));
	if(_probe_timeout(&probe) != 0)
		/* FIXME free memory */
		return 1;
	if((event = event_new()) == NULL)
		return _probe_error("Event", 2);
	if((appserver = appserver_new_event("Probe", ASO_REMOTE, event))
			== NULL)
	{
		event_delete(event);
		return _probe_error("AppServer", 1);
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
	return 1;
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
	int i;
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
	if((i = _ifinfo(&probe->ifinfo)) < 0)
		return _probe_error("ifinfo", 1);
	probe->ifinfo_cnt = i;
	if((i = _volinfo(&probe->volinfo)) < 0)
		return _probe_error("volinfo", 1);
	probe->volinfo_cnt = i;
	return 0;
}


/* AppInterface */
int uptime(void)
{
#ifdef DEBUG
	printf("%s%ld%s", "Uptime: ", probe.sysinfo.uptime, "\n");
#endif
	return probe.sysinfo.uptime;
}


int load_1(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Load 1: ", probe.sysinfo.loads[0], "\n");
#endif
	return probe.sysinfo.loads[0];
}


int load_5(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Load 5: ", probe.sysinfo.loads[1], "\n");
#endif
	return probe.sysinfo.loads[1];
}


int load_15(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Load 15: ", probe.sysinfo.loads[2], "\n");
#endif
	return probe.sysinfo.loads[2];
}


int ram_total(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Total RAM: ", probe.sysinfo.totalram, "\n");
#endif
	return probe.sysinfo.totalram;
}

int ram_free(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Free RAM: ", probe.sysinfo.freeram, "\n");
#endif
	return probe.sysinfo.freeram;
}

int ram_shared(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Shared RAM: ", probe.sysinfo.sharedram, "\n");
#endif
	return probe.sysinfo.sharedram;
}

int ram_buffer(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Buffered RAM: ", probe.sysinfo.bufferram, "\n");
#endif
	return probe.sysinfo.bufferram;
}


int swap_total(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Total swap: ", probe.sysinfo.totalswap, "\n");
#endif
	return probe.sysinfo.totalswap;
}

int swap_free(void)
{
#ifdef DEBUG
	printf("%s%lu%s", "Free swap: ", probe.sysinfo.freeswap, "\n");
#endif
	return probe.sysinfo.freeswap;
}


int procs(void)
{
#ifdef DEBUG
	printf("%s%u%s", "Procs: ", probe.sysinfo.procs, "\n");
#endif
	return probe.sysinfo.procs;
}


int users(void)
{
#ifdef DEBUG
	printf("%s%u%s", "Users: ", probe.users, "\n");
#endif
	return probe.users;
}


int ifrxbytes(char * dev)
{
	unsigned int i;

	for(i = 0; i < probe.ifinfo_cnt
			&& string_compare(probe.ifinfo[i].name, dev) != 0; i++);
	if(i == probe.ifinfo_cnt)
		return -1;
#ifdef DEBUG
	printf("%s%s%s%u%s", "Interface ", probe.ifinfo[i].name, " RX: ",
			probe.ifinfo[i].stats[IF_RX_BYTES], "\n");
#endif
	return probe.ifinfo[i].stats[IF_RX_BYTES];
}

int iftxbytes(char * dev)
{
	unsigned int i;

	for(i = 0; i < probe.ifinfo_cnt
			&& string_compare(probe.ifinfo[i].name, dev) != 0; i++);
	if(i == probe.ifinfo_cnt)
		return -1;
#ifdef DEBUG
	printf("%s%s%s%u%s", "Interface ", probe.ifinfo[i].name, " TX: ",
			probe.ifinfo[i].stats[IF_TX_BYTES], "\n");
#endif
	return probe.ifinfo[i].stats[IF_TX_BYTES];
}


/* main */
int main(void)
{
	return _probe() == 0 ? 0 : 2;
}
