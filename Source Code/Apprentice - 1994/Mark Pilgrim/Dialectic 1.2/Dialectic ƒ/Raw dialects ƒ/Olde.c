/**********************************************************************\

File:		Olde.c

Purpose:	This module handles actually converting text into Olde talk.


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

void ConvertOlde(void)
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
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)==(NextChar(2)|0x20)) &&
		(IsConsonant(NextChar(1))) && ((NextChar(3)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'y'-'i');
		StoreChar(NextChar(1));
		StoreChar(NextChar(3));
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='i') && (IsConsonant(NextChar(1))) && ((NextChar(2)|0x20)=='e') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'y'-'i');
		StoreChar(NextChar(1));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='c') && ((NextChar(2)|0x20)=='k') &&
		(IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'y'-'i');
		StoreChar(ThisChar()+'k'-'i');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='a') && (IsConsonant(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'e'-'i');
		StoreChar(NextChar(2));
		StoreChar(ThisChar()+'e'-'i');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='e') && (IsConsonant(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(2));
		StoreChar(NextChar(1));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='a') && (IsConsonant(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(2));
		StoreChar(ThisChar());
		InputPlus(3);
		return;
	}
	
	if ((IsConsonant(oneChar)) && ((NextChar(1)|0x20)=='y'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'e'-'y');
		StoreChar(NextChar(1)+'e'-'y');
		InputPlus(2);
		return;
	}
	
	if ((IsConsonant(oneChar)) && ((NextChar(1)|0x20)=='e') && ((NextChar(2)|0x20)=='r'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(2));
		StoreChar(NextChar(1));
		InputPlus(3);
		return;
	}
	
	if ((IsVowel(oneChar)) && ((NextChar(1)|0x20)=='r') && ((NextChar(2)|0x20)=='e') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='a') && (IsConsonant(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(2));
		StoreChar(ThisChar()+'e'-'o');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='i') && ((NextChar(2)|0x20)=='o') &&
		((NextChar(3)|0x20)=='n') && (!IsAlpha(NextChar(4))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'c'-'t');
		StoreChar(NextChar(1));
		StoreChar(NextChar(2));
		StoreChar(NextChar(2)+'u'-'o');
		StoreChar(NextChar(3));
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='o') && ((NextChar(2)|0x20)=='n'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(1)+'u'-'o');
		StoreChar(NextChar(2));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='i') && ((NextChar(2)|0x20)=='d'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(ThisChar()+'y'-'a');
		StoreChar(NextChar(2));
		StoreChar(NextChar(2)+'e'-'d');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='i'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'e'-'a');
		StoreChar(NextChar(1)+'y'-'i');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='y') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='y'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'e'-'a');
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && ((NextChar(1)|0x20)=='n') && ((NextChar(2)|0x20)=='t'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'u'-'n');
		StoreChar(NextChar(1));
		StoreChar(NextChar(2));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='a'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'e'-'a');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='a'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'o'-'a');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='u') && ((NextChar(1)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='u'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'w'-'u');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='w'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'u'-'w');
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='h') && ((NextChar(1)|0x20)=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'i'-'e');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='v') && ((NextChar(1)|0x20)=='e') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(1)+'t'-'e');
		StoreChar(NextChar(1)+'h'-'e');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='s') && ((NextChar(1)|0x20)=='e') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'e'-'s');
		InputPlus(2);
		return;
	}
	
	if (((ThisChar()=='�') || (ThisChar()==0x27)) && ((NextChar(1)|0x20)=='s') &&
		(!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(NextChar(1)+'e'-'s');
		StoreChar(NextChar(1));
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='c') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(1)+'k'-'c');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='c') && ((NextChar(2)|0x20)=='s') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2)+'c'-'s');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='i') && ((NextChar(1)|0x20)=='c') && ((NextChar(2)|0x20)=='a') &&
		((NextChar(3)|0x20)=='l') && (!IsAlpha(NextChar(4))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2)+'k'-'a');
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='t') && ((NextChar(1)|0x20)=='l') && ((NextChar(2)|0x20)=='e') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'i'-'l');
		StoreChar(NextChar(1));
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='l') && ((NextChar(1)|0x20)=='l') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='u') && ((NextChar(2)|0x20)=='l') &&
		((NextChar(3)|0x20)=='d') && (!IsAlpha(NextChar(4))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(3));
		StoreChar(NextChar(3)+'e'-'d');
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='o') && ((NextChar(1)|0x20)=='w') && ((NextChar(2)|0x20)=='n') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1)+'u'-'w');
		StoreChar(NextChar(2));
		StoreChar(NextChar(2)+'e'-'n');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='u') && ((NextChar(1)|0x20)=='n') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'o'-'u');
		StoreChar(NextChar(1));
		StoreChar(NextChar(1));
		StoreChar(NextChar(1)+'e'-'n');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='r') && ((NextChar(1)|0x20)=='r') && ((NextChar(2)|0x20)=='y') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(2));
		StoreChar(NextChar(2)+'e'-'y');
		InputPlus(3);
		return;
	}
	
	if ((oneChar=='e') && ((NextChar(1)|0x20)=='s') && ((NextChar(2)|0x20)=='t') &&
		(!IsAlpha(NextChar(3))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2));
		StoreChar(NextChar(2)+'e'-'t');
		InputPlus(3);
		return;
	}
	
	if ((((oneChar=='p') && ((NextChar(1)|0x20)=='t')) ||
		((oneChar=='t') && ((NextChar(1)|0x20)=='h')) ||
		((oneChar=='c') && ((NextChar(1)|0x20)=='h')) ||
		((oneChar=='s') && ((NextChar(1)|0x20)=='s'))) && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(1)+'e'-(NextChar(1)|0x20));
		InputPlus(2);
		return;
	}
	
	if (((oneChar=='w') ||
		(oneChar=='y') ||
		(oneChar=='b') ||
		(oneChar=='d') ||
		(oneChar=='p')) && (!IsAlpha(NextChar(1))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(ThisChar()+'e'-oneChar);
		InputPlus(1);
		return;
	}
	
	if (((oneChar=='r') ||
		(oneChar=='n') ||
		(oneChar=='t')) && (!IsAlpha(NextChar(1))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(ThisChar());
		StoreChar(ThisChar()+'e'-oneChar);
		InputPlus(1);
		return;
	}
	
	if ((!gInWord) && (oneChar=='f') && ((NextChar(1)|0x20)=='r') &&
		((NextChar(2)|0x20)=='o') && ((NextChar(3)|0x20)=='m') && (!IsAlpha(NextChar(4))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2));
		InputPlus(4);
		return;
	}
	
	if ((!gInWord) && (oneChar=='w') && ((NextChar(1)|0x20)=='h') &&
		((NextChar(2)|0x20)=='e') && ((NextChar(3)|0x20)=='n') && (!IsAlpha(NextChar(4))))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(NextChar(1));
		StoreChar(NextChar(2)+'a'-'e');
		StoreChar(NextChar(3));
		InputPlus(4);
		return;
	}
	
	gInWord=TRUE;
	StoreChar(ThisChar());
	InputPlus(1);
}
