/**********************************************************************\

File:		util.c

Purpose:	This module handles standard memory copying and comparing.


Pentominoes - a 2-D geometry board game
Copyright (C) 1993 Mark Pilgrim

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

void Mymemcpy(Ptr output, Ptr input, unsigned char len)
{
	unsigned char	i;
	
	for (i=0; i<len; i++)
		output[i]=input[i];
}

void Mymemset(Ptr output, int value, unsigned char len)
{
	unsigned char	i;
	
	for (i=0; i<len; i++)
		((char*)output)[i]=(char)value;
}
