/**********************************************************************\

File:		pig.c

Purpose:	This module handles actually converting text into Pig Latin.


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

void ConvertPig(void)
{
	Str255			theConsonants;
	Str255			thisWord;
	int				i;
	int				origLength;
	Boolean			firstIsCaps;
	Boolean			isAllCaps;
	
	if (!IsAlpha(ThisChar()))
	{
		StoreChar(ThisChar());
		InputPlus(1);
		return;
	}
	
	origLength=GetRestOfWord(thisWord);
	if (thisWord[0]==0x00)	return;
	
	firstIsCaps=IsUpperAlpha(thisWord[1]);
	isAllCaps=IsAllCaps(thisWord);
	
	theConsonants[0]=0x00;
	for (i=1; i<=origLength; i++)
	{
		if (IsConsonant(thisWord[i]))
			theConsonants[++theConsonants[0]]=thisWord[i];
		else
			i=origLength+1;
	}
	
	if (theConsonants[0]>0x00)
	{
		for (i=1; i<=origLength-theConsonants[0]; i++)
			thisWord[i]=thisWord[i+theConsonants[0]];
		for (i=1; i<=theConsonants[0]; i++)
			thisWord[origLength-theConsonants[0]+i]=isAllCaps ? theConsonants[i]&0xdf :
				theConsonants[i]|0x20;
		if (firstIsCaps)
			thisWord[1]&=0xdf;
	}

	thisWord[++thisWord[0]]=isAllCaps ? 'A' : 'a';
	thisWord[++thisWord[0]]=isAllCaps ? 'Y' : 'y';
	StoreString(thisWord);
	InputPlus(origLength);
}
