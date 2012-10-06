/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef SURFER_DOWNLOAD_H
# define SURFER_DOWNLOAD_H


/* Download */
/* types */
typedef struct _Download Download;

typedef struct _DownloadPrefs
{
	int embedded;
	char * output;
	char * user_agent;
} DownloadPrefs;


/* functions */
Download * download_new(DownloadPrefs * prefs, char const * url);
void download_delete(Download * download);

/* accessors */
void download_set_close(Download * download, gboolean close);

/* useful */
int download_cancel(Download * download);

#endif /* !SURFER_DOWNLOAD_H */
