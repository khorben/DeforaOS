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



#include "../src/gsm.c"


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


/* main */
int main(int argc, char * argv[])
{
	time_t timestamp;
	char number[32];
	GSMEncoding encoding;
	char * p;
	struct tm t;
	char buf[32];
	size_t len;

	if(argc != 2)
		return 1;
	if((p = _cmgr_pdu_parse(argv[1], &timestamp, number, &encoding, &len))
			!= NULL)
	{
		printf("Number: %s\n", number);
		localtime_r(&timestamp, &t);
		strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &t);
		printf("Timestamp: %s\n", buf);
		printf("Encoding: %u\n", encoding);
		_hexdump(p, len);
		if(encoding == GSM_ENCODING_UTF8)
			printf("Message: %s\n", p);
		free(p);
	}
	return 0;
}
