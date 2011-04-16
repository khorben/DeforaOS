/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#ifndef LIBDESKTOP_COMPAT_H
# define LIBDESKTOP_COMPAT_H


/* Compatibility */
/* constants */
# if !GTK_CHECK_VERSION(3, 0, 0)
#  define GDK_KEY_A	GDK_A
#  define GDK_KEY_B	GDK_B
#  define GDK_KEY_C	GDK_C
#  define GDK_KEY_D	GDK_D
#  define GDK_KEY_E	GDK_E
#  define GDK_KEY_F	GDK_F
#  define GDK_KEY_G	GDK_G
#  define GDK_KEY_H	GDK_H
#  define GDK_KEY_I	GDK_I
#  define GDK_KEY_J	GDK_J
#  define GDK_KEY_K	GDK_K
#  define GDK_KEY_L	GDK_L
#  define GDK_KEY_M	GDK_M
#  define GDK_KEY_N	GDK_N
#  define GDK_KEY_O	GDK_O
#  define GDK_KEY_P	GDK_P
#  define GDK_KEY_Q	GDK_Q
#  define GDK_KEY_R	GDK_R
#  define GDK_KEY_S	GDK_S
#  define GDK_KEY_T	GDK_T
#  define GDK_KEY_U	GDK_U
#  define GDK_KEY_V	GDK_V
#  define GDK_KEY_W	GDK_W
#  define GDK_KEY_X	GDK_X
#  define GDK_KEY_Y	GDK_Y
#  define GDK_KEY_Z	GDK_Z
# endif

#endif /* !LIBDESKTOP_COMPAT_H */
