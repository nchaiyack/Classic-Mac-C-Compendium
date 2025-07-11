/**********************************************************************\

File:		fudd meat.c

Purpose:	This module handles actually converting text into Fudd talk.


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

#include "fudd meat.h"
#include "program globals.h"

unsigned long	gInputOffset;
unsigned long	gOutputOffset;
Ptr				gInputBuffer;
Ptr				gOutputBuffer;
Boolean			gInputNeedsUpdate;
Boolean			gOutputNeedsUpdate;
unsigned long	gAbsoluteOffset;
unsigned long	gInputLength;

void ConvertIt(void)
{
	char			oneChar;
	
	oneChar=(ThisChar())|0x20;
	
	if ((oneChar=='r') || (oneChar=='l'))
	{
		StoreChar(ThisChar()+'w'-oneChar);
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='q') && ((NextChar(1)|0x20)=='u'))
	{
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'w'-'u');
		InputPlus(2);
		return;		
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='h') && (NextChar(2)==' '))
	{
		StoreChar(ThisChar()+'f'-'t');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='h'))
	{
		StoreChar(ThisChar()+'d'-'t');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='n') && (NextChar(1)=='.'))
	{
		StoreString("\pn, uh-hah-hah-hah");
		InputPlus(1);
		return;
	}
	
	StoreChar(ThisChar());
	InputPlus(1);
}

char ThisChar(void)
{
	if (gInputOffset>=INPUT_BUFFER_MAX)
		gInputNeedsUpdate=TRUE;
	if (gAbsoluteOffset>=gInputLength)
		return 0x00;
	else
		return *((char*)((long)gInputBuffer+gInputOffset));
}

char NextChar(int howmany)
{
	if (gAbsoluteOffset+howmany>=gInputLength)
		return 0x00;
	else
		return *((char*)((long)gInputBuffer+gInputOffset+howmany));
}

void StoreChar(char thisChar)
{
	*((char*)((long)gOutputBuffer+(gOutputOffset++)))=thisChar;
	if (gOutputOffset>=OUTPUT_BUFFER_MAX)
		gOutputNeedsUpdate=TRUE;
}

void StoreString(Str255 thisString)
{
	int				i;
	
	for (i=1; i<=thisString[0]; i++)
		StoreChar(thisString[i]);
}

void InputPlus(int howmany)
{
	gInputOffset+=howmany;
	gAbsoluteOffset+=howmany;
}
