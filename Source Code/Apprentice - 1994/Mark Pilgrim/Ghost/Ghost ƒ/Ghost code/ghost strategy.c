/**********************************************************************\

File:		ghost strategy.c

Purpose:	This module handles computer player strategy -- looking
			at the dictionary and figuring out the best move.
			


Ghost -=- a classic word-building challenge
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

#include "ghost globals.h"
#include "ghost strategy.h"
#include "ghost end.h"
#include "msg graphics.h"
#include "msg dialogs.h"
#include "msg timing.h"

#define whichList	gTheWord[1]-'A'

long			listPtr;
long			startPtr;
long			oldStartPtr;
Str255			thisWord;

static int		screwMe[26],screwThem[26];

void DoComputerPlayer(void)
{
	Boolean			nextPlayerIsHuman;
	
	StartTiming();
	if (gTheWord[0]==0x00)
		AddLetter(RandomChoice());
	else if (gTheWord[0]==0x01)
	{
		if (gComputerIntelligence==0x03)
			AddLetter(SmartChoice());
		else
			AddLetter(RandomChoiceButRealWord());
	}
	else
	{
		switch (gComputerIntelligence)
		{
			case 0x01:
				AddLetter(RandomChoiceButRealWord());
				break;
			case 0x02:
				if (((Random()&0x7fff)%2)==1)
					AddLetter(RandomChoiceButRealWord());
				else
					AddLetter(SmartChoice());
				break;
			case 0x03:
				GoToNextPlayer();
				nextPlayerIsHuman=(gPlayOrderIndex[gCurrentPlayer]<gNumHumanPlayers);
				GoToPreviousPlayer();
				
				if (nextPlayerIsHuman)
				{
					if (((Random()&0x7fff)%4)==1)
						AddLetter(RandomChoice());
					else
						AddLetter(SmartChoice());
				}
				else AddLetter(SmartChoice());
				break;
		}
	}
}

char RandomChoice(void)
{
	return ('A'+(Random()&0x7fff)%26);
}

char RandomChoiceButRealWord(void)
{
	char			theChar;
	Boolean			atLeastOne;
	int				i;
	
	atLeastOne=FALSE;
	SearchList();
	for (i=0; (i<26) && (!atLeastOne); i++)
		atLeastOne|=((screwThem[i]>0) || (screwMe[i]>0));
		
	if (atLeastOne)
	{
		do
		{
			theChar='A'+(Random()&0x7fff)%26;
		}
		while ((screwThem[theChar-'A']==0) && (screwMe[theChar-'A']==0));
		
		return theChar;
	}
	else return ' ';
}

char SmartChoice(void)
{
	int				bestMe, bestThem;
	char			bestChar;
	int				theIndex;
	char			theChar;
	char			startChar;
	Boolean			goon;
	
	SearchList();
	bestMe=9999;
	bestThem=0;
	bestChar=' ';
	theChar=startChar='A'+((Random()&0x7fff)%26);
	goon=TRUE;
	
	do
	{
		theIndex=theChar-'A';
		
		if ((screwMe[theIndex]>0) && (screwThem[theIndex]>0))
		{
			if (screwThem[theIndex]/screwMe[theIndex]>=bestThem/bestMe)
			{
				bestMe=screwMe[theIndex];
				bestThem=screwThem[theIndex];
				bestChar=theChar;
			}
		}
		else if ((screwMe[theIndex]==0) && (screwThem[theIndex]>0))
		{
			bestChar=theChar;
			goon=FALSE;
		}
		else if ((screwMe[theIndex]>0) && (screwThem[theIndex]==0))
		{
			if ((bestThem==0) && (screwMe[theIndex]<=bestMe))
			{
				bestMe=screwMe[theIndex];
				bestChar=theChar;
			}
		}
		theChar++;
		if (theChar>'Z')
			theChar='A';
	}
	while ((theChar!=startChar) && (goon));
	
	return bestChar;
}

void FindStartPtr(Boolean updateOldStartPtr)
{
	Boolean			match;
	
	if ((updateOldStartPtr) && (gTheWord[0]==0x01))
		oldStartPtr=startPtr=gIndex[gUseFullDictionary ? kFull : kPartial][gTheWord[1]-'A'];
	
	listPtr=oldStartPtr;
	if (gTheWord[0]>0x01)
	{
		do
		{
			GetNextWord(FALSE);
			match=MatchWord();
			if (!match)
				GetNextWord(TRUE);
		}
		while ((!match) && (listPtr<gIndex[gUseFullDictionary ? kFull : kPartial][whichList+1]));
	}
	else match=FALSE;

	if (updateOldStartPtr)
		oldStartPtr=startPtr;

	startPtr=listPtr;
}

Boolean FindOneOccurrence(void)
{
	Boolean		found;
	
	found=FALSE;
	FindStartPtr(FALSE);
	listPtr=startPtr;
	while ((!found) && (listPtr<gIndex[gUseFullDictionary ? kFull : kPartial][whichList+1]))
	{
		GetNextWord(TRUE);
		if (MatchWord())
			found=TRUE;
	}
	
	return found;
}

void SearchList(void)
{
	Boolean			match;
	int				i;
	
	FindStartPtr(FALSE);
	for (i=0; i<26; i++)
		screwMe[i]=screwThem[i]=0;
	listPtr=startPtr;
	match=TRUE;
	while ((match) && (listPtr<gIndex[gUseFullDictionary ? kFull : kPartial][whichList+1]))
	{
		GetNextWord(TRUE);
		match=MatchWord();
		if (match)
		{
			if ((thisWord[0]-gTheWord[0]-1)%(gActualHumanPlayers+gActualComputerPlayers))
				screwThem[thisWord[gTheWord[0]+1]-'A']++;
			else
				screwMe[thisWord[gTheWord[0]+1]-'A']++;
		}
	}
}

void GetNextWord(Boolean updateListPtr)
{
	char			oneChar;
	long			tempPtr;
	
	tempPtr=listPtr;
	thisWord[0]=0x00;
	if (listPtr<gIndex[gUseFullDictionary ? kFull : kPartial][whichList+1])
	{
		do
		{
			oneChar=*((char*)((long)(gTheDictionary[gUseFullDictionary ? kFull : kPartial])+(tempPtr++)));
			if (oneChar!=0x0d)
				thisWord[++thisWord[0]]=oneChar;
		}
		while (oneChar!=0x0d);
		if (updateListPtr)
			listPtr=tempPtr;
	}
}

Boolean MatchWord(void)
{
	int				i;
	Boolean			match;
	
	i=gTheWord[0];
	if (thisWord[0]<i)
		match=FALSE;
	else
	{
		do
		{
			match=(gTheWord[i]==thisWord[i]);
			i--;
		}
		while ((i>0) && (match));
	}
	return match;
}
