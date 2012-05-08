/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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



#ifndef DESKTOP_KEYBOARD_H
# define DESKTOP_KEYBOARD_H


/* Keyboard */
/* types */
typedef enum _KeyboardMessage
{
	KEYBOARD_MESSAGE_SET_PAGE = 0,
	KEYBOARD_MESSAGE_SET_VISIBLE
} KeyboardMessage;

typedef enum _KeyboardPage
{
	KEYBOARD_PAGE_DEFAULT = 0,
	KEYBOARD_PAGE_KEYPAD,
	KEYBOARD_PAGE_URL
} KeyboardPage;


/* constants */
# define KEYBOARD_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_KEYBOARD_CLIENT"

#endif /* !DESKTOP_KEYBOARD_H */
