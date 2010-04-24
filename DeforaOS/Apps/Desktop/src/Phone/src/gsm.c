/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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



#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include "phone.h"
#include "gsm.h"


/* GSM */
/* private */
/* types */
struct _GSM
{
	char * device;
	unsigned int baudrate;
	unsigned int retry;
	GIOChannel * channel;
	char * rd_buf;
	size_t rd_buf_cnt;
	char * wr_buf;
	size_t wr_buf_cnt;

	/* internal */
	guint source;
	guint rd_io;
	guint wr_io;
};


/* prototypes */
static int _is_figure(int c);
static int _is_number(char const * number);

/* callbacks */
static gboolean _on_reset(gpointer data);

static gboolean _on_watch_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* functions */
/* gsm_new */
GSM * gsm_new(char const * device, unsigned int baudrate)
{
	GSM * gsm;

	if(device == NULL)
		return NULL;
	if((gsm = malloc(sizeof(*gsm))) == NULL)
		return NULL;
	gsm->device = strdup(device);
	gsm->baudrate = baudrate;
	gsm->retry = 1000;
	gsm->channel = NULL;
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	gsm->wr_buf = NULL;
	gsm->wr_buf_cnt = 0;
	gsm->source = 0;
	gsm->rd_io = 0;
	gsm->wr_io = 0;
	if(gsm->device == NULL)
	{
		gsm_delete(gsm);
		return NULL;
	}
	gsm_reset(gsm, 0);
	return gsm;
}


/* gsm_delete */
void gsm_delete(GSM * gsm)
{
	free(gsm->device);
	if(gsm->channel != NULL)
	{
		g_io_channel_shutdown(gsm->channel, TRUE, NULL);
		g_io_channel_unref(gsm->channel);
	}
	free(gsm->rd_buf);
	free(gsm->wr_buf);
	if(gsm->source != 0)
		g_source_remove(gsm->source);
	if(gsm->rd_io != 0)
		g_source_remove(gsm->rd_io);
	if(gsm->wr_io != 0)
		g_source_remove(gsm->wr_io);
	free(gsm);
}


/* accessors */
/* gsm_get_retry */
unsigned int gsm_get_retry(GSM * gsm)
{
	return gsm->retry;
}


/* gsm_set_retry */
void gsm_set_retry(GSM * gsm, unsigned int retry)
{
	gsm->retry = retry;
}


/* useful */
/* gsm_call */
int gsm_call(GSM * gsm, char const * number)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, number);
#endif
	/* FIXME check current status before calling */
	return gsm_modem_call(gsm, number);
}


/* gsm_hangup */
int gsm_hangup(GSM * gsm)
{
	/* FIXME check current status before hanging up */
	return gsm_modem_hangup(gsm);
}


/* gsm_modem_call */
int gsm_modem_call(GSM * gsm, char const * number)
{
	int ret;
	char const cmd[] = "ATD";
	size_t len;
	char * buf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, number);
#endif
	if(!_is_number(number))
		return 1;
	len = sizeof(cmd) + strlen(number) + 2;
	if((buf = malloc(len)) == NULL)
		return phone_error(NULL, "malloc", 1);
	snprintf(buf, len, "%s%s\r\n", cmd, number);
	ret = gsm_modem_queue(gsm, buf);
	free(buf);
	return ret;
}


/* gsm_modem_hangup */
int gsm_modem_hangup(GSM * gsm)
{
	return gsm_modem_queue(gsm, "\r\nATH\r\n");
}


/* gsm_modem_queue */
int gsm_modem_queue(GSM * gsm, char const * command)
{
	size_t len = strlen(command);
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, command);
#endif
	if((p = realloc(gsm->wr_buf, gsm->wr_buf_cnt + len)) == NULL)
		return phone_error(NULL, "malloc", 1);
	gsm->wr_buf = p;
	memcpy(&gsm->wr_buf[gsm->wr_buf_cnt], command, len);
	gsm->wr_buf_cnt += len;
	if(gsm->channel != NULL && gsm->wr_buf_cnt > 0 && gsm->wr_io == 0)
		gsm->wr_io = g_io_add_watch(gsm->channel, G_IO_OUT,
				_on_watch_write, gsm);
	return 0;
}


/* gsm_modem_reset */
int gsm_modem_reset(GSM * gsm)
{
	/* FIXME queue commands in sequence and prepend this one to the list */
	return gsm_modem_queue(gsm, "\r\nATZ\r\n");
}


/* gsm_reset */
void gsm_reset(GSM * gsm, unsigned int delay)
{
	GError * error = NULL;

	free(gsm->rd_buf);
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	free(gsm->wr_buf);
	gsm->wr_buf = NULL;
	gsm->wr_buf_cnt = 0;
	if(gsm->source != 0)
	{
		g_source_remove(gsm->source);
		gsm->source = 0;
	}
	if(gsm->rd_io != 0)
	{
		g_source_remove(gsm->rd_io);
		gsm->rd_io = 0;
	}
	if(gsm->wr_io != 0)
	{
		g_source_remove(gsm->wr_io);
		gsm->wr_io = 0;
	}
	if(gsm->channel != NULL)
	{
		g_io_channel_shutdown(gsm->channel, TRUE, &error);
		g_io_channel_unref(gsm->channel);
		gsm->channel = NULL;
	}
	if(delay > 0)
		gsm->source = g_timeout_add(delay, _on_reset, gsm);
	else
		gsm->source = g_idle_add(_on_reset, gsm);
}


/* private */
/* functions */
/* is_figure */
static int _is_figure(int c)
{
	if(c >= '0' && c <= '9')
		return 1;
	if(c == '*' || c == '+' || c == '#')
		return 1;
	return 0;
}


/* is_number */
static int _is_number(char const * number)
{
	if(number == NULL || number[0] == '\0')
		return 0;
	while(*number != '\0')
		if(!_is_figure(*(number++)))
			return 0;
	return 1;
}


/* callbacks */
/* on_reset */
static int _reset_do(GSM * gsm, int fd);

static gboolean _on_reset(gpointer data)
{
	GSM * gsm = data;
	int fd;
	GError * error = NULL;

	gsm->source = 0;
	if((fd = open(gsm->device, O_RDWR | O_NONBLOCK)) < 0)
		return phone_error(NULL, "open", 1);
	if(_reset_do(gsm, fd) != 0)
	{
		close(fd);
		if(gsm->retry > 0)
			gsm->source = g_timeout_add(gsm->retry, _on_reset, gsm);
		return FALSE;
	}
	gsm->channel = g_io_channel_unix_new(fd);
	if((g_io_channel_set_encoding(gsm->channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
		/* XXX ugly */
		fprintf(stderr, "ERROR: %s() g_io_channel_set_encoding\n",
				__func__);
	g_io_channel_set_buffered(gsm->channel, FALSE);
	gsm->rd_io = g_io_add_watch(gsm->channel, G_IO_IN, _on_watch_read, gsm);
	gsm_modem_reset(gsm);
	return FALSE;
}

static int _reset_do(GSM * gsm, int fd)
{
	struct stat st;
	int fl;
	struct termios term;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	if(flock(fd, LOCK_EX | LOCK_NB) != 0)
		return phone_error(NULL, "flock", 1);
	if(fstat(fd, &st) != 0)
		return phone_error(NULL, "fstat", 1);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d) mode 0%o\n", __func__, fd, st.st_mode);
#endif
	if(st.st_mode & S_IFCHR) /* character special */
	{
		if(tcgetattr(fd, &term) != 0)
			return phone_error(NULL, "tcgetattr", 1);
		switch(gsm->baudrate) /* XXX rewrite this a nicer way */
		{
			case 4800:
				term.c_cflag = B4800;
				break;
			case 9600:
				term.c_cflag = B9600;
				break;
			case 19200:
				term.c_cflag = B19200;
				break;
			case 38400:
				term.c_cflag = B38400;
				break;
			case 115200:
				term.c_cflag = B115200;
				break;
			default:
				errno = EINVAL;
				return phone_error(NULL, "baudrate", 1);
		}
		term.c_cflag |= CS8;
		term.c_cflag |= CREAD;
		term.c_cflag |= CLOCAL;
		term.c_iflag = (IGNPAR | IGNBRK);
		term.c_lflag = 0;
		term.c_oflag = 0;
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		if(tcsetattr(fd, TCSAFLUSH, &term) != 0)
			return phone_error(NULL, "tcsetattr", 1);
	}
	fl = fcntl(fd, F_GETFL, 0);
	if(fcntl(fd, F_SETFL, fl & ~O_NONBLOCK) == -1)
		return phone_error(NULL, "fcntl", 1);
	return 0;
}


/* on_watch_read */
static gboolean _on_watch_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	GSM * gsm = data;
	char buf[256];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, condition);
#endif
	if(condition != G_IO_IN || source != gsm->channel)
		return FALSE;
	/* FIXME really implement */
	status = g_io_channel_read_chars(source, buf, sizeof(buf), &cnt,
			&error);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__, cnt);
#endif
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			/* XXX report error, do not break */
			fprintf(stderr, "%s%s\n", "phone: read: ",
					error->message);
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			if(gsm->retry > 0)
				gsm_reset(gsm, gsm->retry);
			return FALSE;
	}
	/* FIXME parse and interpret the output */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"", __func__);
	fwrite(buf, sizeof(*buf), cnt, stderr);
	fputs("\"\n", stderr);
#endif
	return TRUE;
}


/* on_watch_write */
static gboolean _on_watch_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	GSM * gsm = data;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, condition);
#endif
	if(condition != G_IO_OUT || source != gsm->channel)
		return FALSE;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() write %zu\n", __func__, gsm->wr_buf_cnt);
#endif
	status = g_io_channel_write_chars(source, gsm->wr_buf, gsm->wr_buf_cnt,
			&cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() wrote %lu \"", __func__, cnt);
	fwrite(gsm->wr_buf, sizeof(*gsm->wr_buf), cnt, stderr);
	fputs("\"\n", stderr);
#endif
	if(cnt != 0) /* data may have be written anyway */
	{
		memmove(gsm->wr_buf, &gsm->wr_buf[cnt], gsm->wr_buf_cnt - cnt);
		gsm->wr_buf_cnt -= cnt;
		if((p = realloc(gsm->wr_buf, gsm->wr_buf_cnt)) != NULL)
			gsm->wr_buf = p;
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			/* XXX report error, do not break */
#ifdef DEBUG
			perror("phone: write");
#else
			fprintf(stderr, "%s%s\n", "phone: write: ",
					error->message);
#endif
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			if(gsm->retry > 0)
				gsm_reset(gsm, gsm->retry);
			return FALSE;
	}
	if(gsm->wr_buf_cnt > 0) /* there is more data to write */
		return TRUE;
	gsm->wr_io = 0;
	return FALSE;
}
