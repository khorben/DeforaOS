/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include "otherbox.h"


/* halt.c */
#define main _halt_main
#define _usage _halt_usage
#define _Prefs _halt_Prefs
#define Prefs halt_Prefs
#define _prefs_parse _halt_prefs_parse
#define _halt __halt
#include "../src/halt.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _halt

/* hexdump.c */
#define main _hexdump_main
#define _usage _hexdump_usage
#define _Prefs _hexdump_Prefs
#define Prefs hexdump_Prefs
#define _prefs_parse _hexdump_prefs_parse
#define _hexdump __hexdump
#include "../src/hexdump.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _hexdump

/* host.c */
#define main _host_main
#define _usage _host_usage
#define _Prefs _host_Prefs
#define Prefs host_Prefs
#define _prefs_parse _host_prefs_parse
#define _host __host
#include "../src/host.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _host

/* hostname.c */
#define main _hostname_main
#define _usage _hostname_usage
#define _Prefs _hostname_Prefs
#define Prefs hostname_Prefs
#define _prefs_parse _hostname_prefs_parse
#define _hostname __hostname
#include "../src/hostname.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _hostname

/* ifconfig.c */
#define main _ifconfig_main
#define _usage _ifconfig_usage
#define _Prefs _ifconfig_Prefs
#define Prefs ifconfig_Prefs
#define _prefs_parse _ifconfig_prefs_parse
#define _ifconfig __ifconfig
#include "../src/ifconfig.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _ifconfig

/* login.c */
#define main _login_main
#define _usage _login_usage
#define _Prefs _login_Prefs
#define Prefs login_Prefs
#define _prefs_parse _login_prefs_parse
#define _login __login
#include "../src/login.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _login

/* mktemp.c */
#define main _mktemp_main
#define _usage _mktemp_usage
#define _Prefs _mktemp_Prefs
#define Prefs mktemp_Prefs
#define _prefs_parse _mktemp_prefs_parse
#define _mktemp __mktemp
#include "../src/mktemp.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _mktemp

/* mount.c */
#define main _mount_main
#define _usage _mount_usage
#define _Prefs _mount_Prefs
#define Prefs mount_Prefs
#define _prefs_parse _mount_prefs_parse
#define _mount __mount
#include "../src/mount.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _mount

/* poweroff.c */
#define main _poweroff_main
#define _usage _poweroff_usage
#define _Prefs _poweroff_Prefs
#define Prefs poweroff_Prefs
#define _prefs_parse _poweroff_prefs_parse
#define _poweroff __poweroff
#include "../src/poweroff.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _poweroff

/* reboot.c */
#define main _reboot_main
#define _usage _reboot_usage
#define _Prefs _reboot_Prefs
#define Prefs reboot_Prefs
#define _prefs_parse _reboot_prefs_parse
#define _reboot __reboot
#include "../src/reboot.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _reboot

/* tar.c */
#define main _tar_main
#define _usage _tar_usage
#define _Prefs _tar_Prefs
#define Prefs tar_Prefs
#define _prefs_parse _tar_prefs_parse
#define _tar __tar
#include "../src/tar.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _tar

/* umount.c */
#define main _umount_main
#define _usage _umount_usage
#define _Prefs _umount_Prefs
#define Prefs umount_Prefs
#define _prefs_parse _umount_prefs_parse
#define _umount __umount
#include "../src/umount.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _umount

/* uptime.c */
#define main _uptime_main
#define _usage _uptime_usage
#define _Prefs _uptime_Prefs
#define Prefs uptime_Prefs
#define _prefs_parse _uptime_prefs_parse
#define _uptime __uptime
#include "../src/uptime.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _uptime

/* w.c */
#define main _w_main
#define _usage _w_usage
#define _Prefs _w_Prefs
#define Prefs w_Prefs
#define _prefs_parse _w_prefs_parse
#define _w __w
#include "../src/w.c"
#undef main
#undef _usage
#undef _Prefs
#undef Prefs
#undef _prefs_parse
#undef _w


Call calls[] =
{

	{ "halt",	_halt_main	},
	{ "hexdump",	_hexdump_main	},
	{ "host",	_host_main	},
	{ "hostname",	_hostname_main	},
	{ "ifconfig",	_ifconfig_main	},
	{ "login",	_login_main	},
	{ "mktemp",	_mktemp_main	},
	{ "mount",	_mount_main	},
	{ "poweroff",	_poweroff_main	},
	{ "reboot",	_reboot_main	},
	{ "tar",	_tar_main	},
	{ "umount",	_umount_main	},
	{ "uptime",	_uptime_main	},
	{ "w",	_w_main	},
	{ NULL,		NULL		}
};
