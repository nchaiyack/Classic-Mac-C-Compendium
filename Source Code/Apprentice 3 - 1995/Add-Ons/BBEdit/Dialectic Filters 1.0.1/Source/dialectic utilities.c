/**********************************************************************\

File:		dialectic utilities.c

Purpose:	This module contains various utilities which the raw
			dialect routines use in converting text.
			
Modified:	November 1994, by Leo Breebaart

			I've replaced the bodies of the ThisChar(), NextChar(),
			StoreChar() and InputPlus() functions. These functions now
			work with the data structures I have defined for the 
			Dialectic Filters BBEdit extension, but otherwise they 
			have exactly the same effect as before. Since the prototypes
			have remained the same, this means that I can now compile in 
			every 'Raw Filter' dialect as is, without change.


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

--------

Changes made by Leo Breebaart for the Dialect Filters BBedit extension:

7 Nov 1994: I rewrote some of the functions in this file in order to
make them suitable for use by the extension.

\**********************************************************************/

#include "dialectic utilities.h"
#include "program globals.h"
#include "Dialectic Filters.h"

Boolean			gInWord;
Boolean			gSeenI;

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
			(thisChar=='�'));
}

Boolean IsUpperAlpha(char thisChar)
{
	return (((thisChar>='A') && (thisChar<='Z')) || (thisChar==0x27) || (thisChar=='�'));
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
	if (gInput.offset >= gInput.length)
		return 0x00;
	else
		return *(gInput.base + gInput.offset);
}

char NextChar(int howmany)
{
	if (gInput.offset + howmany >= gInput.length)
		return 0x00;
	else
		return *(gInput.base + gInput.offset + howmany);
}


void StoreChar(char thisChar)
{
	if (gOutput.offset >= gOutput.length)
	{
		HUnlock(gOutput.theHandle);
		SetHandleSize(gOutput.theHandle, gOutput.length + gInput.length);
		if (MemError() != 0)
		{
			gAbort = TRUE;
			return;
		}

		HLockHi(gOutput.theHandle);
		gOutput.base = (char *) *gOutput.theHandle;
		gOutput.length += gInput.length;
	}
	*(gOutput.base + gOutput.offset) = thisChar;
	gOutput.offset++;
}

void StoreString(Str255 thisString)
{
	int	i;
	
	for (i=1; i<=thisString[0]; i++)
		StoreChar(thisString[i]);
}

void InputPlus(int howmany)
{
	gInput.offset += howmany;
}
