/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of Directory */
/* Directory is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Directory is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Directory; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef DIRECTORY_DIRECTORY_H
# define DIRECTORY_DIRECTORY_H

# include <System.h>


/* Directory */
/* types */
typedef struct _Directory Directory;


/* functions */
Directory * directory_new(Event * event);
void directory_delete(Directory * directory);

#endif /* !DIRECTORY_DIRECTORY_H */
