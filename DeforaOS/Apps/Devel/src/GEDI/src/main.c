/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
/* GEDI is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * GEDI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * GEDI; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include "gedi.h"


/* main */
int main(int argc, char * argv[])
{
	GEDI * gedi;

	gtk_init(&argc, &argv);
	if((gedi = gedi_new()) == NULL)
		return 1;
	gtk_main();
	gedi_delete(gedi);
	return 0;
}
