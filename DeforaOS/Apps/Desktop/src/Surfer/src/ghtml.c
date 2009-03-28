/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



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
