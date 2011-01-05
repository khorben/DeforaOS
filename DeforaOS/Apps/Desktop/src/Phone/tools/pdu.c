/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include "../src/gsm.c"
#include "../src/modem.c"


/* pdu */
/* prototypes */
static int _pdu_decode(char const * string);
static int _pdu_encode(char const * number, char const * string);

static void _hexdump(char const * buf, size_t len);


/* functions */
/* pdu_decode */
static int _pdu_decode(char const * string)
{
	time_t timestamp;
	char number[32];
	GSMEncoding encoding;
	char * p;
	struct tm t;
	char buf[32];
	size_t len;

	if((p = _cmgr_pdu_parse(string, &timestamp, number, &encoding, &len))
			== NULL)
	{
		fputs("pdu: Unable to decode PDU\n", stderr);
		return -1;
	}
	printf("Number: %s\n", number);
	localtime_r(&timestamp, &t);
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &t);
	printf("Timestamp: %s\n", buf);
	printf("Encoding: %u\n", encoding);
	_hexdump(p, len);
	if(encoding == GSM_ENCODING_UTF8)
		printf("Message: %s\n", p);
	free(p);
	return 0;
}

static int _pdu_encode(char const * number, char const * string)
{
	char * cmd;
	char * pdu;

	if(_message_to_pdu(0, number, GSM_MODEM_ALPHABET_DEFAULT, string,
				strlen(string), &cmd, &pdu) != 0)
	{
		fputs("pdu: Unable to encode PDU\n", stderr);
		return -1;
	}
	puts(cmd);
	puts(pdu);
	free(cmd);
	free(pdu);
	return 0;
}


/* hexdump */
static void _hexdump(char const * buf, size_t len)
{
	unsigned char const * b = (unsigned char *)buf;
	size_t i;

	for(i = 0; i < len; i++)
	{
		printf(" %02x", b[i]);
		if((i % 16) == 7)
			fputc(' ', stdout);
		else if((i % 16) == 15)
			fputc('\n', stdout);
	}
	fputc('\n', stdout);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: pdu -d pdu\n"
"       pdu -e -n number text\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int op = 0;
	char const * number = NULL;

	while((o = getopt(argc, argv, "den:")) != -1)
		switch(o)
		{
			case 'd':
			case 'e':
				op = o;
				break;
			case 'n':
				number = optarg;
				break;
			default:
				return _usage();
		}
	if(op == 0 || optind + 1 != argc || (op == 'e' && number == NULL))
		return _usage();
	return ((op == 'd') ? _pdu_decode(argv[optind])
			: _pdu_encode(number, argv[optind])) == 0 ? 0 : 2;
}
