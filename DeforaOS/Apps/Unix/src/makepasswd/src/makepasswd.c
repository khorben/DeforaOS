/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix makepasswd */
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
/* TODO:
 * - this is a full rewrite, needs review for regressions */



#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "global.h"
#include "md5.h"
#include "../config.h"


/* makepasswd */
/* private */
/* types */
typedef struct _Prefs
{
	char const * characters;
	char const * password;
	size_t min;
	size_t max;
	int encryption;
	char const * salt;
	int iterations;
	size_t count;
} Prefs;

typedef enum _PrefsEncryption
{
	PE_NONE = 0, PE_BASE64, PE_BLOWFISH, PE_DES, PE_MD5, PE_SHA1, PE_SHA256,
	PE_SHMD5
} PrefsEncryption;
#define PE_LAST PE_SHMD5
#define PE_COUNT (PE_LAST + 1)


/* constants */
/* options */
static const char * _encryption_strings[PE_COUNT + 1] =
{
	"none", "base64", "blowfish", "des", "md5", "sha1", "sha256", "shmd5",
	NULL
};



/* prototypes */
static int _makepasswd(Prefs * prefs);
static char * _makepasswd_hash(PrefsEncryption encryption,
		char const * password, char const * salt, int iterations);
static char * _makepasswd_password(char const * characters, size_t min,
		size_t max);
static int _makepasswd_print(Prefs * prefs, char const * password,
		char const * hash);
static int _makepasswd_seed(void);

/* parsing */
static int _parse_enum(char const ** strings, char const * string);
static int _parse_unsigned(char const * string);

/* useful */
static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
/* makepasswd */
static int _makepasswd_do(Prefs * prefs);

static int _makepasswd(Prefs * prefs)
{
	size_t i;

	if(_makepasswd_seed() != 0)
		return -1;
	for(i = 0; i < prefs->count; i++)
		if(_makepasswd_do(prefs) != 0)
			return -1;
	return 0;
}

static int _makepasswd_do(Prefs * prefs)
{
	int ret = -1;
	char * password;
	char * salt = NULL;
	char * hash = NULL;

	if(prefs->password != NULL)
	{
		if((password = strdup(prefs->password)) == NULL)
			_error("malloc", 1);
	}
	else if((password = _makepasswd_password(prefs->characters, prefs->min,
					prefs->max)) == NULL)
		return -1;
	if(prefs->salt != NULL && (salt = strdup(prefs->salt)) == NULL)
	{
		free(password);
		return -_error("malloc", 1);
	}
	if(prefs->encryption != PE_NONE && (hash = _makepasswd_hash(
					prefs->encryption, password, salt,
					prefs->iterations)) == NULL)
	{
		free(password);
		free(salt);
		return -1;
	}
	if(password != NULL)
		ret = _makepasswd_print(prefs, password, hash);
	if(hash != NULL)
		memset(hash, 0, strlen(hash));
	free(hash);
	if(salt != NULL)
		memset(salt, 0, strlen(salt));
	free(salt);
	if(password != NULL)
		memset(password, 0, strlen(password));
	free(password);
	return ret;
}


/* makepasswd_hash */
static char * _hash_base64(char const * password);
static char * _hash_blowfish(char const * password, char const * salt,
		int iterations);
static char * _hash_des(char const * password, char const * salt);
static char * _hash_md5(char const * password);
static char * _hash_none(void);
static char * _hash_sha1(char const * password, char const * salt,
		int iterations);
static char * _hash_sha256(char const * password, char const * salt);
static char * _hash_shmd5(char const * password, char const * salt);

static char * _makepasswd_hash(PrefsEncryption encryption,
		char const * password, char const * salt, int iterations)
{
	switch(encryption)
	{
		case PE_BASE64:
			return _hash_base64(password);
		case PE_BLOWFISH:
			return _hash_blowfish(password, salt, iterations);
		case PE_DES:
			return _hash_des(password, salt);
		case PE_MD5:
			return _hash_md5(password);
		case PE_NONE:
			return _hash_none();
		case PE_SHA1:
			return _hash_sha1(password, salt, iterations);
		case PE_SHA256:
			return _hash_sha256(password, salt);
		case PE_SHMD5:
			return _hash_shmd5(password, salt);
		default:
			errno = ENOSYS;
			_error("encryption", 1);
			return NULL;
	}
}

static char * _hash_base64(char const * password)
{
	char * ret;
	char * r;
	const char conv[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t len;
	size_t i;
	size_t j;
	unsigned char bufi[3];

	len = strlen(password);
	if((ret = malloc(len * 4 / 3 + 4)) == NULL)
	{
		_error("malloc", 1);
		return NULL;
	}
	r = ret;
	for(i = 0; i < len;)
	{
		for(j = 0; j < 3; i++, j++)
			bufi[j] = (i < len) ? password[i] : '\0';
		*(r++) = conv[bufi[0] >> 2];
		*(r++) = conv[(bufi[0] & 0x3) << 4 | (bufi[1] >> 4)];
		*(r++) = (i - 2 < len)
			? conv[(bufi[1] & 0xf) << 2 | (bufi[2] >> 6)] : '=';
		*(r++) = (i - 1 < len) ? conv[bufi[2] & 0x3f] : '=';
	}
	*(r++) = '\0';
	return ret;
}

static char * _hash_blowfish(char const * password, char const * salt,
		int iterations)
{
	const char prefix[] = "$2a$";
	const char characters[] = "!\"%&'()*+,-./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789:;<=>?@[\\]^_`";
	char * ret;
	char * p = NULL;
	char * salt64;
	size_t len;
	char * s;

	if(iterations <= 0)
#if 0 /* XXX ideal situation */
		iterations = (rand() % 92) + 4;
#else /* more realistic */
		iterations = (rand() % 12) + 4;
#endif
	if(iterations < 4 || iterations > 95)
	{
		errno = EINVAL;
		_error("blowfish", 1);
		return NULL;
	}
	if(salt == NULL)
	{
		if((p = _makepasswd_password(characters, 16, 16)) == NULL)
			return NULL;
		salt = p;
	}
	if((salt64 = _hash_base64(salt)) == NULL)
	{
		free(p);
		return NULL;
	}
	len = sizeof(prefix) + 3 + strlen(salt64);
	if((s = malloc(len)) == NULL)
	{
		_error("malloc", 1);
		free(p);
		return NULL;
	}
	snprintf(s, len, "%s%02u%c%s", prefix, iterations, '$', salt64);
	ret = crypt(password, s);
	free(salt64);
	free(p);
	free(s);
	if(ret == NULL)
		_error("crypt", 1);
	else if(strcmp(ret, ":") == 0)
	{
		errno = EINVAL;
		_error("blowfish", 1);
		ret = NULL;
	}
	else if((ret = strdup(ret)) == NULL)
		_error("malloc", 1);
	return ret;
}

static char * _hash_des(char const * password, char const * salt)
{
	char * ret;
	const char characters[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	char * p = NULL;

	if(salt != NULL)
	{
		if(salt[0] == '$' || salt[0] == '_')
		{
			errno = EINVAL;
			_error("des", 1);
			return NULL;
		}
	}
	/* FIXME also implement the mode with '_' as first character */
	else if((p = _makepasswd_password(characters, 2, 2)) == NULL)
		return NULL;
	else
		salt = p;
	ret = crypt(password, salt);
	free(p);
	if(ret == NULL)
	{
		_error("crypt", 1);
		return NULL;
	}
	if((ret = strdup(ret)) == NULL)
		_error("malloc", 1);
	return ret;
}

static char * _hash_md5(char const * password)
{
	char * ret;
	unsigned char digest[16];
	unsigned short i;
	unsigned char j;
	MD5_CTX c;

	if((ret = malloc(33)) == NULL)
	{
		_error("malloc", 1);
		return NULL;
	}
	/* call md5 implementation from RSA Data Security, Inc. */
	MD5Init(&c);
	MD5Update(&c, password, strlen(password));
	MD5Final(digest, &c);
	/* from RFC 2617 */
	for(i = 0; i < 16; i++)
	{
		j = (digest[i] >> 4) & 0xf;
		if(j <= 9)
			ret[i * 2] = (j + '0');
		else
			ret[i * 2] = (j + 'a' - 10);
		j = digest[i] & 0xf;
		if(j <= 9)
			ret[i * 2 + 1] = (j + '0');
		else
			ret[i * 2 + 1] = (j + 'a' - 10);
	}
	ret[32] = '\0';
	return ret;
}

static char * _hash_none(void)
{
	char * ret;

	if((ret = strdup("")) == NULL)
		_error("malloc", 1);
	return ret;
}

static char * _hash_sha1(char const * password, char const * salt,
		int iterations)
{
	const char prefix[] = "$sha1$";
	const char characters[] = "!\"%&'()*+,-./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789:;<=>?@[\\]^_`";
	char * ret;
	char * p = NULL;
	size_t len;
	char * s;

	if(iterations <= 0)
#if 0 /* XXX ideal situation */
		iterations = rand() % (1 << 31);
#else /* more realistic */
		iterations = rand() % (1 << 18);
#endif
	if(salt == NULL)
	{
		if((p = _makepasswd_password(characters, 8, 64)) == NULL)
			return NULL;
		salt = p;
	}
	len = sizeof(prefix) + 11 + strlen(salt) + 1;
	if((s = malloc(len)) == NULL)
	{
		_error("malloc", 1);
		free(p);
		return NULL;
	}
	snprintf(s, len, "%s%u%c%s", prefix, iterations, '$', salt);
	ret = crypt(password, s);
	free(p);
	free(s);
	if(ret == NULL)
		_error("crypt", 1);
	else if((ret = strdup(ret)) == NULL)
		_error("malloc", 1);
	return ret;
}

static char * _hash_sha256(char const * password, char const * salt)
{
	const char prefix[] = "$5$";
	const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789./";
	char * ret;
	char * p = NULL;
	size_t len;
	char * s;

	if(salt == NULL)
	{
		if((p = _makepasswd_password(characters, 8, 64)) == NULL)
			return NULL;
		salt = p;
	}
	len = sizeof(prefix) + strlen(salt);
	if((s = malloc(len)) == NULL)
	{
		_error("malloc", 1);
		free(p);
		return NULL;
	}
	snprintf(s, len, "%s%s", prefix, salt);
	ret = crypt(password, s);
	free(p);
	free(s);
	if(ret == NULL)
		_error("crypt", 1);
	else if((ret = strdup(ret)) == NULL)
		_error("malloc", 1);
	return ret;
}

static char * _hash_shmd5(char const * password, char const * salt)
{
	const char prefix[] = "$1$";
	const char characters[] = "!\"%&'()*+,-./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789:;<=>?@[\\]^_`";
	char * ret;
	char * p = NULL;
	size_t len;
	char * s;

	if(salt == NULL)
	{
		if((p = _makepasswd_password(characters, 8, 8)) == NULL)
			return NULL;
		salt = p;
	}
	len = sizeof(prefix) + strlen(salt);
	if((s = malloc(len)) == NULL)
	{
		_error("malloc", 1);
		free(p);
		return NULL;
	}
	snprintf(s, len, "%s%s", prefix, salt);
	ret = crypt(password, s);
	free(p);
	free(s);
	if(ret == NULL)
		_error("crypt", 1);
	else if((ret = strdup(ret)) == NULL)
		_error("malloc", 1);
	return ret;
}


/* makepasswd_password */
static char * _makepasswd_password(char const * characters, size_t min,
		size_t max)
{
	char * ret;
	size_t len = min;
	size_t clen;
	size_t i;

	if(characters == NULL)
		characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()-_=+";
	if((clen = strlen(characters)) == 0)
	{
		errno = EINVAL;
		_error("password", 1);
		return NULL;
	}
	if(max > min)
		len += rand() % (max - min + 1);
	if((ret = malloc(len + 1)) == NULL)
	{
		_error("malloc", 1);
		return NULL;
	}
	for(i = 0; i <= len; i++)
		ret[i] = characters[rand() % clen];
	ret[len] = '\0';
	return ret;
}


/* makepasswd_print */
static int _makepasswd_print(Prefs * prefs, char const * password,
		char const * hash)
{
	size_t i;

	if(password == NULL)
		return -1;
	fputs(password, stdout);
	if(hash != NULL)
	{
		for(i = strlen(password); i < prefs->max; i++)
			fputc(' ', stdout);
		fputc(' ', stdout);
		fputs(hash, stdout);
	}
	fputc('\n', stdout);
	return 0;
}


/* makepasswd_seed */
static int _makepasswd_seed(void)
{
	struct timeval tv;
	int fd;
	unsigned int seed;
	ssize_t res;

	if(gettimeofday(&tv, NULL) != 0)
		return -_error("gettimeofday", 1);
	if((fd = open("/dev/random", O_RDONLY)) < 0)
		return -_error("/dev/random", 1);
	res = read(fd, &seed, sizeof(seed));
	close(fd);
	if(res != sizeof(seed))
		return -_error("/dev/random", 1);
	seed ^= tv.tv_sec ^ tv.tv_usec;
	seed ^= (getuid() << 16) ^ getgid();
	seed ^= (getpid() << 16);
	srand(seed);
	return 0;
}


/* parse_enum */
static int _parse_enum(char const ** strings, char const * string)
{
	size_t i;

	for(i = 0; strings[i] != NULL; i++)
		if(strcmp(strings[i], string) == 0)
			return i;
	return -1;
}


/* parse_unsigned */
static int _parse_unsigned(char const * string)
{
	char * p;
	long l;

	if(string == NULL)
		return -1;
	l = strtol(string, &p, 0);
	if(string[0] == '\0' || *p != '\0')
		return -1;
	return l;
}


/* error */
static int _error(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: makepasswd -ceilMnps\n"
"  -c	String of allowed characters (A-Za-z0-9`~!@#$%^&*()-_=+)\n"
"  -e	Encryption algorithm (none,base64,blowfish,des,md5,sha1,sha256,shmd5)\n"
"  -i	Number of iterations in encryption algorithm\n"
"  -l	Password length\n"
"  -M	Maximum password length\n"
"  -m	Minimum password length\n"
"  -n	Number of passwords to generate\n"
"  -p	Password to use\n"
"  -s	Salt to use\n", stderr);
	return 1;
}


/* public */
/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	prefs.count = 1;
	prefs.max = 8;
	prefs.min = 6;
	while((o = getopt(argc, argv, "c:e:i:l:M:m:n:p:s:")) != -1)
		switch(o)
		{
			case 'c':
				prefs.characters = optarg;
				break;
			case 'e':
				prefs.encryption = _parse_enum(
						_encryption_strings, optarg);
				if(prefs.encryption == -1)
					return _usage();
				break;
			case 'i':
				if((prefs.iterations = _parse_unsigned(optarg))
						<= 0)
					return _usage();
				break;
			case 'l':
				if((prefs.max = _parse_unsigned(optarg)) <= 0)
					return _usage();
				prefs.min = prefs.max;
				break;
			case 'M':
				if((prefs.max = _parse_unsigned(optarg)) <= 0)
					return _usage();
				break;
			case 'm':
				if((prefs.min = _parse_unsigned(optarg)) <= 0)
					return _usage();
				break;
			case 'n':
				if((prefs.count = _parse_unsigned(optarg)) <= 0)
					return _usage();
				break;
			case 'p':
				prefs.password = optarg;
				break;
			case 's':
				prefs.salt = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_makepasswd(&prefs) == 0) ? 0 : 2;
}
