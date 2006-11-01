/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */
/* TODO:
 * - check ifinfo code (and thus volinfo)
 * - free memory allocated
 * - kvm_getprocs() may be more portable for *BSD */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <System.h>


#if defined(__linux__)
# define _sysinfo_linux			_sysinfo
# define _userinfo_generic		_userinfo
# define _ifinfo_linux			_ifinfo
# define _volinfo_linux			_volinfo
#elif defined(__NetBSD__) /* FIXME Other BSDs not tested */
# define _sysinfo_generic		_sysinfo
# define _userinfo_generic		_userinfo
# define _ifinfo_bsd			_ifinfo
# define _volinfo_bsd			_volinfo
#else
# define _sysinfo_generic		_sysinfo
# define _userinfo_generic		_userinfo
# define _ifinfo_generic		_ifinfo
# define _volinfo_generic		_volinfo
#endif

#define PROBE_REFRESH 10


/* functions */
static int _probe_error(char const * message, int ret);


/* sysinfo */
#if defined(_sysinfo_linux)
# include <sys/sysinfo.h>
# define _sysinfo sysinfo
#endif /* defined(_sysinfo_linux) */

#if defined(_sysinfo_generic)
# if defined(__NetBSD__) /* FIXME Other BSDs not tested */
#  define _sysinfo_uptime_sysctl	_sysinfo_uptime
#  define _sysinfo_loads_sysctl		_sysinfo_loads
#  define _sysinfo_ram_sysctl		_sysinfo_ram
#  define _sysinfo_procs_sysctl		_sysinfo_procs
# else
#  define _sysinfo_uptime_generic	_sysinfo_uptime
#  define _sysinfo_loads_generic	_sysinfo_loads
#  define _sysinfo_ram_generic		_sysinfo_ram
#  define _sysinfo_procs_generic	_sysinfo_procs
# endif
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
static int _sysinfo_generic(struct sysinfo * info)
{
	int ret = 0;

	ret |= _sysinfo_uptime(info);
	ret |= _sysinfo_loads(info);
	ret |= _sysinfo_ram(info);
	ret |= _sysinfo_procs(info);
	return ret;
}

/* sysinfo sysctl */
# if defined(_sysinfo_uptime_sysctl) || defined(_sysinfo_loads_sysctl) \
	|| defined(_sysinfo_ram_sysctl) || defined(_sysinfo_procs_sysctl)
#  include <sys/param.h>
#  include <sys/sysctl.h>
# endif
# if defined(_sysinfo_uptime_sysctl)
static int _sysinfo_uptime_sysctl(struct sysinfo * info)
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
# endif

# if defined(_sysinfo_loads_sysctl)
#  include <sys/resource.h>
static int _sysinfo_loads_sysctl(struct sysinfo * info)
{
	int mib[2];
	size_t len;
	struct loadavg la;

	mib[0] = CTL_VM;
	mib[1] = VM_LOADAVG;
	len = sizeof(la);
	if(sysctl(mib, 2, &la, &len, NULL, 0) == -1)
	{
		memset(info->loads, 0, sizeof(info->loads));
		return 1;
	}
	info->loads[0] = la.ldavg[0] * la.fscale / 64;
	info->loads[1] = la.ldavg[1] * la.fscale / 64;
	info->loads[2] = la.ldavg[2] * la.fscale / 64;
	return 0;
}
# endif

# if defined(_sysinfo_ram_sysctl)
static int _sysinfo_ram_sysctl(struct sysinfo * info)
{
	int mib[2];
	size_t len;
	struct uvmexp ue;

	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;
	len = sizeof(ue);
	if(sysctl(mib, 2, &ue, &len, NULL, 0) == -1)
	{
		memset(info, 0, sizeof(*info));
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
# endif

# if defined(_sysinfo_procs_sysctl)
static int _sysinfo_procs_sysctl(struct sysinfo * info)
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
# endif

/* sysinfo generic */
# if defined(_sysinfo_uptime_generic)
#  warning Generic uptime reporting is not supported
static int _sysinfo_uptime_generic(struct sysinfo * info)
{
	info->uptime = 0;
	return 0;
}
# endif

# if defined(_sysinfo_loads_generic)
static int _sysinfo_loads_generic(struct sysinfo * info)
{
	double la[3];

	if(getloadavg(la, 3) != 3)
	{
		memset(info->loads, 0, sizeof(info->loads));
		return 1;
	}
	info->loads[0] = la[0];
	info->loads[1] = la[1];
	info->loads[2] = la[2];
	return 0;
}
# endif

# if defined(_sysinfo_ram_generic)
#  warning Generic RAM reporting is not supported
static int _sysinfo_ram_generic(struct sysinfo * info)
{
	info->totalram = 0;
	info->freeram = 0;
	info->sharedram = 0;
	info->bufferram = 0;
	info->totalswap = 0;
	info->freeswap = 0;
	return 0;
}
# endif

# if defined(_sysinfo_procs_generic)
#  warning Generic process reporting is not supported
static int _sysinfo_procs_generic(struct sysinfo * info)
{
	info->procs = 0;
	return 0;
}
# endif
#endif /* defined(_sysinfo_generic) */


/* userinfo */
#if defined(_userinfo_generic)
#  include <utmpx.h>
static int _userinfo_generic(unsigned int * userinfo)
{
	struct utmpx * ut;

	for(*userinfo = 0; (ut = getutxent()) != NULL;)
		if(ut->ut_type == USER_PROCESS)
			(*userinfo)++;
	endutxent();
	return 0;
}
#endif /* defined(_userinfo_generic) */


/* ifinfo */
struct ifinfo
{
	char name[6];
	unsigned int ibytes;
	unsigned int obytes;
};

/* ifinfo linux */
#if defined(_ifinfo_linux)
enum InterfaceInfo
{
	IF_RX_BYTES = 0, IF_RX_PACKETS, IF_RX_ERRS, IF_RX_DROP, IF_RX_FIFO,
	IF_RX_FRAME, IF_RX_COMPRESSED, IF_RX_MULTICAST,
	IF_TX_BYTES, IF_TX_PACKETS, IF_TX_ERRS, IF_TX_DROP, IF_TX_FIFO,
	IF_TX_FRAME, IF_TX_COMPRESSED
};
# define IF_LAST IF_TX_COMPRESSED

static int _ifinfo_linux_append(struct ifinfo ** dev, char * buf, int nb);
static int _ifinfo_linux(struct ifinfo ** dev)
{
	int ret = 0;
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
		if(_ifinfo_linux_append(dev, buf, i - 2) != 0)
		{
			ret = -1;
			break;
		}
		ret++;
	}
	fclose(fp);
	return ret;
}

static int _ifinfo_linux_append(struct ifinfo ** dev, char * buf, int nb)
{
	struct ifinfo * p;
	size_t i;
	char * q;
	int j = 0;

	if((p = realloc(*dev, sizeof(*p) * (nb + 1))) == NULL)
		return _probe_error("realloc", 1);
	*dev = p;
	for(i = 0; i < sizeof(p->name) && buf[i] != '\0'; i++);
	if(i != sizeof(p->name))
		return 1;
	buf[sizeof(p->name)] = '\0';
	for(q = buf; q[0] == ' '; q++);
	strcpy(p[nb].name, q);
# if defined(DEBUG)
	fprintf(stderr, "_ifinfo_append: %s\n", p[nb].name);
# endif
	for(i++; buf[i] != '\0'; i++)
	{
		if(j > IF_LAST)
			break;
		if(buf[i] == ' ')
			continue;
		q = &buf[i];
		for(; buf[i] >= '0' && buf[i] <= '9'; i++);
		buf[i] = '\0';
		switch(j++)
		{
			case IF_RX_BYTES:
				(*dev)[nb].ibytes = strtoll(q, &q, 10);
				break;
			case IF_TX_BYTES:
				(*dev)[nb].obytes = strtoll(q, &q, 10);
				break;
			default:
				continue;
		}
		if(*q != '\0')
			return 1;
	}
	return 0;
}
#endif /* defined(_ifinfo_linux) */

/* ifinfo netbsd */
#if defined(_ifinfo_bsd)
# include <net/if.h>
# include <ifaddrs.h>
static int _ifinfo_bsd_append(struct ifinfo ** dev, char * ifname, int fd,
		int nb);
static int _ifinfo_bsd(struct ifinfo ** dev)
{
	int ret = 0;
	static int fd = -1;
	struct ifaddrs * ifa;
	struct ifaddrs * p;

	if(fd < 0 && (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return _probe_error("socket", -1);
	if(getifaddrs(&ifa) != 0)
		return _probe_error("getifaddrs", -1);
	for(p = ifa; p != NULL; p = p->ifa_next)
	{
		if(p->ifa_addr->sa_family != AF_LINK)
			continue;
		if(_ifinfo_bsd_append(dev, p->ifa_name, fd, ret++) == 0)
			continue;
		ret = -1;
		break;
	}
	freeifaddrs(ifa);
	return ret;
}

static int _ifinfo_bsd_append(struct ifinfo ** dev, char * ifname, int fd,
		int nb)
{
	struct ifdatareq ifdr;
	struct ifinfo * p;

	strcpy(ifdr.ifdr_name, ifname);
	if(ioctl(fd, SIOCGIFDATA, &ifdr) == -1)
		return _probe_error("SIOCGIFDATA", 1);
	if((p = realloc(*dev, sizeof(*p) * (nb + 1))) == NULL)
		return _probe_error("realloc", 1);
	*dev = p;
	strcpy(p[nb].name, ifname);
# if defined(DEBUG)
	fprintf(stderr, "_ifinfo_append: %s\n", p[nb].name);
# endif
	p[nb].ibytes = ifdr.ifdr_data.ifi_ibytes;
	p[nb].obytes = ifdr.ifdr_data.ifi_obytes;
	return 0;
}
#endif /* defined(_ifinfo_bsd) */

/* ifinfo generic */
#if defined(_ifinfo_generic)
# warning Generic interface reporting is not supported
static int _ifinfo_generic(struct ifinfo ** dev)
{
	*dev = NULL;
	return 0;
}
#endif /* defined(_ifinfo_generic) */


/* volinfo */
struct volinfo
{
	char name[256];
	unsigned long block_size;
	unsigned long total;
	unsigned long free;
};

/* volinfo linux */
#if defined(_volinfo_linux)
# include <sys/statvfs.h>
enum VolInfo
{
	VI_DEVICE = 0, VI_MOUNTPOINT, VI_FS, VI_OPTIONS, VI_DUMP, VI_PASS
};
#define VI_LAST VI_PASS

static int _volinfo_linux_append(struct volinfo ** dev, char * buf, int nb);
static int _volinfo_linux(struct volinfo ** dev)
{
	int ret = 0;
	FILE * fp;
	char buf[200];
	int i;
	int len;

	if((fp = fopen("/etc/mtab", "r")) == NULL)
		return _probe_error("/etc/mtab", -1);
	for(i = 0; fgets(buf, sizeof(buf), fp) != NULL; i++)
	{
		len = string_length(buf);
		if(buf[len-1] != '\n')
		{
			ret = -1;
			break;
		}
		if(_volinfo_linux_append(dev, buf, i) != 0)
		{
			ret = -1;
			break;
		}
		ret++;
	}
	fclose(fp);
	return ret;
}

static int _volinfo_linux_append(struct volinfo ** dev, char * buf, int nb)
{
	unsigned int i;
	unsigned int j;
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
	if(j-i >= sizeof(p[nb].name)-1)
		return 1;
	strncpy(p[nb].name, &buf[i], j-i);
	p[nb].name[j-i] = '\0';
# if defined(DEBUG)
	fprintf(stderr, "_volinfo_append: %s\n", p[nb].name);
# endif
	if(statvfs(p[nb].name, &sv) != 0)
		return 1;
	p[nb].block_size = sv.f_bsize;
	p[nb].total = sv.f_blocks;
	p[nb].free = sv.f_bavail;
	return 0;
}
#endif /* defined(_volinfo_linux) */

/* volinfo_bsd */
#if defined(_volinfo_bsd)
# include <sys/statvfs.h>
static int _volinfo_bsd_append(struct volinfo ** dev, struct statvfs * buf,
		int nb);
static int _volinfo_bsd(struct volinfo ** dev)
{
	int ret;
	struct statvfs * buf;
	int cnt;
	int cnt2;

	if((cnt = getvfsstat(NULL, 0, ST_WAIT)) == -1)
		return _probe_error("getvfsstat", -1);
	if((buf = malloc(sizeof(struct statvfs) * cnt)) == NULL)
		return _probe_error("malloc", -1);
	if((cnt2 = getvfsstat(buf, sizeof(struct statvfs) * cnt, ST_WAIT))
			== -1)
	{
		free(buf);
		return _probe_error("getvfsstat", -1);
	}
	for(ret = 0; ret < cnt && ret < cnt2; ret++)
	{
		if(_volinfo_bsd_append(dev, &buf[ret], ret) == 0)
			continue;
		ret = -1;
		break;
	}
	free(buf);
	return ret;
}

static int _volinfo_bsd_append(struct volinfo ** dev, struct statvfs * buf,
		int nb)
{
	struct volinfo * p;

	if((p = realloc(*dev, sizeof(*p) * (nb + 1))) == NULL)
		return _probe_error("realloc", 1);
	*dev = p;
	strcpy(p[nb].name, buf->f_mntonname);
# if defined(DEBUG)
	fprintf(stderr, "_volinfo_append: %s\n", p[nb].name);
# endif
	p[nb].block_size = buf->f_bsize;
	p[nb].total = buf->f_blocks * 2048 / buf->f_bsize;
	p[nb].free = buf->f_bavail * 2048 / buf->f_bsize;
	return 0;
}
#endif /* defined(_volinfo_bsd) */

/* volinfo generic */
#if defined(_volinfo_generic)
# warning Generic volume information is not supported
static int _volinfo_generic(struct volinfo ** dev)
{
	*dev = NULL;
	return 0;
}
#endif /* defined(_volinfo_generic) */


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
static int _probe_timeout(Probe * probe);
static int _probe(void)
{
	AppServer * appserver;
	Event * event;
	struct timeval tv;

	memset(&probe, 0, sizeof(Probe));
	if(_probe_timeout(&probe) != 0)
	{
		free(probe.ifinfo);
		free(probe.volinfo);
		return 1;
	}
	if((event = event_new()) == NULL)
	{
		free(probe.ifinfo);
		free(probe.volinfo);
		return _probe_error("Event", 1);
	}
	if((appserver = appserver_new_event("Probe", ASO_REMOTE, event))
			== NULL)
	{
		free(probe.ifinfo);
		free(probe.volinfo);
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
	free(probe.ifinfo);
	free(probe.volinfo);
	return 1;
}

static int _probe_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "Probe: ");
	perror(message);
	return ret;
}

static int _probe_timeout(Probe * probe)
{
	int i;
#if defined(DEBUG)
	static unsigned int count = 0;

	fprintf(stderr, "%s%d%s", "_probe_timeout(", count++, ")\n");
#endif
	if(_sysinfo(&probe->sysinfo) != 0)
		return _probe_error("sysinfo", 1);
	if(_userinfo(&probe->users) != 0)
		return _probe_error("userinfo", 1);
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
#if defined(DEBUG)
	printf("%s%ld%s", "Uptime: ", probe.sysinfo.uptime, "\n");
#endif
	return probe.sysinfo.uptime;
}


int load_1(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Load 1: ", probe.sysinfo.loads[0], "\n");
#endif
	return probe.sysinfo.loads[0];
}


int load_5(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Load 5: ", probe.sysinfo.loads[1], "\n");
#endif
	return probe.sysinfo.loads[1];
}


int load_15(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Load 15: ", probe.sysinfo.loads[2], "\n");
#endif
	return probe.sysinfo.loads[2];
}


int ram_total(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Total RAM: ", probe.sysinfo.totalram, "\n");
#endif
	return probe.sysinfo.totalram;
}

int ram_free(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Free RAM: ", probe.sysinfo.freeram, "\n");
#endif
	return probe.sysinfo.freeram;
}

int ram_shared(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Shared RAM: ", probe.sysinfo.sharedram, "\n");
#endif
	return probe.sysinfo.sharedram;
}

int ram_buffer(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Buffered RAM: ", probe.sysinfo.bufferram, "\n");
#endif
	return probe.sysinfo.bufferram;
}


int swap_total(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Total swap: ", probe.sysinfo.totalswap, "\n");
#endif
	return probe.sysinfo.totalswap;
}

int swap_free(void)
{
#if defined(DEBUG)
	printf("%s%lu%s", "Free swap: ", probe.sysinfo.freeswap, "\n");
#endif
	return probe.sysinfo.freeswap;
}


int procs(void)
{
#if defined(DEBUG)
	printf("%s%u%s", "Procs: ", probe.sysinfo.procs, "\n");
#endif
	return probe.sysinfo.procs;
}


int users(void)
{
#if defined(DEBUG)
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
#if defined(DEBUG)
	printf("%s%s%s%u%s", "Interface ", probe.ifinfo[i].name, " RX: ",
			probe.ifinfo[i].ibytes, "\n");
#endif
	return probe.ifinfo[i].ibytes;
}

int iftxbytes(char * dev)
{
	unsigned int i;

	for(i = 0; i < probe.ifinfo_cnt
			&& string_compare(probe.ifinfo[i].name, dev) != 0; i++);
	if(i == probe.ifinfo_cnt)
		return -1;
#if defined(DEBUG)
	printf("%s%s%s%u%s", "Interface ", probe.ifinfo[i].name, " TX: ",
			probe.ifinfo[i].obytes, "\n");
#endif
	return probe.ifinfo[i].obytes;
}


int voltotal(char * vol)
{
	unsigned int i;

	for(i = 0; i < probe.volinfo_cnt
			&& string_compare(probe.volinfo[i].name, vol) != 0;
			i++);
	if(i == probe.volinfo_cnt)
		return -1;
#if defined(DEBUG)
	printf("%s%s%s%lu%s", "Volume ", probe.volinfo[i].name, " total: ",
			probe.volinfo[i].total, "\n");
#endif
	return probe.volinfo[i].total * (probe.volinfo[i].block_size / 1024);
}

int volfree(char * vol)
{
	unsigned int i;

	for(i = 0; i < probe.volinfo_cnt
			&& string_compare(probe.volinfo[i].name, vol) != 0;
			i++);
	if(i == probe.volinfo_cnt)
		return -1;
#if defined(DEBUG)
	printf("%s%s%s%lu%s", "Volume ", probe.volinfo[i].name, " free: ",
			probe.volinfo[i].free, "\n");
#endif
	return probe.volinfo[i].free * (probe.volinfo[i].block_size / 1024);
}


/* main */
int main(void)
{
	return _probe() == 0 ? 0 : 2;
}
