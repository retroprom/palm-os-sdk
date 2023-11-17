/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: StringMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		String manipulation functions
 *
 *****************************************************************************/

#ifndef _STRINGMGR_H_
#define _STRINGMGR_H_

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <stdarg.h>
#include <stdio.h>						// sprintf, vsprintf

#define _Palm_va_list va_list

// Max length of string returned by StrIToA, for -2147483647, plus space
// for the terminating null.
#define	maxStrIToALen	12

#ifdef __cplusplus
extern "C" {
#endif

// String Manipulation routines

// Deprecated. Use strlen instead.
size_t StrLen(const char *src);

// Deprecated. Use strcpy instead.
char* StrCopy(char *dst, const char *src);

// Only use strncpy if you know that your source text only contains
// 7-bit ASCII text. Otherwise continue to use StrNCopy, which
// guarantees that the result doesn't end with a partial multi-
// byte character.
char* StrNCopy(char *dst, const char *src, size_t n);

// Only use strlcpy if you know that your source text only contains
// 7-bit ASCII text. Otherwise use StrLCopy, which guarantees that
// the resulting string doesn't end with a partial multi-byte
// character.
size_t StrLCopy(char *dst, const char *src, size_t siz);

// Deprecated. Use strcat instead.
char* StrCat(char *dst, const char *src);

// Only use strncat if you know that your source text only contains
// 7-bit ASCII text. Otherwise continue to use StrNcat, which
// guarantees that the result doesn't end with a partial multi-
// byte character. Also note that the meaning of <n> differs
// between strncat and StrNCat.
char* StrNCat(char *dst, const char *src, size_t n);

// Only use strlcat if you know that your source text only contains
// 7-bit ASCII text. Otherwise use StrLCat, which guarantees that
// the resulting string doesn't end with a partial multi-byte
// character.
size_t StrLCat(char *dst, const char *src, size_t siz);

// Only use strchr if you know that the string doesn't contain multi-byte
// characters, or the character you are searching for has a character code
// value less than 64 (0x40). Otherwise use StrChar, which guarantees that
// you won't match against part of a multi-byte character.
char* StrChr(const char *str, wchar32_t chr);

// Only use strstr if you know that the string being searched doesn't contain
// multi-byte characters. Otherwise use StrStr, which guarantees that
// you won't match against the beginning or end of a multi-byte character.
char* StrStr(const char *str, const char *token);

// Only use strcmp if you are checking for equality, not sorting,
// and you know that the strings don't contain multi-byte characters,
// unless the results of the comparison aren't displayed to the user.
// Otherwise use StrCompare, which provides locale-sensitive sorting
// support. Also note that strcmp returns an int (4 bytes) versus an
// int16_t from this routine.
int16_t	StrCompare(const char *s1, const char *s2);

// Only use strncmp if you are checking for equality, not sorting,
// and you know that the strings don't contain multi-byte characters,
// unless the results of the comparison aren't displayed to the user.
// Otherwise use StrNCompare, which provides locale-sensitive sorting
// support. Also note that strncmp returns an int (4 bytes) versus an
// int16_t from this routine.
int16_t	StrNCompare(const char *s1, const char *s2, size_t n);

// Only use strcasecmp if you know that both strings only contain
// 7-bit ASCII, and the results of the comparison are not displayed
// to the end user. Note that strcasecmp returns an int (4 bytes)
// versus an int16_t from this routine.
int16_t StrCaselessCompare(const char *s1, const char *s2);

// Only use strncasecmp if you know that both strings only contain
// 7-bit ASCII, and the results of the comparison are not displayed
// to the end user. Note that strncasecmp returns an int (4 bytes)
// versus an int16_t from this routine.
int16_t	StrNCaselessCompare(const char *s1, const char *s2, size_t n);

// Deprecated. Use strcmp instead. Note that strcmp returns an
// int (4 bytes) versus an int16_t from this routine.
int16_t	StrCompareAscii(const char *s1, const char *s2);

// Deprecated. Use strncmp instead. Note that strncmp returns an
// int (4 bytes) versus an int16_t from this routine.
int16_t	StrNCompareAscii(const char *s1, const char *s2, size_t n);

char* StrIToA(char *s, int32_t i);

char* StrIToH(char *s, uint32_t i);

int32_t StrAToI(const char *str);

char* StrToLower(char *dst, const char *src);

// These two routines are deprecated, and sprintf/vsprintf should be used
// instead.
int16_t 	StrPrintFV50(char *s, const char *formatStr, ...);
int16_t 	StrVPrintFV50(char *s, const char *formatStr, _Palm_va_list arg);

#define		StrPrintF		sprintf
#define		StrVPrintF		vsprintf

// Replace all occurances of ',' with <thousandSeparator> and all occurrances
// of '.' with <decimalSeparator>.
char* StrLocalizeNumber(char *s, char thousandSeparator, char decimalSeparator);

// Replace all occurances of <thousandSeparator> with ',' and all occurances
// of <decimalSeparator> with '.'.
char* StrDelocalizeNumber(char *s, char thousandSeparator, char decimalSeparator);

#ifdef __cplusplus 
}
#endif


#endif // _STRINGMGR_H_
