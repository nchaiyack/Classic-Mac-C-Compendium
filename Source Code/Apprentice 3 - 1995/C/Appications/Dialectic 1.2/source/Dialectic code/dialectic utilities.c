/**********************************************************************\

File:		dialectic utilities.c

Purpose:	This module contains various utilities which the raw
			dialect routines use in converting text.
			


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

#include "dialectic utilities.h"
#include "program globals.h"

Boolean IsAllCaps(Str255 theWord)
{
	int				i;
	
	for (i=1; i<=theWord[0]; i++)
		if (!IsUpperAlpha(theWord[i]))
			return FALSE;
	
	return TRUE;
}

unsigned char GetRestOfWord(Str255 theWord)
{
	int				i;
	
	i=0;
	theWord[0]=0x00;
	while ((theWord[0]<0xFF) && (IsAlpha(NextChar(i))))
		theWord[++theWord[0]]=NextChar(i++);
	
	return theWord[0];
}

Boolean IsAlpha(char thisChar)
{
	return ((((thisChar|0x20)>='a') && ((thisChar|0x20)<='z')) || (thisChar==0x27) ||
			(thisChar=='Õ'));
}

Boolean IsUpperAlpha(char thisChar)
{
	return (((thisChar>='A') && (thisChar<='Z')) || (thisChar==0x27) || (thisChar=='Õ'));
}

Boolean IsNumeric(char thisChar)
{
	return ((thisChar>='0') && (thisChar<='9'));
}

Boolean IsVowel(char thisChar)
{
	char		lowerChar;
	
	lowerChar=thisChar|0x20;
	return ((lowerChar=='a') || (lowerChar=='e') || (lowerChar=='i') ||
			(lowerChar=='o') || (lowerChar=='u'));
}

Boolean IsConsonant(char thisChar)
{
	return (IsAlpha(thisChar) && (!IsVowel(thisChar)));
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
