#ifndef CORE_STRING_H
#define CORE_STRING_H

#include "scalar.h"

#include <stdarg.h>
#include <stdlib.h>

enum string_format_flag {
	STRING_FORMAT_FLAG_ALT      =  1, /* '#' */
	STRING_FORMAT_FLAG_SPACE    =  2, /* ' ' */
	STRING_FORMAT_FLAG_LEFT     =  4, /* '-' */
	STRING_FORMAT_FLAG_SHOWSIGN =  8, /* '+' */
	STRING_FORMAT_FLAG_ZERO     = 16, /* '0' */
	STRING_FORMAT_FLAG_PARSE    = 32
};

enum string_format_length {
	STRING_FORMAT_LENGTH_CHAR,      /* hh */
	STRING_FORMAT_LENGTH_SHORT,     /* h */
	STRING_FORMAT_LENGTH_NONE,
	STRING_FORMAT_LENGTH_LONG,      /* l */
	STRING_FORMAT_LENGTH_LONGLONG,  /* ll */
	STRING_FORMAT_LENGTH_LONGDOUBLE /* L */
};

struct format_info {
	u4                        type;
	s4                        precision;
	s4                        width;
	enum string_format_flag   flag;
	u4                        pad;
	enum string_format_length length;
};

size_t string_formatv(char *, size_t, const char *, va_list);
size_t string_format(char *, size_t, const char *, ...);
const char *string_from_formatv(const char *, va_list);
const char *string_from_format(const char *, ...);
const char *strings_join(int, char *const*, const char *);
int string_icmp(const char *, const char *);
const char *string_lower(const char *);

#define string_dup(X) string_from_format("%s", X)
#define string_sub(X, START, LENGTH) string_from_format("%.*s", LENGTH, X + START)
#define with_string_format(NAME, FORMAT, ...) for(const char *NAME = string_from_format(FORMAT, ## __VA_ARGS__); NAME; NAME = NULL)

#endif
