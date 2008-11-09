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



#ifndef INIT_INIT_H
# define INIT_INIT_H


/* Init */
/* types */
typedef struct _Init Init;


/* functions */
Init * init_new(char const * profile);
void init_delete(Init * init);

/* AppInterface */
int init_get_profile(String ** profile);
int init_login(String const * username);
int init_logout(void);
int init_register(char const * service, uint16_t port);
int init_set_profile(String const * profile);

/* useful */
int init_loop(Init * init);

#endif /* !INIT_INIT_H */
