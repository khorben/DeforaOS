/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
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



#ifndef AS_AS_H
# define AS_AS_H

# include "As/as.h"
# include "arch.h"
# include "format.h"


/* functions */
/* accessors */
Arch * as_get_arch(As * as);
Format * as_get_format(As * as);

#endif /* !AS_AS_H */
