/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
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



#ifndef INIT_COMMON_H
# define INIT_COMMON_H

# include "../config.h"


/* constants */
/* Session */
# ifdef __DeforaOS__
#  define SESSIONDIR	PREFIX "/Config/Sessions"
# else
#  define SESSIONDIR	PREFIX "/etc/Sessions"
# endif
# define SESSIONEXT	"session"

/* Service */
# ifdef __DeforaOS__
#  define SERVICEDIR	PREFIX "/Config/Services"
# else
#  define SERVICEDIR	PREFIX "/etc/Services"
# endif
# define SERVICEEXT	"service"

#endif /* !INIT_COMMON_H */
