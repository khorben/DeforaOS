/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Framer */
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



#ifndef FRAMER_FRAMER_H
# define FRAMER_FRAMER_H


/* Framer */
/* types */
typedef struct _Framer Framer;


/* functions */
Framer * framer_new(void);
void framer_delete(Framer * framer);

/* accessors */
void framer_set_show_desktop(Framer * framer, gboolean shown);

#endif /* !FRAMER_FRAMER_H */
