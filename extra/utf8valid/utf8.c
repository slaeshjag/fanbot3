/*
Copyright (c) 2011-2013 Steven Arnow
'utf8.c' - This file is part of libdarnit

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/

/* NOTE: Modified release, some functions removed */

#include "utf8.h"


/* Internal, should not be called from your code */
int utf8GetCharLength(const unsigned char *str) {
	if (!((*str & 0xC0) ^ 0x80))
		return 0;
	if (!(*str & 0x80))
		return 1;
	if (!((*str & 0xE0) ^ 0xC0))
		return 2;
	if (!((*str & 0xF0) ^ 0xE0))
		return 3;
	if (!((*str & 0xF8) ^ 0xF0))
		return 4;
	
	return 0;
}


/* Internal, should not be called from your code */
int utf8Validate(const unsigned char *str) {
	int i, j;

	j = utf8GetCharLength(str);
	if (j == 0)
		return -1;
	
	for (i = 1; i < j; i++)
		if (((str[i] & 0xC0) ^ 0x80))
			return -1;
	if (str[0] == 0xC0 || str[i] == 0xC1)
		return -1;
	if (str[0] >= 0xF5)
		return -1;
	
	return 0;
}


/* Okay to call */
unsigned int utf8GetChar(const char *str_s) {
	const unsigned char *str = (const unsigned char *) str_s;
	int i;
	unsigned int chr, len, shift;

	if ((len = utf8GetCharLength(str)) == 0)
		return UTF8_REPLACEMENT_CHAR;
	if (!utf8Validate(str) == -1)
		return UTF8_REPLACEMENT_CHAR;

	shift = 1;
	if (len > 1) shift += len;
		chr = (*str & (0xFF >> shift));
	chr <<= (len-1) * 6;

	for (i = 1; i < len; i++) 
		chr += ((unsigned int) str[i] & 0x3F) << (len - i - 1) * 6;

	return (chr > UTF8_CHAR_LIMIT) ? UTF8_REPLACEMENT_CHAR : chr;
}


/* Okay to call */
int utf8GetValidatedCharLength(const char *str_s) {
	int len;

	if (*str_s == 0)
		return 0;
	
	len = utf8GetCharLength((const unsigned char *) str_s);
	if (utf8Validate((const unsigned char *) str_s) == -1)
		return 1;
	
	return (len > 0) ? len : 1;
}

