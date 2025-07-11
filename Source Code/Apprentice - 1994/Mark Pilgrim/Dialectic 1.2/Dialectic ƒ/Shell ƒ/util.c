/**********************************************************************\

File:		util.c

Purpose:	This module handles standard memory copying and comparing.

\**********************************************************************/

#include "util.h"

void Mymemcpy(Ptr output, Ptr input, unsigned long len)
/* like unix memcpy -- output first, then input, then length */
{
	BlockMove(input, output, len);
}

void Mymemset(Ptr output, unsigned char value, unsigned long len)
/* Good for zeroing sensitive data.  Not that I have anything sensitive, of course ;) */
{
	unsigned long	i;
	
	for (i=0; i<len; i++)
		*((unsigned char*)((long)output+i))=value;
}

Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned char len)
/* standard memory compare, byte for byte */
{
	unsigned char	i;
	Boolean			goon;
	
	goon=TRUE;
	for (i=0; (i<len) && (goon); i++)
		goon=(*((unsigned char*)((long)thisThing+i))==*((unsigned char*)((long)thatThing+i)));
	return goon;
}
