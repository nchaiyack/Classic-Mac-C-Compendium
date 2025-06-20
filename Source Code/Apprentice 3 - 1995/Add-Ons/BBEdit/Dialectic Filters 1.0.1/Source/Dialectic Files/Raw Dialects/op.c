/**********************************************************************\

File:		op.c

Purpose:	This module handles actually converting text into Op.


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

void ConvertOp(void)
{
	Str255			thisWord, thatWord;
	int				i;
	int				origLength;
	Boolean			firstIsCaps;
	Boolean			isAllCaps;
	Boolean			lookingForVowel;
	Boolean			justGotQ;
	
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
	
	thatWord[0]=0x00;
	lookingForVowel=TRUE;
	justGotQ=FALSE;
	for (i=1; i<=origLength; i++)
	{
		if ((IsVowel(thisWord[i])) || ((thisWord[i]|0x20)=='y'))
		{
			if ((lookingForVowel) && (!justGotQ))
			{
				thatWord[++thatWord[0]]=isAllCaps ? 'O' : 'o';
				thatWord[++thatWord[0]]=isAllCaps ? 'P' : 'p';
				lookingForVowel=FALSE;
			}
			justGotQ=FALSE;
		}
		else
		{
			lookingForVowel=TRUE;
			justGotQ=((thisWord[i]|0x20)=='q');
		}
		
		thatWord[++thatWord[0]]=thisWord[i];
		if ((i==1) && (!isAllCaps) && (thisWord[1]>='A') && (thisWord[1]<='Z'))
			thatWord[thatWord[0]]|=0x20;
	}
	
	if ((firstIsCaps) && (thatWord[1]>='a') && (thatWord[1]<='z'))
		thatWord[1]&=0xdf;

	StoreString(thatWord);
	InputPlus(origLength);
}
