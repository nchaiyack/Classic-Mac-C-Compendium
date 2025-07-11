/**********************************************************************\

File:		chef meat.c

Purpose:	This module handles the actual conversion to Swedish Chef.


Chef -=- convert text to Swedish chef talk
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

#include "chef meat.h"
#include "program globals.h"

Boolean			gInWord;
Boolean			gSeenI;
unsigned long	gInputOffset;
unsigned long	gOutputOffset;
Ptr				gInputBuffer;
Ptr				gOutputBuffer;
Boolean			gInputNeedsUpdate;
Boolean			gOutputNeedsUpdate;
unsigned long	gAbsoluteOffset;
unsigned long	gInputLength;
Boolean			gSeenBackslash;
int				gCurlyLevel;

void ConvertIt(void)
{
	char			oneChar;
	
	if (gUseRTF)
	{
		oneChar=ThisChar();
		if ((oneChar=='}') && (gCurlyLevel>0))
			gCurlyLevel--;
		else if (oneChar=='{')
			gCurlyLevel++;
		
		if (oneChar==0x5c)
			gSeenBackslash=TRUE;
		
		if ((gCurlyLevel>2) || (gSeenBackslash))
		{
			StoreChar(oneChar);
			InputPlus(1);
			if ((oneChar==' ') || (IsNumeric(oneChar)))
				gSeenBackslash=FALSE;
			return;
		}
	}
	
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
	
	if ((!gInWord) && (oneChar=='b') && (NextChar(1)=='o')
			&& (NextChar(2)=='r') && (NextChar(3)=='k'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreString("\pork");
		InputPlus(4);
		return;
	}
	
	if ((oneChar=='a') && (NextChar(1)=='n'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'u'-'a');
		StoreChar('n');
		InputPlus(2);
		return;
	}
	
	if ((oneChar=='a') && (NextChar(1)=='u'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'o'-'a');
		StoreChar('o');
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
	
	if ((ThisChar()=='e') && (NextChar(1)=='n') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreString("\pee");
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='e') && (NextChar(1)=='w'))
	{
		StoreString("\poo");
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='e') && (!IsAlpha(NextChar(1))))
	{
		StoreString("\pe-a");
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
	
	if ((gInWord) && (ThisChar()=='f'))
	{
		StoreString("\pff");
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='i') && (NextChar(1)=='r'))
	{
		StoreString("\pur");
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='i'))
	{
		if (gSeenI)
			StoreChar('i');
		else
			StoreString("\pee");
		gSeenI=TRUE;
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='o') && (NextChar(1)=='w'))
	{
		StoreString("\poo");
		InputPlus(2);
		return;
	}
	
	if ((!gInWord) && (oneChar=='o'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar());
		StoreChar(((oneChar=='O') && (IsUpperAlpha(NextChar(1)))) ? 'O' : 'o');
		InputPlus(1);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='o'))
	{
		StoreChar('u');
		InputPlus(1);
		return;
	}
	
	if ((oneChar=='t') && (NextChar(1)=='h') && (NextChar(2)=='e'))
	{
		gInWord=TRUE;
		StoreChar(ThisChar()+'z'-'t');
		StoreString("\pee");
		InputPlus(3);
		return;
	}
	
	if ((ThisChar()=='t') && (NextChar(1)=='h') && (!IsAlpha(NextChar(2))))
	{
		gInWord=TRUE;
		StoreChar('t');
		InputPlus(2);
		return;
	}
	
	if ((gInWord) && (ThisChar()=='t') && (NextChar(1)=='i') && (NextChar(2)=='o') && (NextChar(3)=='n'))
	{
		gInWord=TRUE;
		StoreString("\pshun");
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

Boolean IsAlpha(char thisChar)
{
	return ((((thisChar|0x20)>='a') && ((thisChar|0x20)<='z')) || (thisChar==0x27));
}

Boolean IsUpperAlpha(char thisChar)
{
	return (((thisChar>='A') && (thisChar<='Z')) || (thisChar==0x27));
}

Boolean IsNumeric(char thisChar)
{
	return ((thisChar>='0') && (thisChar<='9'));
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
