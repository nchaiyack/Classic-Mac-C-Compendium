/**********************************************************************\

File:		v hex.c

Purpose:	This module handles hex numbers as strings and strings as
			hex numbers.


Voyeur -- a no-frills file viewer
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

#include "v hex.h"
#include "v structs.h"

void LongToHexString(unsigned long input, Str255 result)
{
	result[0]=0x08;
	result[1]=hexchar[0][(input>>24)&0xff];
	result[2]=hexchar[1][(input>>24)&0xff];
	result[3]=hexchar[0][(input>>16)&0xff];
	result[4]=hexchar[1][(input>>16)&0xff];
	result[5]=hexchar[0][(input>>8)&0xff];
	result[6]=hexchar[1][(input>>8)&0xff];
	result[7]=hexchar[0][input&0xff];
	result[8]=hexchar[1][input&0xff];
}

Boolean ValidHex(Str255 tempStr)
{
	int			i;
	
	for (i=1; i<=tempStr[0]; i++)
		if (!(((tempStr[i]>='0') && (tempStr[i]<='9')) ||
			(((tempStr[i]|0x20)>='a') && ((tempStr[i]|0x20)<='f'))))
			return FALSE;
	
	return TRUE;
}

unsigned long HexStringToLong(Str255 tempStr)
{
	int				i;
	unsigned long	result;
	unsigned long	mult;
	unsigned long	digit;
	
	while (tempStr[0]<8)
	{
		for (i=tempStr[0]; i>0; i--)
			tempStr[i+1]=tempStr[i];
		tempStr[1]='0';
		tempStr[0]++;
	}

	mult=1L;
	result=0L;
	for (i=8; i>0; i--)
	{
		if ((tempStr[i]>='0') && (tempStr[i]<='9'))
			digit=tempStr[i]-'0';
		else
			digit=(tempStr[i]|0x20)-'a'+10;
		result+=digit*mult;
		mult=mult<<4;
	}
	
	return result;
}
