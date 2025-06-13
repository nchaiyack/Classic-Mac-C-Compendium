
/*
 *  stringx.c
 *
 *	miscellaneous string functions
 *
 *  Copyright (c) 1994 Ithran Einhorn.  All rights reserved.
 *
 */


#include "stringx.h"

/******************************************************************************
 stricmp

		case insensitive string compare.
 ******************************************************************************/
 
int stricmp(const char * s1, const char *s2)
{
	while (*s1 && *s2 && (*s1 & 0xDF) == (*s2 & 0xDF))
	{
		s1++; s2++;
	}
	return (int)(*s1 - *s2);
}

/******************************************************************************
 TrimRight

		calculate number of characters in TM buffer.
 ******************************************************************************/
 
char *TrimRight(char *t_str)
{
	register long pos;
	
	for (pos = strlen(t_str) - 1; pos >= 0 && isspace(t_str[pos]); pos--)
		 t_str[pos] = 0;
			
	return t_str;
}

