/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
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



#ifndef CONFIGURE_CONFIGURE_H
# define CONFIGURE_CONFIGURE_H

# include <System.h>


/* types */
typedef enum _HostArch
{
	HA_I386 = 0, HA_I486, HA_I586, HA_I686,
	HA_AMD64,
	HA_SPARC, HA_SPARC64,
	HA_SUN4U,
	HA_ZAURUS,
	HA_UNKNOWN
} HostArch;
# define HA_LAST	HA_UNKNOWN
# define HA_COUNT	(HA_LAST + 1)
extern const String * sHostArch[HA_COUNT];

typedef enum _HostOS
{
	HO_DEFORAOS = 0,
	HO_GNU_LINUX,
	HO_MACOSX,
	HO_FREEBSD, HO_NETBSD, HO_OPENBSD,
	HO_SUNOS,
	HO_UNKNOWN
} HostOS;
# define HO_LAST	HO_UNKNOWN
# define HO_COUNT	(HO_LAST + 1)
extern const String * sHostOS[HO_COUNT];

typedef enum _HostKernel
{
	HK_LINUX20 = 0, HK_LINUX22, HK_LINUX24, HK_LINUX26,
	HK_MACOSX106,
	HK_NETBSD2, HK_NETBSD3, HK_NETBSD4, HK_NETBSD5, HK_NETBSD51,
	HK_OPENBSD40, HK_OPENBSD41,
	HK_SUNOS57, HK_SUNOS58, HK_SUNOS59, HK_SUNOS510,
	HK_UNKNOWN
} HostKernel;
# define HK_LAST	HK_UNKNOWN
# define HK_COUNT	(HK_LAST + 1)
struct HostKernel
{
	HostOS os;
	const char * version;
};
extern const struct HostKernel sHostKernel[HK_COUNT];

typedef enum _TargetType
{
	TT_BINARY = 0, TT_LIBRARY, TT_LIBTOOL, TT_OBJECT, TT_PLUGIN, TT_SCRIPT,
	TT_UNKNOWN
} TargetType;
# define TT_LAST	TT_UNKNOWN
# define TT_COUNT	(TT_LAST + 1)
extern const String * sTargetType[TT_COUNT];

typedef enum _ObjectType
{
	OT_C_SOURCE = 0,
       	OT_CXX_SOURCE,
       	OT_ASM_SOURCE,
       	OT_UNKNOWN
} ObjectType;
# define OT_LAST	OT_UNKNOWN
# define OT_COUNT	(OT_LAST + 1)
struct ExtensionType
{
	const char * extension;
	ObjectType type;
};
extern const struct ExtensionType * sExtensionType;
String const * _source_extension(String const * source);
ObjectType _source_type(String const * source);


/* constants */
# define PROJECT_CONF	"project.conf"
# define MAKEFILE	"Makefile"


/* configure */
/* types */
typedef struct _Prefs
{
	int flags;
	char * bindir;
	char * destdir;
	char * includedir;
	char * libdir;
	char * prefix;
	char * os;
} Prefs;
# define PREFS_n	0x1
# define PREFS_S	0x2
# define PREFS_v	0x4
typedef struct _Configure
{
	Prefs * prefs;
	Config * config;
	HostArch arch;
	HostOS os;
	HostKernel kernel;
} Configure;


/* functions */
int configure_error(char const * message, int ret);

int enum_string(int last, const String * strings[], String const * str);
int enum_string_short(int last, const String * strings[], String const * str);

#endif /* !CONFIGURE_CONFIGURE_H */
