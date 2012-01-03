/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
# if !GTK_CHECK_VERSION(2, 22, 0)
#  define GDK_KEY_0		GDK_0
#  define GDK_KEY_1		GDK_1
#  define GDK_KEY_2		GDK_2
#  define GDK_KEY_3		GDK_3
#  define GDK_KEY_4		GDK_4
#  define GDK_KEY_5		GDK_5
#  define GDK_KEY_6		GDK_6
#  define GDK_KEY_7		GDK_7
#  define GDK_KEY_8		GDK_8
#  define GDK_KEY_9		GDK_9
#  define GDK_KEY_A		GDK_A
#  define GDK_KEY_B		GDK_B
#  define GDK_KEY_C		GDK_C
#  define GDK_KEY_D		GDK_D
#  define GDK_KEY_E		GDK_E
#  define GDK_KEY_F		GDK_F
#  define GDK_KEY_G		GDK_G
#  define GDK_KEY_H		GDK_H
#  define GDK_KEY_I		GDK_I
#  define GDK_KEY_J		GDK_J
#  define GDK_KEY_K		GDK_K
#  define GDK_KEY_L		GDK_L
#  define GDK_KEY_M		GDK_M
#  define GDK_KEY_N		GDK_N
#  define GDK_KEY_O		GDK_O
#  define GDK_KEY_P		GDK_P
#  define GDK_KEY_Q		GDK_Q
#  define GDK_KEY_R		GDK_R
#  define GDK_KEY_S		GDK_S
#  define GDK_KEY_T		GDK_T
#  define GDK_KEY_U		GDK_U
#  define GDK_KEY_V		GDK_V
#  define GDK_KEY_W		GDK_W
#  define GDK_KEY_X		GDK_X
#  define GDK_KEY_Y		GDK_Y
#  define GDK_KEY_Z		GDK_Z
#  define GDK_KEY_asterisk	GDK_asterisk
#  define GDK_KEY_Back		GDK_Back
#  define GDK_KEY_Delete	GDK_Delete
#  define GDK_KEY_downarrow	GDK_downarrow
#  define GDK_KEY_Escape	GDK_Escape
#  define GDK_KEY_F1		GDK_F1
#  define GDK_KEY_F2		GDK_F2
#  define GDK_KEY_F3		GDK_F3
#  define GDK_KEY_F4		GDK_F4
#  define GDK_KEY_F5		GDK_F5
#  define GDK_KEY_F6		GDK_F6
#  define GDK_KEY_F7		GDK_F7
#  define GDK_KEY_F8		GDK_F8
#  define GDK_KEY_F9		GDK_F9
#  define GDK_KEY_F10		GDK_F10
#  define GDK_KEY_F11		GDK_F11
#  define GDK_KEY_F12		GDK_F12
#  define GDK_KEY_Forward	GDK_Forward
#  define GDK_KEY_Home		GDK_Home
#  define GDK_KEY_Left		GDK_Left
#  define GDK_KEY_minus		GDK_minus
#  define GDK_KEY_plus		GDK_plus
#  define GDK_KEY_Return	GDK_Return
#  define GDK_KEY_Right		GDK_Right
#  define GDK_KEY_slash		GDK_slash
#  define GDK_KEY_Up		GDK_Up
#  define GDK_KEY_uparrow	GDK_uparrow
# endif

#endif /* !LIBDESKTOP_COMPAT_H */
