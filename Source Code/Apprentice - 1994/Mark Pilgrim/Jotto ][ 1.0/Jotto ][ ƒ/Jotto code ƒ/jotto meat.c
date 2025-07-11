/**********************************************************************\

File:		jotto meat.c

Purpose:	This module handles the behind-the-scenes calculations in
			Jotto -- includes preparing for a new word, checking
			all relevant dictionaries for words, calculating the
			number of correct letters in a guess.


Jotto ][ -=- a simple word game, revisited
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

#include "jotto globals.h"
#include "jotto meat.h"
#include "jotto dictionary.h"
#include "util.h"

enum
{
	kGoDown=-1,
	kGotIt,
	kGoUp
};

void NewWord(void)
{
	int				i;
	
	for (i=0; i<gNumLetters; i++)
		gHumanWord[gNumTries][i]=' ';
	gWhichChar=0x00;
}

Boolean ValidWord(char *thisWord)
{
	Boolean			result;
	
	if (InRealDictionary(thisWord))
		return TRUE;
	else
		return (InCustomDictionary(thisWord));
}

Boolean InCustomDictionary(char *thisWord)
{
	unsigned long	index;
	unsigned char	temp[6];
	Boolean			stillMoreWords;
	int				found;
	
	if ((gNumLetters==5) ? (gFiveLetterCustomOK) : (gSixLetterCustomOK))
	{
		index=0L;
		found=666;
		while ((GetCustomWord(temp, index++)) && (found!=kGotIt))
			found=DirectionalCompare(thisWord, temp);
		
		return (found==kGotIt);
	}
	else return FALSE;
}

Boolean InRealDictionary(char *thisWord)
{
	unsigned long	index;
	char			temp[6];
	int				direction;
	unsigned long	gap;
	int				iter;
	
	index=gNumHumanWords[gNumLetters-5]/2;
	gap=index;
	iter=(gNumLetters==6) ? 14 : 13;
	while (iter>0)
	{
		iter--;
		if (gap%2)
			gap=(gap>>1)+1;
		else
			gap=gap>>1;
		GetHumanWord(temp, index);
		direction=DirectionalCompare(thisWord, temp);
		switch (direction)
		{
			case kGotIt:	return TRUE; break;
			case kGoDown:	index-=gap; break;
			case kGoUp:		index+=gap; break;
		}
		if (index<0)
			index=0;
		else if (index>=gNumHumanWords[gNumLetters-5])
			index=gNumHumanWords[gNumLetters-5]-1;
	}
	
	return FALSE;
}

int DirectionalCompare(char *thisOne, char *thatOne)
{
	int			i;
	
	for (i=0; i<gNumLetters; i++)
		if (thisOne[i]<thatOne[i])
			return kGoDown;
		else if (thisOne[i]>thatOne[i])
			return kGoUp;

	return kGotIt;
}

void CalculateNumRight(void)
{
	Boolean			used[6];
	int				i,j;
	
	gNumRight[gNumTries]=0;
	for (i=0; i<gNumLetters; i++)
		used[i]=FALSE;
	for (i=0; i<gNumLetters; i++)
		for (j=0; j<gNumLetters; j++)
		{
			if ((gHumanWord[gNumTries][i]==gComputerWord[j]) && (!used[j]))
			{
				gNumRight[gNumTries]++;
				used[j]=TRUE;
				j=gNumLetters;
			}
		}	
}
