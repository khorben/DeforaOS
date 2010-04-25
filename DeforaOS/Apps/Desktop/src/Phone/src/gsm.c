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
typedef enum _GSMStatus
{
	GS_INIT, GS_COMMAND
} GSMStatus;

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
	GSMStatus status;

	/* internal */
	guint source;
	guint rd_io;
	guint wr_io;
};


/* variables */
/* CME ERROR */
static struct
{
	int code;
	char const * error;
} _gsm_cme_errors[] =
{
	{	0,	"Phone failure"					},
	{	1,	"No connection to phone"			},
	{	3,	"Operation not allowed"				},
	{	4,	"Operation not supported"			},
	{	10,	"SIM not inserted"				},
	{	11,	"SIM PIN required"				},
	{	12,	"SIM PUK required"				},
	{	13,	"SIM failure"					},
	{	14,	"SIM busy"					},
	{	15,	"SIM wrong"					},
	{	20,	"Memory full"					},
	{	21,	"Invalid index"					},
	{	30,	"No network service"				},
	{	31,	"Network timeout"				},
	{	32,	"Network not allowed - emergency calls only"	},
	{	0,	NULL						}
};


/* prototypes */
static int _gsm_parse(GSM * gsm);

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
	gsm->status = GS_INIT;
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
	if(number == NULL)
		return gsm_modem_call_last(gsm);
	return gsm_modem_call(gsm, number);
}


/* gsm_hangup */
int gsm_hangup(GSM * gsm)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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
	len = sizeof(cmd) + strlen(number) + 3;
	if((buf = malloc(len)) == NULL)
		return phone_error(NULL, "malloc", 1);
	snprintf(buf, len, "%s%s;\r\n", cmd, number);
	ret = gsm_modem_queue(gsm, buf);
	free(buf);
	return ret;
}


/* gsm_modem_call_last */
int gsm_modem_call_last(GSM * gsm)
{
	char const cmd[] = "ATDL;\r\n";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return gsm_modem_queue(gsm, cmd);
}


/* gsm_modem_hangup */
int gsm_modem_hangup(GSM * gsm)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return gsm_modem_queue(gsm, "ATH\r\n");
}


/* gsm_modem_is_pin_needed */
int gsm_modem_is_pin_needed(GSM * gsm)
{
	char const cmd[] = "AT+CPIN?\r\n";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return gsm_modem_queue(gsm, cmd);
}


/* gsm_modem_queue */
int gsm_modem_queue(GSM * gsm, char const * command)
{
	size_t len = strlen(command);
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, command);
#endif
	if(gsm->status != GS_COMMAND)
		return 1; /* XXX queue for later instead */
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
	int ret;
	char const cmd[] = "ATZ\r\n";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* TODO
	 * - queue all commands in sequence
	 * - prepend this one to the list (flush the others?) */
	if(gsm->status == GS_INIT) /* XXX crude hack */
	{
		gsm->status = GS_COMMAND;
		ret = gsm_modem_queue(gsm, cmd);
		gsm->status = GS_INIT;
		return ret;
	}
	return gsm_modem_queue(gsm, cmd);
}


/* gsm_modem_send_dtmf */
int gsm_modem_send_dtmf(GSM * gsm, char const * sequence)
{
	int ret;
	char const cmd[] = "AT+VTS=";
	size_t len = sizeof(cmd) + strlen(sequence) + 2;
	char * buf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, sequence);
#endif
	if(!_is_number(sequence)) /* XXX is '+' allowed? */
		return 1;
	if((buf = malloc(len)) == NULL)
		return 1;
	snprintf(buf, len, "%s%s\r\n", cmd, sequence);
	ret = gsm_modem_queue(gsm, buf);
	free(buf);
	return ret;
}


/* gsm_modem_set_echo */
int gsm_modem_set_echo(GSM * gsm, int echo)
{
	char cmd[] = "ATE?\r\n";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, (echo != 0) ? "TRUE"
			: "FALSE");
#endif
	cmd[3] = (echo != 0) ? '1' : '0';
	return gsm_modem_queue(gsm, cmd);
}


/* gsm_modem_set_pin */
int gsm_modem_set_pin(GSM * gsm, int oldpin, int newpin)
{
	int ret;
	char cmd[] = "AT+CPIN=";
	size_t len = sizeof(cmd) + 11;
	char * buf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d)\n", __func__, oldpin, newpin);
#endif
	if(oldpin < 999 || oldpin > 9999 || newpin < 999 || newpin > 9999)
		return 1; /* XXX probably limiting */
	if((buf = malloc(len)) == NULL)
		return 1;
	snprintf(buf, len, "%s%d,%d\r\n", cmd, oldpin, newpin);
	ret = gsm_modem_queue(gsm, buf);
	free(buf);
	return ret;
}


/* gsm_reset */
void gsm_reset(GSM * gsm, unsigned int delay)
{
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	free(gsm->rd_buf);
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	free(gsm->wr_buf);
	gsm->wr_buf = NULL;
	gsm->wr_buf_cnt = 0;
	gsm->status = GS_INIT;
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
/* gsm_parse */
static int _parse_init(GSM * gsm, char const * line);
static int _parse_command(GSM * gsm, char const * line);

static int _gsm_parse(GSM * gsm)
{
	int ret = 0;
	size_t i = 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(i < gsm->rd_buf_cnt)
	{
		if(gsm->rd_buf[i++] != '\r')
			continue;
		if(i == gsm->rd_buf_cnt)
			break;
		if(gsm->rd_buf[i] != '\n')
			continue;
		gsm->rd_buf[i++ - 1] = '\0';
		if(gsm->rd_buf[0] != '\0')
			switch(gsm->status)
			{
				case GS_INIT:
					ret |= _parse_init(gsm, gsm->rd_buf);
					break;
				case GS_COMMAND:
					ret |= _parse_command(gsm, gsm->rd_buf);
					break;
			}
		gsm->rd_buf_cnt -= i;
		memmove(gsm->rd_buf, &gsm->rd_buf[i], gsm->rd_buf_cnt);
		if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt)) != NULL)
			gsm->rd_buf = p; /* we can ignore errors */
		i = 0;
	}
	return ret;
}

static int _parse_init(GSM * gsm, char const * line)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(strcmp(line, "OK") != 0)
		return 0;
	g_source_remove(gsm->source);
	gsm->source = 0;
	gsm->status = GS_COMMAND;
	gsm_modem_set_echo(gsm, FALSE);
	gsm_modem_is_pin_needed(gsm);
	return 0;
}

static int _command_cme_error(GSM * gsm, char const * line);
static int _command_cpin(GSM * gsm, char const * line);
static int _parse_command(GSM * gsm, char const * line)
{
	char const cme_error[] = "+CME ERROR: ";
	char const cpin[] = "+CPIN: ";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(strcmp(line, "OK") == 0)
		return 0;
	if(strncmp(line, "AT", 2) == 0) /* ignore echo */
		return 0;
	if(strncmp(line, cme_error, sizeof(cme_error) - 1) == 0)
		return _command_cme_error(gsm, &line[sizeof(cme_error) - 1]);
	if(strncmp(line, cpin, sizeof(cpin) - 1) == 0)
		return _command_cpin(gsm, &line[sizeof(cpin) - 1]);
	/* FIXME implement */
	fprintf(stderr, "%s%s%s", "phone: ", line, ": Unknown answer\n");
	return 1;
}

static int _command_cme_error(GSM * gsm, char const * line)
{
	int code;
	char * p;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	code = strtol(line, &p, 10);
	if(line[0] == '\0' || *p != '\0')
	{
		fprintf(stderr, "%s%s%s", "phone: ", line,
				": Invalid CME error code\n");
		return 1;
	}
	for(i = 0; _gsm_cme_errors[i].error != NULL; i++)
		if(_gsm_cme_errors[i].code == code)
			break;
	if(_gsm_cme_errors[i].error == NULL)
	{
		fprintf(stderr, "%s%s%s", "phone: ", line,
				": Unknown CME error code\n");
		return 1;
	}
	/* FIXME implement callbacks */
	printf("%s%s\n", "phone: ", _gsm_cme_errors[i].error);
	return 0;
}

static int _command_cpin(GSM * gsm, char const * line)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(strcmp(line, "READY") == 0)
		return 0;
	/* FIXME implement */
	return 1;
}


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
static gboolean _reset_settle(gpointer data);

static gboolean _on_reset(gpointer data)
{
	GSM * gsm = data;
	int fd;
	GError * error = NULL;

	gsm->source = 0;
	if((fd = open(gsm->device, O_RDWR | O_NONBLOCK)) < 0)
	{
		if(gsm->retry > 0)
			gsm->source = g_timeout_add(gsm->retry, _on_reset, gsm);
		return phone_error(NULL, "open", FALSE);
	}
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
	gsm->source = g_timeout_add(500, _reset_settle, gsm);
	_reset_settle(gsm);
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

static gboolean _reset_settle(gpointer data)
{
	GSM * gsm = data;

	gsm_modem_reset(gsm);
	return TRUE;
}


/* on_watch_read */
static gboolean _on_watch_read(GIOChannel * source, GIOCondition condition,
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
	if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt + 256)) == NULL)
		return FALSE; /* FIXME trouble here... */
	gsm->rd_buf = p;
	if(condition != G_IO_IN || source != gsm->channel)
		return FALSE;
	status = g_io_channel_read_chars(source, &gsm->rd_buf[gsm->rd_buf_cnt],
			256, &cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: modem: ");
	fwrite(&gsm->rd_buf[gsm->rd_buf_cnt], 1, cnt, stderr);
#endif
	gsm->rd_buf_cnt += cnt;
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
	_gsm_parse(gsm);
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
	status = g_io_channel_write_chars(source, gsm->wr_buf, gsm->wr_buf_cnt,
			&cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: phone: ");
	fwrite(gsm->wr_buf, sizeof(*gsm->wr_buf), cnt, stderr);
#endif
	if(cnt != 0) /* some data may have been written anyway */
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
