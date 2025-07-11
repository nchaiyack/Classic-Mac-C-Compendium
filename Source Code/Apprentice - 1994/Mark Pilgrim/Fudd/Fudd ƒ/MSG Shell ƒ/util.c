/**********************************************************************\

File:		util.c

Purpose:	This module handles standard memory copying and comparing.


Fudd -=- convert text to Elmer Fudd talk
Copyright �1994, Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "util.h"

void Mymemcpy(Ptr output, Ptr input, unsigned long len)
{
	unsigned long	i;
	
	for (i=0; i<len; i++)
		*((unsigned char*)((long)output+i))=*((unsigned char*)((long)input+i));
}

void Mymemset(Ptr output, unsigned char value, unsigned long len)
{
	unsigned long	i;
	
	for (i=0; i<len; i++)
		*((unsigned char*)((long)output+i))=value;
}

Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned char len)
{
	unsigned char	i;
	Boolean			goon;
	
	goon=TRUE;
	for (i=0; (i<len) && (goon); i++)
		goon=(*((unsigned char*)((long)thisThing+i))==*((unsigned char*)((long)thatThing+i)));
	return goon;
}
