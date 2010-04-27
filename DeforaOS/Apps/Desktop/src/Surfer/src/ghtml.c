/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
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



#include "ghtml.h"


#if defined(WITH_GTKMOZEMBED)
# include "ghtml-gtkmozembed.c"
#elif defined(WITH_GTKHTML)
# include "ghtml-gtkhtml.c"
#elif defined(WITH_GTKTEXTVIEW)
# include "ghtml-gtktextview.c"
#elif defined(WITH_WEBKIT)
# include "ghtml-webkit.c"
#else /* default */
# include "ghtml-gtkmozembed.c"
#endif
