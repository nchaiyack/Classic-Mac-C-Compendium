/**********************************************************************\

File:		fudd.c

Purpose:	This module handles actually converting text into Fudd talk.


Dialectic -=- dialect text conversion extraordinare
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

#include "dialectic dispatch.h"
#include "dialectic utilities.h"
#include "program globals.h"

void ConvertFudd(void)
{
	char			oneChar;
	
	if (!IsAlpha(ThisChar()))
	{
		gInWord=FALSE;
		StoreChar(ThisChar());
		InputPlus(1);
		return;
	}
	
	oneChar=(ThisChar())|0x20;
	
	if ((oneChar=='r') || (oneChar=='l'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'w'-oneChar);
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='q') && ((NextChar(1)|0x20)=='u'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'w'-'u');
		InputPlus(2);
		return;		
	}
	
	if ((gInWord) && (oneChar=='t') && ((NextChar(1)|0x20)=='h') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'f'-'t');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='h'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'d'-'t');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='n') && (NextChar(1)=='.'))
	{
		gInWord=TRUE;
		StoreString("\pn, uh-hah-hah-hah.");
		InputPlus(2);
		return;
	}
	
	gInWord=TRUE;
	StoreChar(ThisChar());
	InputPlus(1);
}
