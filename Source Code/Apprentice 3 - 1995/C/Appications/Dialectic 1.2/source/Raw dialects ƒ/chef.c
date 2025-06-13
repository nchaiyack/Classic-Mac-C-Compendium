/**********************************************************************\

File:		chef.c

Purpose:	This module handles actually converting text into Chef talk.


Dialectic -=- dialect text conversion extraordinare
Copyright ©1994, Mark Pilgrim

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

void ConvertChef(void)
{
	char			oneChar;
	
	oneChar=(ThisChar())|0x20;
	
	if ((ThisChar()=='.') && (NextChar(1)==0x0d))
	{
		gInWord=gSeenI=FALSE;
		StoreChar('.');
		StoreChar(0x0d);
		StoreString("\pBork Bork Bork!");
		InputPlus(1);
		return;
	}
	
	if (!IsAlpha(ThisChar()))
	{
		gInWord=gSeenI=FALSE;
		StoreChar(ThisChar());
		InputPlus(1);
		return;
	}
	
	if ((!gInWord) && (oneChar=='b') && ((NextChar(1)|0x20)=='o')
			&& ((NextChar(2)|0x20)=='r') && ((NextChar(3)|0x20)=='k'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2));
		StoreChar(NextChar(3));
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='n'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'u'-'a');
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='u'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'o'-'a');
		StoreChar(NextChar(1)+'o'-'u');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && (IsAlpha(NextChar(1))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'e'-'a');
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='n') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'e'-'n');
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='e') && ((NextChar(1)|0x20)=='w'))
	{
		StoreChar(ThisChar()+'o'-'e');
		StoreChar(NextChar(1)+'o'-'w');
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='e') && (!IsAlpha(NextChar(1))))
	{
		StoreChar(ThisChar());
		StoreChar('-');
		StoreChar(ThisChar()+'a'-'e');
		InputPlus(1);
		return;
	}
	
	if ((!gInWord) && (oneChar=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'i'-'e');
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (oneChar=='f'))
	{
		StoreChar(ThisChar());
		StoreChar(ThisChar());
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (oneChar=='i') && ((NextChar(1)|0x20)=='r'))
	{
		StoreChar(ThisChar()+'u'-'i');
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='i'))
	{
		if (gSeenI)
			StoreChar(ThisChar());
		else
		{
			StoreChar(ThisChar()+'e'-'i');
			StoreChar(ThisChar()+'e'-'i');
		}
		gSeenI=TRUE;
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (oneChar=='o') && ((NextChar(1)|0x20)=='w'))
	{
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'o'-'w');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='o'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(((ThisChar()=='O') && (IsUpperAlpha(NextChar(1)))) ? 'O' : 'o');
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (oneChar=='o'))
	{
		StoreChar(ThisChar()+'u'-'o');
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='h') && ((NextChar(2)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'z'-'t');
		StoreChar(NextChar(1)+'e'-'h');
		StoreChar(NextChar(2));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='h') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (oneChar=='t') && ((NextChar(1)|0x20)=='i') &&
		((NextChar(2)|0x20)=='o') && ((NextChar(3)|0x20)=='n'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'s'-'t');
		StoreChar(NextChar(1)+'h'-'i');
		StoreChar(NextChar(2)+'u'-'o');
		StoreChar(NextChar(3));
		InputPlus(4);
		return;
	}
	
	if ((gInWord) && (oneChar=='u'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'o'-'u');
		StoreChar((IsUpperAlpha(NextChar(1))) ? 'O' : 'o');
		InputPlus(1);
		return;
	}
	
	if (oneChar=='v')
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'f'-'v');
		InputPlus(1);
		return;
	}
	
	if (oneChar=='w')
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'v'-'w');
		InputPlus(1);
		return;
	}
	
	gInWord=TRUE;
	StoreChar(ThisChar());
	InputPlus(1);
}
