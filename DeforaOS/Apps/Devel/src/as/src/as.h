/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef AS_AS_H
# define AS_AS_H

# define AS_FILENAME_DEFAULT "a.out"


/* functions */
/* useful */
int as_error(char const * msg, int ret);

/* plugins helpers */
void * as_plugin_new(char const * type, char const * name,
		char const * description);
void as_plugin_delete(void * plugin);
void as_plugin_list(char const * type, char const * description);

#endif /* !AS_AS_H */
