/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


/* printf */
static int _printf_error(char const * message, int ret);
static int _printf_unescape(char const * p);
static int _printf_format(char const ** p, char const * arg);

static int _printf(char const * format, int argc, char * argv[])
{
	char const * p;

	if(argc < 0)
	{
		errno = EINVAL;
		return _printf_error(format, 1);
	}
	for(p = format; *p != '\0'; p++)
	{
		if(*p == '\\')
		{
			p++;
			if(_printf_unescape(p) != 0)
				break;
			if(*p == '\0')
				break;
		}
		else if(*p != '%')
			putc(*p, stdout);
		else if(*(p + 1) == '%')
			putc(*++p, stdout);
		else if(argc == 0)
		{
			errno = EINVAL; /* XXX find a better error message */
			return _printf_error(p, 1);
		}
		else
		{
			p++;
			argc--;
			if(_printf_format(&p, *(argv++)) != 0)
				break;
			if(*p == '\0')
				break;
		}
	}
	if(*p != '\0')
		return 1;
	if(argc != 0)
	{
		errno = E2BIG;
		return _printf_error(format, 1);
	}
	return 0;
}

static int _printf_error(char const * message, int ret)
{
	fputs("printf: ", stderr);
	perror(message);
	return ret;
}

static int _printf_unescape(char const * p)
{
	switch(*p)
	{
		case '\0':
			break;
		case 'a':
			putc('\a', stdout);
			break;
		case 'b':
			putc('\b', stdout);
			break;
		case 'e':
			putc('\x1b', stdout);
			break;
		case 'f':
			putc('\f', stdout);
			break;
		case 'n':
			putc('\n', stdout);
			break;
		case 'r':
			putc('\r', stdout);
			break;
		case 't':
			putc('\t', stdout);
			break;
		case 'v':
			putc('\v', stdout);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'x': /* FIXME implement */
			errno = ENOSYS;
			return _printf_error(p, 1);
		default:
			putc(*p, stdout);
			break;
	}
	return 0;
}

static int _printf_format(char const ** p, char const * arg)
{
	long i;
	unsigned long u;

	switch(**p)
	{
		case 'c':
			if(fputc(arg[0], stdout) != arg[0])
				return _printf_error("stdout", 1);
			break;
		case 's':
			if(fputs(arg, stdout) != 0)
				return _printf_error("stdout", 1);
			break;
		case 'd':
		case 'i':
			i = atoi(arg);
			printf("%ld", i);
			break;
		case 'u':
			u = strtoul(arg, NULL, 10);
			printf("%lu", u);
			break;
		case 'x':
			u = strtoul(arg, NULL, 10);
			printf("%lx", u);
			break;
		case 'X':
			u = strtoul(arg, NULL, 10);
			printf("%lX", u);
			break;
		case '-':
		case '+':
		case '#':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
		case 'o':
			errno = ENOSYS;
			return _printf_error(*p, 1);
		default:
			errno = EINVAL;
			return _printf_error(*p, 1);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: printf format [argument...]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	if((o = getopt(argc, argv, "")) != -1)
		return _usage();
	if(optind == argc)
		return _usage();
	return _printf(argv[optind], argc - optind - 1, &argv[optind + 1]) == 0
		? 0 : 2;
}
