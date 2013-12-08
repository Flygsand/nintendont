#ifndef _STRING_H_
#define _STRING_H_

#include <stdarg.h>
#include "global.h"
#include "common.h"
#include "vsprintf.h"

char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
size_t strlcpy(char *dest, const char *src, size_t maxlen);
char * strstr ( const char *str1, const char *str2);
int strcmp(const char *, const char *);
int strncmp(const char *p, const char *q, size_t n);
size_t strlen(const char *);
size_t strnlen(const char *, size_t);
char *strchr(const char *s, int c);
int memcmp(const void *s1, const void *s2, size_t n);
int sprintf(char *str, const char *fmt, ...);

extern void memcpy( void *dst, const void *src, u32 size);

int _sprintf( char *buf, const char *fmt, ... );
void hexdump(void *d, int len);

#endif
