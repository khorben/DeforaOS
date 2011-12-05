/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VPN */
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



#include "VPN.h"


/* types */
typedef struct _VPNErrno
{
	int native;
	int error;
} VPNError;

typedef struct _VPNFlag
{
	unsigned int native;
	unsigned int flag;
} VPNFlag;


/* variables */
/* errors */
static VPNError _vpn_error[] =
{
	{ EPERM,	VPN_EPERM	},
	{ EBADF,	VPN_EBADF	},
	{ EPROTO,	VPN_EPROTO	}
};
static const size_t _vpn_error_cnt = sizeof(_vpn_error) / sizeof(*_vpn_error);


/* prototypes */
static int _vpn_errno(VPNError * error, size_t error_cnt, int value,
		int reverse);
static int _vpn_flags(VPNFlag * flags, size_t flags_cnt, int value,
		int reverse);


/* functions */
/* vpn_errno */
static int _vpn_errno(VPNError * error, size_t error_cnt, int value,
		int reverse)
{
	size_t i;

	for(i = 0; i < error_cnt; i++)
		if(reverse == 0 && value == error[i].native)
			return -error[i].error;
		else if(reverse != 0 && value == error[i].error)
		{
			errno = error[i].native;
			return -1;
		}
	/* FIXME really is an unknown error */
	if(reverse == 0)
		return -VPN_EPROTO;
	errno = EPROTO;
	return -1;
}


/* vpn_flags */
static int _vpn_flags(VPNFlag * flags, size_t flags_cnt, int value, int reverse)
{
	int ret = 0;
	size_t i;

	for(i = 0; i < flags_cnt; i++)
		if(reverse == 0)
		{
			if((value & flags[i].native) == flags[i].native)
			{
				value -= flags[i].native;
				ret |= flags[i].flag;
			}
		}
		else if((value & flags[i].flag) == flags[i].flag)
		{
			value -= flags[i].flag;
			ret |= flags[i].native;
		}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %lu, %d, %d) => %d\n", __func__,
			(void *)flags, flags_cnt, value, reverse, (value == 0)
			? ret : -1);
#endif
	if(value != 0)
		return -1;
	return ret;
}
